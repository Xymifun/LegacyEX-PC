#include "platform/pc/PcDedicated.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameType.hpp"
#include "game/network/PlayerInfoPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "platform/pc/PcNetworkSession.hpp"
#include "platform/pc/PcOs.hpp"
#include "platform/wiiu/C4JStorage.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/CProfile.hpp"

#include "platform/pc/PcPaths.hpp"

#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <wctype.h>

#include <set>
#include <string>

extern C_4JProfile* gProfile;
extern C4JStorage* gStorage;

namespace
{
const unsigned long long MINUTE_NS = 60ULL * 1000000000ULL;

const unsigned long long STOP_CHECK_NS = 1000000000ULL;
const char STOP_FILE[] = "/stop-server";
const char OPS_FILE[] = "/ops.txt";

bool s_dedicated = false;
bool phantomsOn = true;
int saveMinutes = 15;
unsigned long long nextSaveTime = 0;
volatile sig_atomic_t stopSignalled = 0;
bool stopPending = false;
unsigned long long nextStopCheck = 0;
volatile sig_atomic_t saveNow = 0;
volatile sig_atomic_t listNow = 0;
C4JThreadImpl* consoleThread = 0;
char pendingCommand[256];
volatile sig_atomic_t commandReady = 0;

bool SameText(const std::wstring& a, const std::wstring& b)
{
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (towlower(a[i]) != towlower(b[i]))
            return false;
    return true;
}

bool IsCommand(const char* line, const char* name)
{
    const size_t len = strlen(name);
    return strncmp(line, name, len) == 0 && (line[len] == 0 || line[len] == ' ');
}

void QueueCommand(const char* text)
{
    while (commandReady)
        PcOsSleepMilliseconds(20);
    strncpy(pendingCommand, text, sizeof(pendingCommand) - 1);
    pendingCommand[sizeof(pendingCommand) - 1] = 0;
    commandReady = 1;
}

int ConsoleThreadProc(void*)
{
    char buf[256];
    while (fgets(buf, sizeof(buf), stdin) != 0)
    {
        char* cmd = buf;
        while (*cmd == ' ' || *cmd == '\t' || *cmd == '/')
            ++cmd;
        size_t len = strlen(cmd);
        while (len > 0 && (cmd[len - 1] == '\n' || cmd[len - 1] == '\r' || cmd[len - 1] == ' '))
            cmd[--len] = 0;
        if (len == 0)
            continue;
        if (strcmp(cmd, "stop") == 0 || strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0)
            stopSignalled = 1;
        else if (strcmp(cmd, "save") == 0 || strcmp(cmd, "save-all") == 0)
            saveNow = 1;
        else if (strcmp(cmd, "list") == 0)
            listNow = 1;
        else if (IsCommand(cmd, "op") || IsCommand(cmd, "deop") || IsCommand(cmd, "ops")
                 || IsCommand(cmd, "difficulty") || IsCommand(cmd, "phantoms"))
            QueueCommand(cmd);
        else
        {
            printf("[pc] server: commands: stop (save and shut down), save, list, op <player>, deop <player>, ops, "
                   "difficulty <peaceful|easy|normal|hard>, phantoms <on|off>, help\n");
            fflush(stdout);
        }
    }
    return 0;
}

void ListPlayers()
{
    if (gMinecraftServer == 0 || gMinecraftServer->getPlayers() == 0)
        return;
    std::vector<boost::shared_ptr<ServerPlayer> >* all = gMinecraftServer->getPlayers()->getPlayers();
    std::wstring names;
    int n = 0;
    for (size_t i = 0; i < all->size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> p = all->at(i);
        if (p->mHiddenHost)
            continue;
        if (n++ != 0)
            names += L", ";
        names += p->getName();
    }
    printf("[pc] server: %d player(s) online%s%ls\n", n, n != 0 ? ": " : "", names.c_str());
    fflush(stdout);
}

std::string OperatorsPath()
{
    return PcSaveRoot() + OPS_FILE;
}

bool ParseUuid(const char* s, std::string* out)
{
    while (*s == ' ' || *s == '\t')
        ++s;
    if (strncmp(s, "uuid=", 5) == 0)
        s += 5;
    std::string hex;
    for (; *s != 0 && *s != '#'; ++s)
    {
        const char c = *s;
        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
            hex += c;
        else if (c >= 'A' && c <= 'F')
            hex += (char)(c - 'A' + 'a');
        else if (c != '-' && c != ' ' && c != '\t' && c != '\r' && c != '\n')
            return false;
    }
    if (hex.size() != 32)
        return false;
    *out = hex;
    return true;
}

std::string FormatUuid(const unsigned char uuid[16])
{
    char buf[33];
    for (int i = 0; i < 16; ++i)
        snprintf(buf + i * 2, 3, "%02x", uuid[i]);
    return std::string(buf, 32);
}

std::set<std::string> ReadOperators()
{
    std::set<std::string> ops;
    FILE* fp = fopen(OperatorsPath().c_str(), "r");
    if (fp == 0)
        return ops;
    char buf[256];
    while (fgets(buf, sizeof(buf), fp) != 0)
    {
        std::string uuid;
        if (ParseUuid(buf, &uuid))
            ops.insert(uuid);
    }
    fclose(fp);
    return ops;
}

bool WriteOperators(const std::set<std::string>& ops)
{
    const std::string path = OperatorsPath();
    const std::string tmpPath = path + ".tmp";
    FILE* fp = fopen(tmpPath.c_str(), "w");
    if (fp == 0)
        return false;
    fprintf(fp, "# Identity uuids with host privileges on this server, one per line (see the README).\n");
    for (std::set<std::string>::const_iterator it = ops.begin(); it != ops.end(); ++it)
        fprintf(fp, "%s\n", it->c_str());
    const bool ok = fclose(fp) == 0;
    return ok && PcOsReplaceFile(tmpPath.c_str(), path.c_str());
}

bool PlayerUuid(boost::shared_ptr<ServerPlayer> player, std::string* uuid)
{
    if (player->mConnection == 0)
        return false;
    INetworkPlayer* networkPlayer = player->mConnection->getNetworkPlayer();
    unsigned char bytes[16];
    if (networkPlayer == 0 || networkPlayer->IsHost()
        || !PcNetworkSession::VerifiedUuid(networkPlayer->GetSmallId(), bytes))
        return false;
    *uuid = FormatUuid(bytes);
    return true;
}

boost::shared_ptr<ServerPlayer> FindPlayer(const std::wstring& name)
{
    std::vector<boost::shared_ptr<ServerPlayer> >* all = gMinecraftServer->getPlayers()->getPlayers();
    for (size_t i = 0; i < all->size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> p = all->at(i);
        if (!p->mHiddenHost && SameText(p->getName(), name))
            return p;
    }
    return boost::shared_ptr<ServerPlayer>();
}

void SetOperatorPrivileges(boost::shared_ptr<ServerPlayer> p, bool on)
{
    if (on)
    {
        p->enableAllPlayerPrivileges(true);
        p->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_Moderator, 1);
    }
    else
    {
        p->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_Moderator, 0);
        p->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_Invisible, 0);
        p->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_Invulnerable, 0);
        p->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanFly, 0);
        p->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_ExhaustionDisabled, 0);
        p->enableAllPlayerPrivileges(false);
    }
    gMinecraftServer->getPlayers()->broadcastAll(boost::shared_ptr<Packet>(new PlayerInfoPacket(p)));
}

void ChangeOperator(const std::wstring& name, bool on)
{
    boost::shared_ptr<ServerPlayer> p = FindPlayer(name);
    if (!p)
    {
        printf("[pc] server: %ls is not online (op and deop take a player who is in the world)\n", name.c_str());
        return;
    }
    std::string uuid;
    if (!PlayerUuid(p, &uuid))
    {
        printf("[pc] server: %ls did not join through the lobby, so their identity is not verified; add their uuid "
               "to ops.txt instead\n", p->getName().c_str());
        return;
    }
    std::set<std::string> ops = ReadOperators();
    if (on)
        ops.insert(uuid);
    else
        ops.erase(uuid);
    if (!WriteOperators(ops))
    {
        printf("[pc] server: could not write %s\n", OperatorsPath().c_str());
        return;
    }
    SetOperatorPrivileges(p, on);
    printf("[pc] server: %ls %s host privileges (%s)\n", p->getName().c_str(), on ? "now has" : "no longer has",
           uuid.c_str());
}

void ListOperators()
{
    std::set<std::string> ops = ReadOperators();
    printf("[pc] server: %u operator(s) in %s\n", (unsigned int)ops.size(), OperatorsPath().c_str());
    std::vector<boost::shared_ptr<ServerPlayer> >* all = gMinecraftServer->getPlayers()->getPlayers();
    for (std::set<std::string>::const_iterator it = ops.begin(); it != ops.end(); ++it)
    {
        std::wstring onlineName;
        for (size_t i = 0; i < all->size(); ++i)
        {
            std::string uuid;
            if (PlayerUuid(all->at(i), &uuid) && uuid == *it)
                onlineName = all->at(i)->getName();
        }
        printf("[pc] server:   %s%s%ls\n", it->c_str(), onlineName.empty() ? "" : "  online as ", onlineName.c_str());
    }
}

void ChangeDifficulty(const char* arg)
{
    const char* names[] = {"peaceful", "easy", "normal", "hard"};
    int d = -1;
    for (int i = 0; i < 4; ++i)
    {
        std::string lower;
        for (const char* c = arg; *c != 0; ++c)
            lower += (char)tolower((unsigned char)(*c));
        if (lower == names[i] || (arg[0] == '0' + i && arg[1] == 0))
            d = i;
    }
    if (d < 0)
    {
        printf("[pc] server: difficulty is %s; use difficulty peaceful, easy, normal or hard\n",
               names[gApp.GetGameHostOption(eGameHostOption_Difficulty) & 3]);
        return;
    }
    gMinecraftServer->setDifficulty(Difficulty::byId(d));
    printf("[pc] server: difficulty set to %s\n", names[d]);
}

void ChangePhantoms(const char* arg)
{
    if (strcmp(arg, "on") == 0 || strcmp(arg, "off") == 0)
    {
        phantomsOn = strcmp(arg, "on") == 0;
        printf("[pc] server: phantoms %s until the server restarts (the startup option sets them for good)\n",
               phantomsOn ? "spawn" : "no longer spawn");
    }
    else
        printf("[pc] server: phantoms are %s; use phantoms on or off\n", phantomsOn ? "on" : "off");
}

void RunCommand(const char* cmd)
{
    const char* arg = strchr(cmd, ' ');
    while (arg != 0 && *arg == ' ')
        ++arg;
    std::wstring name;
    if (arg != 0)
        for (const char* c = arg; *c != 0; ++c)
            name += (wchar_t)((unsigned char)(*c));
    if (IsCommand(cmd, "ops"))
        ListOperators();
    else if (IsCommand(cmd, "phantoms"))
        ChangePhantoms(arg != 0 ? arg : "");
    else if (IsCommand(cmd, "difficulty"))
        ChangeDifficulty(arg != 0 ? arg : "");
    else if (name.empty())
        printf("[pc] server: %s needs a player name\n", IsCommand(cmd, "op") ? "op" : "deop");
    else
        ChangeOperator(name, IsCommand(cmd, "op"));
    fflush(stdout);
}

void StopRequested()
{
    stopSignalled = 1;
}

void HideHostPlayer()
{
    if (gMinecraftServer == 0 || gMinecraftServer->getPlayers() == 0)
        return;
    std::vector<boost::shared_ptr<ServerPlayer> >* all = gMinecraftServer->getPlayers()->getPlayers();
    for (size_t i = 0; i < all->size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> p = all->at(i);
        if (p->mHiddenHost && !p->isSpectator())
        {
            p->setGameMode(GameType::SPECTATOR);
            printf("[pc] server: %ls is the hidden host (spectator at spawn)\n", p->getName().c_str());
            fflush(stdout);
        }
    }
}
}

bool PcIsServerOperator(INetworkPlayer* player)
{
    unsigned char bytes[16];
    if (!s_dedicated || player == 0 || player->IsHost()
        || !PcNetworkSession::VerifiedUuid(player->GetSmallId(), bytes))
        return false;
    std::set<std::string> operators = ReadOperators();
    return operators.find(FormatUuid(bytes)) != operators.end();
}

bool PcPhantomsEnabled()
{
    return phantomsOn;
}

void PcSetPhantomsEnabled(bool on)
{
    phantomsOn = on;
}

bool PcIsHiddenHost(INetworkPlayer* player)
{
    return player != 0 && player->IsHost() && (s_dedicated || PcNetworkSession::JoinedDedicatedServer());
}

bool PcIsDedicatedServer()
{
    return s_dedicated;
}

void PcSetDedicatedServer(bool on)
{
    s_dedicated = on;
    if (!on)
        return;
    PcOsInstallStopHandler(StopRequested);
    if (consoleThread == 0)
    {
        consoleThread = new C4JThreadImpl(ConsoleThreadProc, 0, "ServerConsole", 0);
        consoleThread->Run();
    }
}

void PcSetDedicatedSaveMinutes(int minutes)
{
    saveMinutes = minutes < 1 ? 1 : minutes;
}

bool PcDedicatedStopRequested()
{
    if (!s_dedicated)
        return false;
    if (stopPending)
        return true;
    const unsigned long long now = PcOsMonotonicNanoseconds();
    bool stop = stopSignalled != 0;
    if (!stop && now >= nextStopCheck)
    {
        nextStopCheck = now + STOP_CHECK_NS;
        const std::string path = PcSaveRoot() + STOP_FILE;
        if (FILE* fp = fopen(path.c_str(), "r"))
        {
            fclose(fp);
            remove(path.c_str());
            stop = true;
        }
    }
    if (stop)
    {
        stopPending = true;
        printf("[pc] server: stop requested; saving the world and shutting down\n");
        fflush(stdout);
    }
    return stop;
}

void PcDedicatedTick()
{
    if (!s_dedicated)
        return;
    const unsigned long long now = PcOsMonotonicNanoseconds();
    if (!CGameNetworkManager::sInstance.IsInGameplay() || !CGameNetworkManager::sInstance.IsHost())
    {
        nextSaveTime = now + (unsigned long long)saveMinutes * MINUTE_NS;
        return;
    }
    HideHostPlayer();
    if (listNow)
    {
        listNow = 0;
        ListPlayers();
    }
    if (commandReady)
    {
        RunCommand(pendingCommand);
        commandReady = 0;
    }
    if (saveNow)
    {
        saveNow = 0;
        nextSaveTime = now;
    }
    if (now < nextSaveTime)
        return;
    const int pad = gProfile->GetPrimaryPad();
    if (gApp.GetXuiAction(pad) != eXuiAction_Idle || gStorage->GetSaveState() != C4JStorage::ESaveGame_Idle)
        return;
    nextSaveTime = now + (unsigned long long)saveMinutes * MINUTE_NS;
    printf("[pc] server: saving the world (then every %d minutes)\n", saveMinutes);
    fflush(stdout);
    gApp.SetAction(pad, eXuiAction_AutoSave, 0);
}

#include "platform/pc/PcScripts.hpp"
#include "platform/pc/PcScriptsInternal.hpp"

#include "platform/pc/PcDedicated.hpp"
#include "platform/pc/PcInput.hpp"
#include "platform/pc/PcOs.hpp"

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "game/client/render/Font.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Gui.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CProfile.hpp"

using namespace PcScripts;

namespace
{
enum ClientEvent
{
    eClient_Tick,
    eClient_WorldJoin,
    eClient_WorldLeave,
    eClient_Hud,
    eClient_Chat,
    eClient_Key,
    eClient_Count
};
const char* const clientEvents[eClient_Count] = {
    "tick",
    "world_join",
    "world_leave",
    "hud",
    "chat",
    "key"};

const unsigned long long TICK_NS = 50000000ULL;
const unsigned int MAX_CATCH_UP = 5;
const size_t MAX_HUD_ITEMS = 1024;
const unsigned int MAX_CHAT = 100;

std::vector<State*> clientStates;
bool clientLoaded = false;
pthread_t mainThread;
bool wasInWorld = false;
unsigned long long nextTickTime = 0;
unsigned long long tickCount = 0;
bool showingOwnMessage = false;
std::vector<PcScriptHudItem>* hudItems = 0;
int hudWidth = 0;
int hudHeight = 0;

struct KeyName
{
    int key;
    const char* name;
};
const KeyName keyNames[] = {{0x08, "backspace"}, {0x09, "tab"},       {0x0D, "enter"},     {0x10, "shift"},
                             {0x11, "control"},   {0x12, "alt"},       {0x13, "pause"},     {0x14, "caps_lock"},
                             {0x1B, "escape"},    {0x20, "space"},     {0x21, "page_up"},   {0x22, "page_down"},
                             {0x23, "end"},       {0x24, "home"},      {0x25, "left"},      {0x26, "up"},
                             {0x27, "right"},     {0x28, "down"},      {0x2D, "insert"},    {0x2E, "delete"},
                             {0x6A, "multiply"},  {0x6B, "add"},       {0x6D, "subtract"},  {0x6E, "decimal"},
                             {0x6F, "divide"},    {0xBA, "semicolon"}, {0xBB, "equals"},    {0xBC, "comma"},
                             {0xBD, "minus"},     {0xBE, "period"},    {0xBF, "slash"},     {0xC0, "grave"},
                             {0xDB, "left_bracket"}, {0xDC, "backslash"}, {0xDD, "right_bracket"},
                             {0xDE, "apostrophe"}};

std::string KeyToName(int key)
{
    char buf[16];
    if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9'))
    {
        buf[0] = (char)(key >= 'A' && key <= 'Z' ? key - 'A' + 'a' : key);
        buf[1] = 0;
        return buf;
    }
    if (key >= 0x70 && key <= 0x87)
    {
        snprintf(buf, sizeof(buf), "f%d", key - 0x70 + 1);
        return buf;
    }
    if (key >= 0x60 && key <= 0x69)
    {
        snprintf(buf, sizeof(buf), "numpad%d", key - 0x60);
        return buf;
    }
    for (size_t i = 0; i < sizeof(keyNames) / sizeof(keyNames[0]); ++i)
    {
        if (keyNames[i].key == key)
            return keyNames[i].name;
    }
    snprintf(buf, sizeof(buf), "key_%d", key);
    return buf;
}

int NameToKey(const char* name)
{
    for (int k = 0; k < 256; ++k)
    {
        if (KeyToName(k) == name)
            return k;
    }
    return -1;
}

Minecraft* Game()
{
    return Minecraft::GetInstance();
}

MultiplayerLocalPlayer* LocalPlayer()
{
    Minecraft* mc = Game();
    if (mc == 0 || gProfile == 0)
        return 0;
    const int pad = gProfile->GetPrimaryPad();
    if (pad < 0 || pad >= 4)
        return 0;
    return mc->mLocalPlayers[pad].get();
}

bool Active()
{
    return !clientStates.empty() && pthread_equal(pthread_self(), mainThread);
}

bool InWorld()
{
    Minecraft* mc = Game();
    return mc != 0 && mc->mLevel != 0 && LocalPlayer() != 0;
}

bool UpdateInWorld()
{
    if (!wasInWorld)
        return InWorld() && gApp.GetGameStarted();
    return InWorld();
}

void ShowMessage(const std::wstring& text)
{
    Minecraft* mc = Game();
    if (mc == 0 || mc->mGui == 0 || !InWorld())
        return;
    showingOwnMessage = true;
    mc->mGui->addMessage(text, -1, false, false, false, true);
    showingOwnMessage = false;
}

std::wstring Clean(const std::wstring& s, size_t maxLen)
{
    std::wstring out;
    for (size_t i = 0; i < s.size() && out.size() < maxLen; ++i)
    {
        if (s[i] >= 0x20 && s[i] != 0x7F)
            out.push_back(s[i]);
    }
    return out;
}

int ClientInWorld(lua_State* L)
{
    lua_pushboolean(L, InWorld());
    return 1;
}

int ClientPlayerName(lua_State* L)
{
    MultiplayerLocalPlayer* p = InWorld() ? LocalPlayer() : 0;
    if (p == 0)
        return 0;
    PushWide(L, p->getName());
    return 1;
}

int ClientPosition(lua_State* L)
{
    MultiplayerLocalPlayer* p = InWorld() ? LocalPlayer() : 0;
    if (p == 0)
        return 0;
    lua_pushnumber(L, p->mX);
    lua_pushnumber(L, p->mY);
    lua_pushnumber(L, p->mZ);
    return 3;
}

int ClientDimension(lua_State* L)
{
    MultiplayerLocalPlayer* p = InWorld() ? LocalPlayer() : 0;
    if (p == 0)
        return 0;
    lua_pushinteger(L, p->mDimension);
    return 1;
}

int ClientMessage(lua_State* L)
{
    luaL_checkany(L, 1);
    lua_settop(L, 1);
    luaL_tolstring(L, 1, 0);
    ShowMessage(Clean(ArgWide(L, -1), 1024));
    return 0;
}

int ClientSay(lua_State* L)
{
    luaL_checkany(L, 1);
    lua_settop(L, 1);
    luaL_tolstring(L, 1, 0);
    MultiplayerLocalPlayer* p = InWorld() ? LocalPlayer() : 0;
    if (p == 0)
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    const std::wstring text = Clean(ArgWide(L, -1), MAX_CHAT);
    if (!text.empty())
        p->chat(text);
    lua_pushboolean(L, !text.empty());
    return 1;
}

int ClientBlock(lua_State* L)
{
    const int x = (int)luaL_checkinteger(L, 1);
    const int y = (int)luaL_checkinteger(L, 2);
    const int z = (int)luaL_checkinteger(L, 3);
    if (!InWorld())
        return 0;
    MultiPlayerLevel* lvl = Game()->mLevel;
    const BlockPos pos(x, y, z);
    if (y < 0 || y >= lvl->getMaxBuildHeight() || !lvl->hasChunkAt(pos))
        return 0;
    const BlockState* bs = lvl->getBlockState(pos);
    PushWide(L, Block::blockRegistry->getKey(bs->getBlock()).toString());
    lua_pushinteger(L, Block::getIdWithData(bs) >> 12);
    return 2;
}

int ClientKeyDown(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);
    const int k = NameToKey(name);
    if (k < 0)
        return luaL_error(L, "there is no key \"%s\"", name);
    lua_pushboolean(L, PcInputKeyHeld(k));
    return 1;
}

void CheckHud(lua_State* L)
{
    if (hudItems == 0)
        luaL_error(L, "hud drawing only works inside a \"hud\" event handler");
}

int HudText(lua_State* L)
{
    CheckHud(L);
    const int x = (int)luaL_checknumber(L, 1);
    const int y = (int)luaL_checknumber(L, 2);
    luaL_checkany(L, 3);
    const unsigned int col = ArgColour(L, 4, 0xFFFFFFFFu);
    const bool shadow = lua_isnoneornil(L, 5) ? true : lua_toboolean(L, 5) != 0;
    lua_settop(L, 3);
    luaL_tolstring(L, 3, 0);
    if (hudItems->size() >= MAX_HUD_ITEMS)
        return 0;
    hudItems->push_back(PcScriptHudItem());
    PcScriptHudItem& item = hudItems->back();
    item.text = true;
    item.shadow = shadow;
    item.x1 = x;
    item.y1 = y;
    item.x2 = x;
    item.y2 = y;
    item.colour = col;
    item.string = Clean(ArgWide(L, -1), 512);
    return 0;
}

int HudRect(lua_State* L)
{
    CheckHud(L);
    const int x1 = (int)luaL_checknumber(L, 1);
    const int y1 = (int)luaL_checknumber(L, 2);
    const int x2 = (int)luaL_checknumber(L, 3);
    const int y2 = (int)luaL_checknumber(L, 4);
    const unsigned int col = ArgColour(L, 5, 0x80000000u);
    if (hudItems->size() >= MAX_HUD_ITEMS)
        return 0;
    hudItems->push_back(PcScriptHudItem());
    PcScriptHudItem& item = hudItems->back();
    item.text = false;
    item.shadow = false;
    item.x1 = x1;
    item.y1 = y1;
    item.x2 = x2;
    item.y2 = y2;
    item.colour = col;
    return 0;
}

int HudTextWidth(lua_State* L)
{
    luaL_checkany(L, 1);
    lua_settop(L, 1);
    luaL_tolstring(L, 1, 0);
    Minecraft* mc = Game();
    if (mc == 0 || mc->mFont == 0)
        return 0;
    lua_pushinteger(L, mc->mFont->width(Clean(ArgWide(L, -1), 512)));
    return 1;
}

int HudSize(lua_State* L)
{
    CheckHud(L);
    lua_pushinteger(L, hudWidth);
    lua_pushinteger(L, hudHeight);
    return 2;
}

void OpenClientApi(lua_State* L)
{
    static const luaL_Reg clientFuncs[] = {
        {"in_world", ClientInWorld},
        {"player_name", ClientPlayerName},
        {"position", ClientPosition},
        {"dimension", ClientDimension},
        {"message", ClientMessage},
        {"say", ClientSay},
        {"block", ClientBlock},
        {"key_down", ClientKeyDown},
        {0, 0}};
    lua_newtable(L);
    SetFunctions(L, clientFuncs);
    lua_setglobal(L, "client");
    static const luaL_Reg hudFuncs[] = {
        {"text", HudText},
        {"rect", HudRect},
        {"text_width", HudTextWidth},
        {"size", HudSize},
        {0, 0}};
    lua_newtable(L);
    SetFunctions(L, hudFuncs);
    lua_setglobal(L, "hud");
}

class SimpleEvent : public Event
{
public:
    SimpleEvent(int id) : Event(id) {}
    virtual int Push(lua_State*) { return 0; }
};

class TickEvent : public Event
{
public:
    TickEvent() : Event(eClient_Tick) {}
    virtual int Push(lua_State* L)
    {
        lua_pushinteger(L, (lua_Integer)tickCount);
        return 1;
    }
};

class KeyEvent : public Event
{
public:
    KeyEvent(int key) : Event(eClient_Key), mName(KeyToName(key)) {}
    virtual int Push(lua_State* L)
    {
        lua_pushstring(L, mName.c_str());
        return 1;
    }
    std::string mName;
};

class HudEvent : public Event
{
public:
    HudEvent() : Event(eClient_Hud) {}
    virtual int Push(lua_State* L)
    {
        lua_pushinteger(L, hudWidth);
        lua_pushinteger(L, hudHeight);
        return 2;
    }
};

class ClientChatEvent : public ChatEvent
{
public:
    ClientChatEvent(const std::wstring& text) : ChatEvent(eClient_Chat, text) {}
    virtual int Push(lua_State* L)
    {
        PushWide(L, mText);
        return 1;
    }
};

class CommandEvent : public Event
{
public:
    CommandEvent(const std::wstring& arguments) : Event(0), mArguments(arguments) {}
    virtual int Push(lua_State* L)
    {
        PushWide(L, mArguments);
        return 1;
    }
    virtual bool Result(lua_State* L, int idx)
    {
        if (lua_type(L, idx) == LUA_TSTRING)
        {
            size_t len;
            const char* text = lua_tolstring(L, idx, &len);
            mReply = Wide(text, len);
        }
        return true;
    }
    std::wstring mArguments;
    std::wstring mReply;
};

void ShowErrors()
{
    for (size_t i = 0; i < clientStates.size(); ++i)
    {
        State* s = clientStates[i];
        if (s->errors == 0 || s->errorShown || !InWorld())
            continue;
        s->errorShown = true;
        const std::string text = "The mod " + s->modName + " has a script error (see latest.log).";
        ShowMessage(Wide(text.data(), text.size()));
    }
}
}

void PcScriptsClientLoad()
{
    if (clientLoaded || PcIsDedicatedServer())
        return;
    clientLoaded = true;
    mainThread = pthread_self();
    LoadStates(eSide_Client, clientEvents, eClient_Count, OpenClientApi, &clientStates);
}

void PcScriptsClientFrame(Minecraft* mc)
{
    if (!Active())
    {
        while (PcInputConsumeScriptKey() >= 0)
        {
        }
        return;
    }
    const bool inWorld = UpdateInWorld();
    if (inWorld != wasInWorld)
    {
        wasInWorld = inWorld;
        SimpleEvent ev(inWorld ? eClient_WorldJoin : eClient_WorldLeave);
        Dispatch(clientStates, &ev);
    }
    for (int k = PcInputConsumeScriptKey(); k >= 0; k = PcInputConsumeScriptKey())
    {
        if (!inWorld || !AnyHandles(clientStates, eClient_Key))
            continue;
        KeyEvent ev(k);
        Dispatch(clientStates, &ev);
    }
    const unsigned long long now = PcOsMonotonicNanoseconds();
    if (nextTickTime == 0 || now > nextTickTime + TICK_NS * MAX_CATCH_UP)
        nextTickTime = now;
    for (unsigned int n = 0; now >= nextTickTime && n < MAX_CATCH_UP; ++n)
    {
        nextTickTime += TICK_NS;
        ++tickCount;
        if (!AnyHandles(clientStates, eClient_Tick))
            continue;
        TickEvent ev;
        Dispatch(clientStates, &ev);
    }
    ShowErrors();
}

bool PcScriptsClientChat(std::wstring* msg)
{
    if (showingOwnMessage || !Active() || !AnyHandles(clientStates, eClient_Chat))
        return true;
    ClientChatEvent ev(*msg);
    Dispatch(clientStates, &ev);
    if (ev.mCancelled)
        return false;
    *msg = Clean(ev.mText, 1024);
    return !msg->empty();
}

bool PcScriptsClientCommand(const std::wstring& line)
{
    if (!Active() || line.empty() || line[0] != L'/')
        return false;
    std::wstring::size_type space = line.find(L' ');
    std::wstring name = line.substr(1, space == std::wstring::npos ? std::wstring::npos : space - 1);
    for (size_t i = 0; i < name.size(); ++i)
    {
        if (name[i] >= L'A' && name[i] <= L'Z')
            name[i] = (wchar_t)(name[i] + 32);
    }
    std::wstring args = space == std::wstring::npos ? std::wstring() : line.substr(space + 1);
    const std::wstring::size_type start = args.find_first_not_of(L' ');
    args = start == std::wstring::npos ? std::wstring() : args.substr(start);
    CommandEvent ev(args);
    if (!RunCommand(eSide_Client, Utf8(name), &ev))
        return false;
    if (!ev.mReply.empty())
        ShowMessage(Clean(ev.mReply, 1024));
    return true;
}

bool PcScriptsClientHasHud()
{
    return Active() && AnyHandles(clientStates, eClient_Hud);
}

void PcScriptsClientHud(int w, int h, std::vector<PcScriptHudItem>* out)
{
    out->clear();
    if (!Active() || !AnyHandles(clientStates, eClient_Hud))
        return;
    hudItems = out;
    hudWidth = w;
    hudHeight = h;
    HudEvent ev;
    Dispatch(clientStates, &ev);
    hudItems = 0;
}

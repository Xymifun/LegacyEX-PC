#include "game/core/CConsoleMinecraftApp.hpp"
#include "platform/pc/PcOnlineConnection.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/GameSessionData.hpp"

#include "platform/pc/PcLanTransport.hpp"
#include "platform/pc/PcLobbyClient.hpp"
#include "platform/pc/PcLobbyProtocol.hpp"
#include "platform/pc/PcNetworkSession.hpp"
#include "platform/pc/PcOs.hpp"
#include "platform/pc/PcPaths.hpp"

#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>

namespace
{
const char OLD_CONNECTION_FILE[] = "online-host.txt";
const char LOBBY_ADDRESS_FILE[] = "online-lobby.txt";
const unsigned long long kPollNanoseconds = 1000ULL * 1000000ULL;
const unsigned long long RESOLVE_RETRY_NS = 60000ULL * 1000000ULL;

bool loaded = false;
std::string rememberedLobby;
std::string s_address;
std::string hostName;
std::vector<unsigned short> s_ports;
unsigned int s_resolved = 0;
unsigned long long resolvedAt = 0;
unsigned long long s_lastPoll = 0;
bool s_attempted = false;
EPcOnlineStatus s_status = ePcOnline_Off;

std::string Trim(const std::string& s)
{
    std::string::size_type start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return std::string();
    std::string::size_type stop = s.find_last_not_of(" \t\r\n");
    return s.substr(start, stop - start + 1);
}

void Apply(const std::string& addr)
{
    s_address = Trim(addr);
    hostName = s_address;
    s_ports.clear();
    std::string::size_type colon = hostName.rfind(':');
    if (colon != std::string::npos)
    {
        int p = atoi(hostName.c_str() + colon + 1);
        if (p > 0 && p < 65536)
            s_ports.push_back((unsigned short)p);
        hostName.erase(colon);
    }
    if (s_ports.empty())
    {
        s_ports.push_back(kLobbyPort);
        s_ports.push_back(kLobbyAlternatePort);
    }
    PcLobbyClientStop();
    PcLobbyClientSetWanted(0, s_ports);
    s_resolved = 0;
    resolvedAt = 0;
    s_lastPoll = 0;
    s_attempted = false;
    s_status = s_address.empty() ? ePcOnline_Off : ePcOnline_Connecting;
}

std::string LobbyAddressPath()
{
    return PcSaveRoot() + "/" + LOBBY_ADDRESS_FILE;
}

void RememberLobby()
{
    if (rememberedLobby == s_address)
        return;
    rememberedLobby = s_address;
    if (FILE* fp = fopen(LobbyAddressPath().c_str(), "w"))
    {
        fprintf(fp, "%s\n", s_address.c_str());
        fclose(fp);
    }
}

void ForgetLobby()
{
    if (rememberedLobby.empty())
        return;
    rememberedLobby.clear();
    remove(LobbyAddressPath().c_str());
}

void Load()
{
    if (loaded)
        return;
    loaded = true;
    remove(OLD_CONNECTION_FILE);
    if (FILE* fp = fopen(LobbyAddressPath().c_str(), "r"))
    {
        char buf[256] = "";
        if (fgets(buf, sizeof(buf), fp) != 0)
            rememberedLobby = Trim(buf);
        fclose(fp);
    }
    if (!rememberedLobby.empty())
    {
        Apply(rememberedLobby);
        printf("[pc] online: connecting to %s again (connected when the game last closed)\n", s_address.c_str());
        fflush(stdout);
    }
}

bool Resolve(unsigned long long now)
{
    if (s_resolved != 0)
        return true;
    if (resolvedAt != 0 && now - resolvedAt < RESOLVE_RETRY_NS)
        return false;
    resolvedAt = now;
    unsigned int addr = 0;
    if (!PcOsNetInitialise()
        || !(PcOsParseIPv4(hostName.c_str(), &addr) || PcOsResolveIPv4(hostName.c_str(), &addr)))
    {
        printf("[pc] online: could not resolve %s\n", hostName.c_str());
        fflush(stdout);
        return false;
    }
    s_resolved = addr;
    PcLobbyClientSetWanted(s_resolved, s_ports);
    return true;
}
}

void PcOnlineConnectionSetAddress(const std::string& addr)
{
    Load();
    Apply(addr);
    if (s_address != rememberedLobby)
        ForgetLobby();
    printf(s_address.empty() ? "[pc] online: disconnected\n" : "[pc] online: connecting to %s\n", s_address.c_str());
    fflush(stdout);
}

const std::string& PcOnlineConnectionAddress()
{
    Load();
    return s_address;
}

EPcOnlineStatus PcOnlineConnectionStatus()
{
    Load();
    return s_status;
}

std::wstring PcOnlineMenuLabel()
{
    switch (PcOnlineConnectionStatus())
    {
    case ePcOnline_Connected:
        return L"<font color=\"#55FF55\">[CONNECTED]</font>";
    case ePcOnline_Connecting:
        return L"[CONNECTING...]";
    case ePcOnline_Linking:
    {
        std::wstring code;
        std::wstring text;
        PcOnlineConnectionLinkCode(&code, &text);
        return L"<font color=\"#FFFF55\">[VERIFY CODE: " + code + L"]</font>";
    }
    case ePcOnline_WaitingForHost:
        return L"[WAITING FOR HOST]";
    case ePcOnline_Refused:
        return L"[NOT ALLOWED]";
    default:
        return L"Online Multiplayer";
    }
}

bool PcOnlineConnectionIsLobby(unsigned short* port)
{
    Load();
    for (size_t i = 0; i < s_ports.size(); ++i)
    {
        if (s_resolved != 0 && PcLobbyClientRunning(s_resolved, s_ports[i]))
        {
            if (port != 0)
                *port = s_ports[i];
            return true;
        }
    }
    return false;
}

std::wstring PcOnlineConnectionRefusal()
{
    return PcLobbyClientRefusal();
}

void PcOnlineConnectionLinkCode(std::wstring* code, std::wstring* text)
{
    PcLobbyClientLinkCode(code, text);
}

void PcOnlineConnectionTick()
{
    Load();
    PcOsSocket relayed = kPcOsInvalidSocket;
    unsigned int relayedAddress = 0;
    PcVerifiedIdentities verified;
    while (PcLobbyClientTakeRelayed(&relayed, &relayedAddress, &verified))
    {
        if (!PcNetworkSession::AcceptRelayedConnection(relayed, relayedAddress, verified.count > 0 ? &verified : 0))
            PcOsSocketClose(relayed);
    }
    if (s_address.empty())
        return;
    unsigned long long now = PcOsMonotonicNanoseconds();
    if (s_lastPoll != 0 && now - s_lastPoll < kPollNanoseconds)
        return;
    s_lastPoll = now;
    std::vector<unsigned char> hosted;
    PcNetworkSession::HostedSessionData(&hosted);
    unsigned char visibility = kVisibilityPublic;
    if (hosted.size() >= sizeof(GameSessionData))
    {
        GameSessionData data;
        std::memcpy(static_cast<void*>(&data), &hosted[0], sizeof(GameSessionData));
        if (!data.m_bPublicGame)
            visibility = gApp.GetGameHostOption(eGameHostOption_FriendsOfFriends) != 0 ? kVisibilityFriendsOfFriends
                                                                                       : kVisibilityFriends;
    }
    PcLobbyClientSetHosted(hosted, visibility);
    if (!Resolve(now))
    {
        s_status = ePcOnline_WaitingForHost;
        return;
    }
    bool found = false;
    bool running = false;
    bool failed = true;
    for (size_t index = 0; index < s_ports.size(); ++index)
    {
        const unsigned short port = s_ports[index];
        if (PcLobbyClientRunning(s_resolved, port))
        {
            switch (PcLobbyClientState())
            {
            case ePcLobby_Connected:
                s_status = ePcOnline_Connected;
                RememberLobby();
                return;
            case ePcLobby_Refused:
                s_status = ePcOnline_Refused;
                return;
            case ePcLobby_Linking:
                s_status = ePcOnline_Linking;
                return;
            default:
                running = true;
                failed = false;
                continue;
            }
        }
        const PcLanDiscovery::ProbeStatus previous = PcNetworkSession::DirectHostStatus(s_resolved, port);
        const bool anyState =
            previous == PcLanDiscovery::eProbeStatus_None || previous == PcLanDiscovery::eProbeStatus_Lobby;
        if (PcNetworkSession::WatchDirectHost(s_resolved, port, anyState))
            s_attempted = true;
        switch (PcNetworkSession::DirectHostStatus(s_resolved, port))
        {
        case PcLanDiscovery::eProbeStatus_Found:
            found = true;
            failed = false;
            break;
        case PcLanDiscovery::eProbeStatus_Failed:
            break;
        default:
            running = true;
            failed = false;
            break;
        }
    }
    if (found)
        s_status = ePcOnline_Connected;
    else if (failed)
        s_status = ePcOnline_WaitingForHost;
    else if (running && (s_status != ePcOnline_WaitingForHost || !s_attempted))
        s_status = ePcOnline_Connecting;
}

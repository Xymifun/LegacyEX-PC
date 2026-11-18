#ifndef PLATFORM_PC_PCLOBBYCLIENT_HPP
#define PLATFORM_PC_PCLOBBYCLIENT_HPP

#include "platform/pc/PcOs.hpp"
#include "platform/pc/PcVerifiedIdentities.hpp"

#include <string>
#include <vector>

enum EPcLobbyState
{
    ePcLobby_Off,
    ePcLobby_Connecting,
    ePcLobby_Linking,
    ePcLobby_Connected,
    ePcLobby_Refused
};

struct PcLobbyWorld
{
    PcLobbyWorld() : id(0), flags(0) {}
    unsigned int id;
    unsigned char flags;
    std::vector<unsigned char> sessionData;
};

struct PcLobbyFriend
{
    std::wstring name;
    std::wstring discord;
    std::string id;
    unsigned int skin;
    bool online;
};

void PcLobbyClientStart(unsigned int address, unsigned short port, PcOsSocket socket, unsigned long long stationId,
                        const std::vector<unsigned char>& lobbyHello);
bool PcLobbyClientRunning(unsigned int address, unsigned short port);
void PcLobbyClientStop();
void PcLobbyClientSetWanted(unsigned int address, const std::vector<unsigned short>& ports);
bool PcLobbyClientWanted(unsigned int address, unsigned short port);

EPcLobbyState PcLobbyClientState();
std::wstring PcLobbyClientRefusal();
void PcLobbyClientLinkCode(std::wstring* code, std::wstring* text);
unsigned int PcLobbyClientRevision();
std::vector<PcLobbyWorld> PcLobbyClientWorlds(unsigned int* address, unsigned short* port,
                                              unsigned long long* secret);
std::vector<PcLobbyFriend> PcLobbyClientFriends(unsigned int* revision);
std::vector<PcLobbyFriend> PcLobbyClientRequests(unsigned int* revision);
void PcLobbyClientAnswerRequest(const std::string& id, bool accept);
void PcLobbyClientRemoveFriend(const std::string& id);
void PcLobbyClientSendFriendRequest(const unsigned char uuid[16]);
bool PcLobbyClientTakeNotice(std::wstring* text);
void PcLobbyClientSetHosted(const std::vector<unsigned char>& sessionData, unsigned char visibility);
bool PcLobbyClientTakeRelayed(PcOsSocket* socket, unsigned int* address, PcVerifiedIdentities* verified);

#endif

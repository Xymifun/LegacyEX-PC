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
void PcLobbyClientSetHosted(const std::vector<unsigned char>& sessionData, unsigned char visibility);
bool PcLobbyClientTakeRelayed(PcOsSocket* socket, unsigned int* address, PcVerifiedIdentities* verified);

#endif

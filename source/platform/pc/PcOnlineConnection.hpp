#ifndef PLATFORM_PC_PCONLINECONNECTION_HPP
#define PLATFORM_PC_PCONLINECONNECTION_HPP

#include <string>

enum EPcOnlineStatus
{
    ePcOnline_Off,
    ePcOnline_Connecting,
    ePcOnline_Linking,
    ePcOnline_Connected,
    ePcOnline_WaitingForHost,
    ePcOnline_Refused
};

void PcOnlineConnectionSetAddress(const std::string& address);
const std::string& PcOnlineConnectionAddress();
EPcOnlineStatus PcOnlineConnectionStatus();
std::wstring PcOnlineMenuLabel();
void PcOnlineConnectionTick();
bool PcOnlineConnectionIsLobby(unsigned short* port);
std::wstring PcOnlineConnectionRefusal();
void PcOnlineConnectionLinkCode(std::wstring* code, std::wstring* text);

#endif

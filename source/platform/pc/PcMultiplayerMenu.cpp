#include "platform/pc/PcMultiplayerMenu.hpp"

#include "platform/pc/PcOnlineConnection.hpp"
#include "platform/pc/PcOs.hpp"

#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/StringIds.hpp"
#include "platform/wiiu/CInput.hpp"

#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include <string>
#include <vector>

static void OpenHostingBox(int pad);

namespace
{
const char kServerListPath[] = "servers.txt";
const unsigned int kAddressLength = 63;
const size_t kMaxListedAddresses = 4;

wchar_t s_message[1024];
std::wstring s_lastAddress;
int s_pad = 0;
bool s_showAddresses = false;
bool s_disconnectShown = false;
int s_alertFrames = 0;
int s_alertTitle = 0;
int s_alertText = 0;
int s_alertPad = 0;
int s_alertParameter = 0;

std::string Trim(const std::string& text)
{
    std::string::size_type first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return std::string();
    std::string::size_type last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

std::vector<std::string> SavedAddresses()
{
    std::vector<std::string> addresses;
    FILE* file = fopen(kServerListPath, "r");
    if (file == 0)
        return addresses;
    char line[256];
    while (fgets(line, sizeof(line), file) != 0)
    {
        std::string text(line);
        std::string::size_type comment = text.find('#');
        if (comment != std::string::npos)
            text.erase(comment);
        text = Trim(text);
        if (!text.empty())
            addresses.push_back(text);
    }
    fclose(file);
    return addresses;
}

void RememberAddress(const std::string& address)
{
    std::vector<std::string> lines;
    FILE* file = fopen(kServerListPath, "r");
    if (file != 0)
    {
        char line[256];
        while (fgets(line, sizeof(line), file) != 0)
        {
            std::string text(line);
            std::string entry = text;
            std::string::size_type comment = entry.find('#');
            if (comment != std::string::npos)
                entry.erase(comment);
            if (Trim(entry) == address && comment == std::string::npos)
                continue;
            if (!text.empty() && text[text.size() - 1] != '\n')
                text.push_back('\n');
            lines.push_back(text);
        }
        fclose(file);
    }
    lines.push_back(address + "\n");
    file = fopen(kServerListPath, "w");
    if (file == 0)
        return;
    for (size_t index = 0; index < lines.size(); ++index)
        fputs(lines[index].c_str(), file);
    fclose(file);
}

int AddressEntered(void*, bool accepted)
{
    if (!accepted)
        return 0;
    wchar_t text[kAddressLength + 1];
    memset(text, 0, sizeof(text));
    g_pInput->GetText(text, kAddressLength);
    std::string address;
    for (const wchar_t* character = text; *character != 0; ++character)
    {
        if (*character > 0x20 && *character < 0x7F)
            address.push_back(static_cast<char>(*character));
    }
    s_lastAddress = std::wstring(address.begin(), address.end());
    if (!address.empty())
        RememberAddress(address);
    PcOnlineConnectionSetAddress(address);
    return 0;
}

int HostingReturned(void*, int, C4JStorage::EMessageResult result)
{
    const C4JStorage::EMessageResult toggleAddresses =
        s_disconnectShown ? C4JStorage::EMessageResult_ThirdOption : C4JStorage::EMessageResult_Decline;
    if (s_disconnectShown && result == C4JStorage::EMessageResult_Decline)
    {
        PcOnlineConnectionSetAddress(std::string());
        OpenHostingBox(s_pad);
        return 0;
    }
    if (result == toggleAddresses)
    {
        s_showAddresses = !s_showAddresses;
        OpenHostingBox(s_pad);
        return 0;
    }
    if (result != C4JStorage::EMessageResult_Accept)
        return 0;
    g_pInput->RequestKeyboard(L"Host Address", s_lastAddress.c_str(), s_pad, kAddressLength,
                              AddressEntered, 0,
                              C_4JInput::EKeyboardMode_0);
    return 0;
}

std::wstring LocalAddresses()
{
    unsigned int addresses[16];
    size_t count = PcOsListIPv4Addresses(addresses, sizeof(addresses) / sizeof(addresses[0]));
    std::wstring text;
    for (size_t index = 0; index < count && index < kMaxListedAddresses; ++index)
    {
        char formatted[32];
        PcOsFormatIPv4(addresses[index], formatted, sizeof(formatted));
        if (!s_showAddresses)
        {
            for (char* character = formatted; *character != 0; ++character)
            {
                if (*character != '.')
                    *character = 'X';
            }
        }
        if (!text.empty())
            text += L", ";
        text += std::wstring(formatted, formatted + strlen(formatted));
    }
    return text.empty() ? std::wstring(L"(no network found)") : text;
}
}

void PcOpenMultiplayerHosting(int pad)
{
    s_showAddresses = false;
    OpenHostingBox(pad);
}

static void OpenHostingBox(int pad)
{
    s_pad = pad;
    const std::string& connected = PcOnlineConnectionAddress();
    if (!connected.empty())
        s_lastAddress = std::wstring(connected.begin(), connected.end());
    else if (s_lastAddress.empty())
    {
        std::vector<std::string> saved = SavedAddresses();
        if (!saved.empty())
            s_lastAddress = std::wstring(saved.back().begin(), saved.back().end());
    }
    std::wstring connection;
    std::wstring connectedText(connected.begin(), connected.end());
    switch (PcOnlineConnectionStatus())
    {
    case ePcOnline_Connected:
        if (PcOnlineConnectionIsLobby(0))
            connection = L"<font color=\"#1E8C1E\">Connected to the " + connectedText
                         + L" session</font>. Everyone's Online worlds there are under Play Game, in the Join "
                           L"tab, and while you host one with Online Game on, it is listed for them.";
        else
            connection = L"<font color=\"#1E8C1E\">Connected to " + connectedText
                         + L"</font>. The Host's world is under Play Game, in the Join tab (or in Minigames)";
        break;
    case ePcOnline_Refused:
        connection = connectedText + L" did not let this game in: " + PcOnlineConnectionRefusal();
        break;
    case ePcOnline_Connecting:
        connection = L"Connecting to " + connectedText + L"...";
        break;
    case ePcOnline_Linking:
    {
        std::wstring code;
        std::wstring text;
        PcOnlineConnectionLinkCode(&code, &text);
        connection = connectedText + L" ties each player to a Discord account. Your code: <font color=\"#1E8C1E\">"
                     + code + L"</font>. " + text;
        break;
    }
    case ePcOnline_WaitingForHost:
        connection = L"Waiting for " + connectedText + L": the Host has not loaded a world with Online Game on, or "
                     L"cannot be reached. This keeps trying.";
        break;
    default:
        connection = L"Not connected to a Host.";
        break;
    }
    swprintf(s_message, sizeof(s_message) / sizeof(s_message[0]),
             L"HOST: Load or create a world (with Online Game on). Friends on your network or a virtual LAN "
             L"(like Tailscale) join you at any of the Local IPs <font color=\"#1E8C1E\">%ls</font>; over the "
             L"internet, forward TCP port 25565. Give the Guest(s) your Local IP.<br><br>"
             L"GUEST: Connect to the Host's address, then join their world from Play Game, in the Join tab (or in Minigames).<br><br>"
             L"%ls",
             LocalAddresses().c_str(), connection.c_str());
    s_disconnectShown = !connected.empty();
    int parameters[4];
    unsigned int count = 0;
    parameters[count++] = static_cast<int>(ePcString_ConnectToHost);
    if (s_disconnectShown)
        parameters[count++] = static_cast<int>(ePcString_Disconnect);
    parameters[count++] = static_cast<int>(s_showAddresses ? ePcString_HideLocalIp : ePcString_ShowLocalIp);
    parameters[count++] = static_cast<int>(IDS_CONFIRM_CANCEL);
    gConsoleUIController.RequestAlertMessage(IDS_ONLINE_GAME, IDS_ONLINE_GAME, parameters, count, pad, HostingReturned,
                                             0, s_message, true);
}

void PcShowAlertSoon(int titleId, int textId, int pad)
{
    s_alertTitle = titleId;
    s_alertText = textId;
    s_alertPad = pad;
    s_alertFrames = 2;
}

void PcMultiplayerMenuTick()
{
    if (s_alertFrames == 0 || --s_alertFrames > 0)
        return;
    s_alertParameter = static_cast<int>(IDS_CONFIRM_OK);
    gConsoleUIController.RequestAlertMessage(s_alertTitle, s_alertText, &s_alertParameter, 1, s_alertPad, 0, 0, 0,
                                             true);
}

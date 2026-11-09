#ifndef GAME_CORE_L10N_HPP
#define GAME_CORE_L10N_HPP

#ifdef MINECRAFT_PC
enum PcStringId
{
    ePcString_ShowLocalIp = 0x7FC0FF01,
    ePcString_HideLocalIp = 0x7FC0FF02,
    ePcString_ConnectToHost = 0x7FC0FF03,
    ePcString_Disconnect = 0x7FC0FF04,
    ePcString_ToggleGhost = 0x7FC0FF05,
    ePcString_FriendsOnly = 0x7FC0FF06,
    ePcString_FriendsOnlyDescription = 0x7FC0FF07,
    ePcString_Massive = 0x7FC0FF08,
    ePcString_ServerLabel = 0x7FC0FF09,
    ePcString_PublicWorldTitle = 0x7FC0FF0A,
    ePcString_PublicWorldTitleDescription = 0x7FC0FF0B,
    ePcString_MismatchedContent = 0x7FC0FF0C,
    ePcString_Mute = 0x7FC0FF0D,
    ePcString_Unmute = 0x7FC0FF0E,
    ePcString_Unbind = 0x7FC0FF0F,
    ePcString_ProximityChatDescription = 0x7FC0FF10
};
#endif

class L10N
{
public:
    static const wchar_t* GetString(int id);
    static void loadStringTable();
};

#endif

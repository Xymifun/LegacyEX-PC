#include "game/core/L10N.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/data/StringTable.hpp"

static StringTable* g_stringTable = NULL;
extern CConsoleMinecraftApp gApp;

#ifdef MINECRAFT_PC
#include "platform/pc/PcContent.hpp"
#include "platform/pc/PcModContent.hpp"

static void ReplaceAll(std::wstring& text, const wchar_t* from, const wchar_t* to)
{
    const std::wstring needle(from);
    const std::wstring replacement(to);
    std::wstring::size_type position = 0;
    while ((position = text.find(needle, position)) != std::wstring::npos)
    {
        text.replace(position, needle.size(), replacement);
        position += replacement.size();
    }
}

static void RewritePlatformName(std::wstring& text)
{
    if (text.find(L"Wii") == std::wstring::npos)
        return;
    static const wchar_t* const replacements[][2] = {
        {L"Wii\u00a0U", L"Wii U"},
        {L"on the Wii U GamePad touch screen", L"on a second screen"},
        {L"Minecraft: Wii U Edition", L"Minecraft: Legacy Edition"},
        {L"Minecraft Wii U Edition", L"Minecraft: Legacy Edition"},
        {L"Wii U Edition", L"Legacy Edition"},
        {L"the Wii U console game", L"Legacy Edition"},
        {L"Minecraft on the Wii U console", L"Minecraft: Legacy Edition"},
        {L"your Wii U console", L"your PC"},
        {L"their Wii U console", L"their PC"},
        {L"same Wii U console", L"same PC"},
        {L" portion of the Wii U Menu", L""},
        {L"from Wii U", L"from Legacy Edition"},
        {L"Wii U", L"Legacy Edition"},
    };
    for (unsigned int i = 0; i < sizeof(replacements) / sizeof(replacements[0]); ++i)
        ReplaceAll(text, replacements[i][0], replacements[i][1]);
}
#endif

const wchar_t* L10N::GetString(int id)
{
#ifdef MINECRAFT_PC
    if (const wchar_t* modString = PcModContentString(id))
        return modString;
    if (id == ePcString_ShowLocalIp)
        return L"Show Local IP";
    if (id == ePcString_HideLocalIp)
        return L"Hide Local IP";
    if (id == ePcString_ConnectToHost)
        return L"Connect to Host / Session";
    if (id == ePcString_Disconnect)
        return L"Disconnect";
    if (id == ePcString_ToggleGhost)
        return L"Toggle Ghost";
    if (id == ePcString_FriendsOnly)
        return L"Friends Only";
    if (id == ePcString_Massive)
        return L"Massive";
    if (id == ePcString_ServerLabel)
        return L"(Server)";
    if (id == ePcString_PublicWorldTitle)
        return L"Public World Title";
    if (id == ePcString_PublicWorldTitleDescription)
        return L"When enabled, players can see the name of your Online World alongside your username.";
    if (id == ePcString_MismatchedContent)
        return PcContentMismatchText();
    if (id == ePcString_Mute)
        return L"Mute";
    if (id == ePcString_Unmute)
        return L"Unmute";
    if (id == ePcString_Unbind)
        return L"Unbind";
    if (id == ePcString_ProximityChatDescription)
        return L"When enabled, players will automatically have Proximity Chat turned on.";
    if (id == IDS_VOICE_CHAT)
        return L"Voice Chat";
    if (id == IDS_IN_GAME_GAMERTAGS)
        return L"In-Game Username";
    if (id == STRING_MII_ALREADY_IN_USE)
        return L"Someone using your player identity is already in this game.";
    if (id == ePcString_FriendsOnlyDescription)
        return L"When enabled, only your friends can see and join this world through the online lobby. Add "
               L"friends with /friend in the Discord Server!";
#endif
    if (g_stringTable == NULL)
        return L"";
    return g_stringTable->getString(id);
}

void L10N::loadStringTable()
{
    delete g_stringTable;
    std::wstring path = L"languages.loc";
    if (gApp.hasArchiveFile(path, true))
    {
        arrayWithLength<unsigned char> data = gApp.getArchiveFile(path, true);
        g_stringTable = new StringTable(data, false, false);
        delete[] data.data;
#ifdef MINECRAFT_PC
        g_stringTable->RewriteStrings(RewritePlatformName);
#endif
    }
    else
    {
        g_stringTable = NULL;
    }
}

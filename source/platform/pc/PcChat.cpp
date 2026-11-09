#include "platform/pc/PcChat.hpp"

#include "platform/pc/PcInput.hpp"
#include "platform/pc/PcScripts.hpp"

#include <vector>

#include "game/core/Minecraft.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

namespace
{
const unsigned int CHAT_LENGTH = 100;
const size_t HISTORY_LENGTH = 100;

bool CHAT_OPEN = false;
std::vector<std::u16string> HISTORY;
size_t HISTORY_INDEX = 0;
std::u16string DRAFT;

void Close()
{
    if (CHAT_OPEN)
        PcTextEntryEnd();
    CHAT_OPEN = false;
    PcInputSetChatOpen(false);
}

std::wstring Trimmed(const std::wstring& s)
{
    std::wstring::size_type first = s.find_first_not_of(L' ');
    if (first == std::wstring::npos)
        return std::wstring();
    std::wstring::size_type last = s.find_last_not_of(L' ');
    return s.substr(first, last - first + 1);
}

void Remember(const std::wstring& msg)
{
    std::u16string ln(msg.begin(), msg.end());
    if (!HISTORY.empty() && HISTORY.back() == ln)
        return;
    HISTORY.push_back(ln);
    if (HISTORY.size() > HISTORY_LENGTH)
        HISTORY.erase(HISTORY.begin());
}

void BrowseHistory(int dir)
{
    if (dir < 0 && HISTORY_INDEX > 0)
    {
        if (HISTORY_INDEX == HISTORY.size())
            DRAFT = PcTextEntryText();
        --HISTORY_INDEX;
        PcTextEntrySetText(HISTORY[HISTORY_INDEX]);
    }
    else if (dir > 0 && HISTORY_INDEX < HISTORY.size())
    {
        ++HISTORY_INDEX;
        PcTextEntrySetText(HISTORY_INDEX == HISTORY.size() ? DRAFT : HISTORY[HISTORY_INDEX]);
    }
}

void Send(Minecraft* mc, int pad)
{
    std::wstring msg = Trimmed(PcChatText());
    if (msg.empty() || mc->mLocalPlayers[pad] == 0)
        return;
    Remember(msg);
    if (PcScriptsClientCommand(msg))
        return;
    mc->mLocalPlayers[pad]->chat(msg);
}
}

void PcChatTick(Minecraft* mc, int pad, bool gameActive)
{
    char16_t key = PcInputConsumeChatKey();
    if (!CHAT_OPEN)
    {
        if (gameActive && key != 0)
        {
            const char16_t initial[2] = {key == u'/' ? u'/' : u'\0', u'\0'};
            PcTextEntryBegin(initial, CHAT_LENGTH);
            HISTORY_INDEX = HISTORY.size();
            DRAFT.clear();
            CHAT_OPEN = true;
            PcInputSetChatOpen(true);
        }
        return;
    }
    if (!gameActive)
    {
        Close();
        return;
    }
    BrowseHistory(PcTextEntryConsumeHistoryKey());
    switch (PcTextEntryCurrentResult())
    {
    case ePcTextEntry_Accepted:
        Send(mc, pad);
        Close();
        break;
    case ePcTextEntry_Cancelled:
        Close();
        break;
    default:
        break;
    }
}

bool PcChatActive()
{
    return CHAT_OPEN;
}

std::wstring PcChatText()
{
    std::wstring s;
    if (!CHAT_OPEN)
        return s;
    for (const char16_t* ch = PcTextEntryText(); *ch != 0; ++ch)
        s.push_back((wchar_t)(*ch));
    return s;
}

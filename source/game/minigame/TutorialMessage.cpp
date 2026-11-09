#include "game/minigame/TutorialMessage.hpp"
#include "game/core/L10N.hpp"

TutorialMessage::TutorialMessage(int messageId, bool repeatMessage, unsigned char maxDisplayCount)
    : m_messageId(messageId),
      m_maxDisplayCount(maxDisplayCount),
      m_displayCount(0),
      m_repeatMessage(repeatMessage)
{
}

unsigned char TutorialMessage::canDisplay()
{
    int canDisplay = 1;
    if (m_repeatMessage && m_displayCount >= m_maxDisplayCount)
        canDisplay = 0;
    return canDisplay;
}

const wchar_t* TutorialMessage::getMessageForDisplay()
{
    if (!canDisplay())
        return L"";
    if (m_repeatMessage)
        ++m_displayCount;
    return L10N::GetString(m_messageId);
}

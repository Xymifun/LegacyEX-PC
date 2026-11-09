#include "game/ui/FJ_LeaderboardEntry.hpp"

#include <cstdio>
#include <vector>

#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/core/StringConversion.hpp"

static std::string s_entryStates[2] = {"Entry_Unselected", "Entry_Selected"};

FJ_LeaderboardEntry::FJ_LeaderboardEntry(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode),
      m_rankTextWidth(0.0f),
      m_gamertagTextWidth(0.0f),
      m_columnTextWidth(0.0f)
{
    FJ_FuiNode* rankContainer = getChildByName("RankContainer")->getChildAt(0)->asMovieClip();
    FJ_FuiNode* gamertagContainer = getChildByName("GamertagContainer")->getChildAt(0)->asMovieClip();
    m_rankText = rankContainer->getRenderNodeChildAt(0)->asEditText();
    m_gamertagText = gamertagContainer->getRenderNodeChildAt(0)->asEditText();
    m_rankTextWidth = m_rankText->getWidth();
    m_gamertagTextWidth = m_gamertagText->getWidth();

    char name[64];
    for (int i = 0; i < 7; ++i)
    {
        std::sprintf(name, "FJ_Column_%d", i);
        FJ_FuiNode* column = getChildByName(name)->getChildAt(0)->asMovieClip();
        m_columnText[i] = column->getRenderNodeChildAt(0)->asEditText();
        addChild(m_columnText[i]);
        m_columnText[i]->setX(getChildByName(name)->getX());
        m_columnText[i]->setY(getChildByName(name)->getY());
        FJ_FuiNode* parent = column->getParent();
        parent->removeChild(column, true);
        m_columnTextWidth = m_columnText[i]->getWidth();
    }
    ChangeState(0);
    addEventListener(eFJEventType_FocusIn, this, _focusInHandler, false);
    addEventListener(eFJEventType_FocusOut, this, _focusOutHandler, false);
}

FJ_LeaderboardEntry* FJ_LeaderboardEntry::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_LeaderboardEntry(pRenderNode);
}

void FJ_LeaderboardEntry::Init(int count, int firstValue, int spacing, int offset)
{
    int i;
    for (i = 0; i < 7; ++i)
    {
        if (i < count)
        {
            m_columnPosition[i] = firstValue + spacing * i + offset;
            m_columnText[i]->setX((float)m_columnPosition[i]);
            m_columnText[i]->setVisibility(true);
        }
        else
            m_columnText[i]->setVisibility(false);
    }
    m_numColumns = count;
    m_firstValuePosition = (firstValue + offset + (firstValue + spacing * (count - 1) + offset)) / 2;
}

void FJ_LeaderboardEntry::SetData(FJ_LeaderboardList::CLeaderboardEntry* pEntry)
{
    int i;
    float scale = 1.0f;
    m_rankText->setText(pEntry != NULL ? _toString(pEntry->rank) : std::wstring(L""));
    m_rankText->setAutoSize(TextFieldAutoSizeLeft);
    m_rankText->setScaleX(scale / getScaleX());
    m_rankText->setScaleY(scale / getScaleY());
    m_rankText->setX(-m_rankText->getWidth() * 0.5f + m_rankTextWidth * 0.5f);

    m_gamertagText->setText(pEntry != NULL ? pEntry->gamertag : std::wstring(L""));
    m_gamertagText->setAutoSize(TextFieldAutoSizeLeft);
    m_gamertagText->setScaleX(scale / getScaleX());
    m_gamertagText->setScaleY(scale / getScaleY());
    m_gamertagText->setX(-m_gamertagText->getWidth() * 0.5f + m_gamertagTextWidth * 0.5f);

    if (pEntry != NULL && pEntry->singleValue)
    {
        for (i = 1; i < 7; ++i)
            m_columnText[i]->setVisibility(false);
        m_columnText[0]->setText(pEntry->values[0]);
        m_columnText[0]->setAutoSize(TextFieldAutoSizeLeft);
        m_columnText[0]->setScaleX(scale / getScaleX());
        m_columnText[0]->setScaleY(scale / getScaleY());
        m_columnText[0]->setX((float)m_firstValuePosition - m_columnText[0]->getWidth() * 0.5f);
        m_columnText[0]->setVisibility(true);
        return;
    }
    for (i = 0; i < 7; ++i)
    {
        if (pEntry != NULL && pEntry->values[i] != L"" && i < m_numColumns)
        {
            m_columnText[i]->setText(pEntry->values[i]);
            m_columnText[i]->setAutoSize(TextFieldAutoSizeLeft);
            m_columnText[i]->setScaleX(scale / getScaleX());
            m_columnText[i]->setScaleY(scale / getScaleY());
            m_columnText[i]->setX((float)m_columnPosition[i] - m_columnText[i]->getWidth() * 0.5f);
            m_columnText[i]->setVisibility(true);
        }
        else
            m_columnText[i]->setVisibility(false);
    }
}

void FJ_LeaderboardEntry::ChangeState(int state)
{
    gotoAndStop(s_entryStates[state]);
    m_state = state;
}

void FJ_LeaderboardEntry::focusInHandler(FJ_Event*)
{
    ChangeState(1);
}

void FJ_LeaderboardEntry::_focusInHandler(FJ_FuiNode* pNode, FJ_Event* pEvent)
{
    static_cast<FJ_LeaderboardEntry*>(pNode)->focusInHandler(pEvent);
}

void FJ_LeaderboardEntry::focusOutHandler(FJ_Event*)
{
    ChangeState(0);
}

void FJ_LeaderboardEntry::_focusOutHandler(FJ_FuiNode* pNode, FJ_Event* pEvent)
{
    static_cast<FJ_LeaderboardEntry*>(pNode)->focusOutHandler(pEvent);
}

unsigned int FJ_LeaderboardEntry::getType()
{
    return 108;
}

void FJ_LeaderboardEntry::UpdateHighlightColours()
{
    FJ_FuiNode* style = getChildByName("StyleContainer")->getChildAt(0)->asMovieClip();
    fuiRenderNodeEditText* text = style->getRenderNodeChildAt(0)->asEditText();
    m_rankText->m_colour = text->m_colour;
    m_gamertagText->m_colour = text->m_colour;
    for (int i = 0; i < 7; ++i)
        m_columnText[i]->m_colour = text->m_colour;
}

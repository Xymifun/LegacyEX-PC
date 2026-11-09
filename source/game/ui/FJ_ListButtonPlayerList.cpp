#include "game/ui/FJ_ListButtonPlayerList.hpp"
#include "game/ui/FJ_FuiNode.hpp"

FJ_ListButtonPlayerList::FJ_ListButtonPlayerList(fuiRenderNode* renderNode) : FJ_Button(renderNode)
{
    m_playerIcon = getChildByName(std::string("PlayerIcons"))->asMovieClip();
    m_voipIcon = getChildByName(std::string("VOIP"))->asMovieClip();
}

void FJ_ListButtonPlayerList::Init(const std::wstring& name, int id)
{
    SetLabelAlignment(4);
    m_textOnly = true;
    FJ_Button::Init(name, id);

    m_playerIcon->setScaleX(static_cast<float>(m_playerIcon->getScaleX() / getScaleX()));
    m_playerIcon->setScaleY(static_cast<float>(m_playerIcon->getScaleY() / getScaleY()));
    m_voipIcon->setScaleX(static_cast<float>(m_voipIcon->getScaleX() / getScaleX()));
    m_voipIcon->setScaleY(static_cast<float>(m_voipIcon->getScaleY() / getScaleY()));
}

void FJ_ListButtonPlayerList::SetPlayerIcon(int frame)
{
    m_playerIcon->gotoAndStop(frame);
}

void FJ_ListButtonPlayerList::SetVOIPIcon(int frame)
{
    m_voipIcon->gotoAndStop(frame);
}

#include "game/ui/FJ_ListButtonLobby.hpp"

#include "game/ui/FJ_FuiNode.hpp"

FJ_ListButtonLobby::FJ_ListButtonLobby(fuiRenderNode* renderNode) : FJ_Button(renderNode)
{
    m_playerIcons = getChildByName(std::string("PlayerIcons"))->asMovieClip();
    m_voipIcon = getChildByName(std::string("VOIP"))->asMovieClip();
    m_readyTick = getChildByName(std::string("ReadyTick"))->asMovieClip();
    m_readySlot = getChildByName(std::string("ReadySlot"))->asMovieClip();
}

void FJ_ListButtonLobby::Init(const std::wstring& name, int id)
{
    SetLabelAlignment(4);
    m_textOnly = true;
    m_ellipsisWidthOverride = static_cast<int>(m_readySlot->getX() * getScaleX());
    FJ_Button::Init(name, id);

    m_playerIcons->setScaleX(static_cast<float>(m_playerIcons->getScaleX() / getScaleX()));
    m_playerIcons->setScaleY(static_cast<float>(m_playerIcons->getScaleY() / getScaleY()));
    m_voipIcon->setScaleX(static_cast<float>(m_voipIcon->getScaleX() / getScaleX()));
    m_voipIcon->setScaleY(static_cast<float>(m_voipIcon->getScaleY() / getScaleY()));
    m_readyTick->setScaleX(static_cast<float>(m_readyTick->getScaleX() / getScaleX()));
    m_readyTick->setScaleY(static_cast<float>(m_readyTick->getScaleY() / getScaleY()));
    m_readySlot->setScaleX(static_cast<float>(m_readySlot->getScaleX() / getScaleX()));
    m_readySlot->setScaleY(static_cast<float>(m_readySlot->getScaleY() / getScaleY()));
}

void FJ_ListButtonLobby::SetPlayerIcon(int icon)
{
    m_playerIcons->gotoAndStop(icon);
}

void FJ_ListButtonLobby::SetVOIPIcon(int icon)
{
    m_voipIcon->gotoAndStop(icon);
}

void FJ_ListButtonLobby::ShowTick(bool show)
{
    m_readyTick->setVisible(show);
}

#include "game/ui/FJ_GameModeScoreboard.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_IconSlot.hpp"
#include "game/ui/FJ_List_ListButtonScoreBoard.hpp"
#include "game/ui/FJ_Button.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_GameModeScoreboard::FJ_GameModeScoreboard(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
    m_statIconHolders.push_back(getChildByName(std::string("PanelAward.Stat1Icon")));
    m_statIconHolders.push_back(getChildByName(std::string("PanelAward.Stat2Icon")));
    m_statIconHolders.push_back(getChildByName(std::string("PanelAward.Stat3Icon")));
    m_statIconHolders.push_back(getChildByName(std::string("PanelAward.Stat4Icon")));
    m_statIconHolders.push_back(getChildByName(std::string("PanelAward.Stat5Icon")));
    for (int i = 0; i < 5; ++i)
    {
        m_statIconHolders[i]->asFJ_IconSlot()->SetParams(0, false);
    }
    m_root = getChildByName(std::string("PanelScoreboard"));

    m_statIconCount = 0;

    m_killedByLabelY = static_cast<int>(getChildByName(std::string("PanelAward.KilledByLabel"))->getY());
    m_killedByLabelHeight = static_cast<int>(getChildByName(std::string("PanelAward.KilledByLabel"))->getHeight());
}

FJ_GameModeScoreboard* FJ_GameModeScoreboard::Create(fuiRenderNode* renderNode)
{
    return new FJ_GameModeScoreboard(renderNode);
}

unsigned int FJ_GameModeScoreboard::getType()
{
    return 79;
}

void FJ_GameModeScoreboard::SetupStatIconHolders()
{
    for (int i = 0; i < 5; ++i)
    {
        m_statIconHolders[i]->asFJ_IconSlot()->setWidth(m_statIconHolders[i]->getWidth());
        m_statIconHolders[i]->asFJ_IconSlot()->Init(std::wstring(L""));
        m_statIconHolders[i]->asFJ_IconSlot()->SetSlotID(std::string("stat"), i);
        m_statIconHolders[i]->asFJ_IconSlot()->ShowSlotHolder(false);
    }
}

void FJ_GameModeScoreboard::SetScoreboardType(int type)
{
    FJ_FuiNode* panel = getChildByName(std::string("PanelScoreboard"));
    m_scoreboardType = type;
    if (type == 0)
    {
        removeChild(getChildByName(std::string("PanelAward")), true);
        m_statIconHolders.clear();
        panel->setX(static_cast<float>(Math::round(getStage()->getStageWidth() * 0.5F - panel->getWidth() * 0.5F)));
        setX(0.0f);
    }
    else
        SetupStatIconHolders();
}

void FJ_GameModeScoreboard::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    if (getStage()->getStageHeight() != 360.0F && getStage()->getStageHeight() != 540.0F)
        return;
    setX(static_cast<float>(left));
    setY(static_cast<float>(top));
}

void FJ_GameModeScoreboard::handlePress(FJ_Event* event)
{
    FJ_Button* button = event->m_pTarget->asFJ_Button();
    if (button != 0)
    {
        m_pCallbackScene->handlePress(static_cast<FJ_Base*>(button->getObjListParent())->getId(), button->getId());
        button->setButtonPressed(false);
    }
}

void FJ_GameModeScoreboard::InitNumColumns(int numColumns)
{
    FJ_FuiNode* panel = getChildByName(std::string("PanelScoreboard"));
    FJ_List_ListButtonScoreBoard* list = static_cast<FJ_List_ListButtonScoreBoard*>(
        getChildByName(std::string("PanelScoreboard.Scoreboard")));
    for (int i = 0; i < numColumns; ++i)
    {
        FJ_IconSlot* icon = static_cast<FJ_IconSlot*>(
            FJ_Factory::createObject("fourj.listClasses.FJ_IconSlot", m_pRenderNode, true, 0));
        icon->SetParams(list->GetButtonHeight(), list->m_checkboxTick);
        icon->ShowSlotHolder(false);
        icon->Init(std::wstring(L""));
        icon->setX(list->getX() + list->getWidth() - icon->getWidth() * static_cast<float>(numColumns - i)
                   - static_cast<float>((numColumns - i - 1) * 2));
        icon->setY(list->getY() - static_cast<float>(static_cast<int>(icon->getHeight() * 1.1f)));
        m_titleSlots.push_back(icon);
        panel->addChild(icon);
        icon->SetSlotID(std::string("header"), i);
    }
    list->SetNumColumns(numColumns);
}

void FJ_GameModeScoreboard::SetTitleSlotText(int slot, const std::wstring& text)
{
    m_titleSlots[slot]->asFJ_IconSlot()->SetLabel(text);
}

void FJ_GameModeScoreboard::ShowTitleSlot(int slot, bool show)
{
    m_titleSlots[slot]->setVisible(false);
}

void FJ_GameModeScoreboard::SetTitleIconTextureName(int slot, const std::wstring& textureName)
{
    m_titleSlots[slot]->asFJ_IconSlot()->SetTextureName(textureName);
}

void FJ_GameModeScoreboard::SetStatIconTextureName(int slot, const std::wstring& textureName)
{
    if (static_cast<unsigned int>(slot) < m_statIconHolders.size())
        m_statIconHolders[slot]->asFJ_IconSlot()->SetTextureName(textureName);
}

void FJ_GameModeScoreboard::SetHasPlayerBeenKilled(bool killed)
{
    FJ_FuiNode* award = getChildByName(std::string("PanelAward"));
    if (award != 0)
    {
        FJ_FuiNode* killedBy = award->getChildByName(std::string("KilledByLabel"));
        FJ_FuiNode* block = award->getChildByName(std::string("KilledByBlock"));
        FJ_FuiNode* gamertag = award->getChildByName(std::string("GamertagLabel"));
        FJ_FuiNode* icon = award->getChildByName(std::string("Stat5Icon"));
        FJ_FuiNode* label = award->getChildByName(std::string("Stat5Label"));
        if (killed)
        {
            killedBy->setY(static_cast<float>(m_killedByLabelY));
        }
        else
        {
            killedBy->setY(block->getY() + block->getHeight() * 0.5f - static_cast<float>(m_killedByLabelHeight / 2));
        }
        gamertag->setVisible(killed);
        icon->setVisible(killed);
        label->setVisible(killed);
    }
}

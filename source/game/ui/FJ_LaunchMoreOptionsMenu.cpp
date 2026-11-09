#include "game/ui/FJ_LaunchMoreOptionsMenu.hpp"

#include <string>

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_HtmlText.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_LaunchMoreOptionsMenu::FJ_LaunchMoreOptionsMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
    addEventListener(eFJEventType_CheckboxToggled, this, _handleCheckboxToggled, false);
    m_worldTabHidden = false;
    m_previousTab = -1;
    m_activeTab = 0;
    m_gameOptionsFocus = -1;
    m_worldOptionsFocus = -1;
    m_descriptionUpdated = false;
    m_menuType = 0;
}

void FJ_LaunchMoreOptionsMenu::handleCheckboxToggled(FJ_Event* event)
{
    FJ_CheckBox* checkBox = event->m_pTarget->asFJ_CheckBox();
    if (checkBox != NULL)
        m_pCallbackScene->handlePress(static_cast<FJ_Base*>(checkBox->getObjListParent())->getId(), checkBox->getId());
}

void FJ_LaunchMoreOptionsMenu::_handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_LaunchMoreOptionsMenu*>(node)->handleCheckboxToggled(event);
}

FJ_Document* FJ_LaunchMoreOptionsMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_LaunchMoreOptionsMenu(renderNode);
}

unsigned int FJ_LaunchMoreOptionsMenu::getType()
{
    return 23;
}

void FJ_LaunchMoreOptionsMenu::UpdateDescription()
{
    FJ_FuiNode* gameOptions = getChildByName("GameOptions");
    FJ_FuiNode* worldOptions = getChildByName("WorldOptions");
    FJ_HtmlText* gameDescription = static_cast<FJ_HtmlText*>(gameOptions->getChildByName("Description_GameOptions"));
    FJ_HtmlText* worldDescription = static_cast<FJ_HtmlText*>(worldOptions->getChildByName("Description_WorldOptions"));

    if (!m_descriptionUpdated)
    {
        worldDescription->Init(L"");
        gameDescription->Init(L"");
        worldDescription->setVisible(false);
        gameDescription->setVisible(false);
    }

    if (m_activeTab == 0)
    {
        worldDescription->setVisible(false);
        gameDescription->setVisible(true);
    }
    else
    {
        worldDescription->setVisible(true);
        gameDescription->setVisible(false);
    }
}

void FJ_LaunchMoreOptionsMenu::SetActiveTab(int tab, bool resize)
{
    FJ_Base* focus = getStage()->getFocus()->asFJ_Base();
    if (focus != NULL)
    {
        if (m_previousTab == 0)
            m_worldOptionsFocus = focus->getId();
        else
            m_gameOptionsFocus = focus->getId();
    }

    FJ_FuiNode* gameOptions = getChildByName("GameOptions");
    FJ_FuiNode* worldOptions = getChildByName("WorldOptions");
    FJ_List* gameList = static_cast<FJ_List*>(gameOptions->getChildByName("GameOptionsList"));
    FJ_List* worldList = static_cast<FJ_List*>(worldOptions->getChildByName("WorldOptionsList"));
    FJ_FuiNode* gameBackground = gameOptions->getChildByName("BackgroundPanel");
    FJ_FuiNode* worldBackground = worldOptions->getChildByName("BackgroundPanel");
    FJ_FuiNode* gameDescription = gameOptions->getChildByName("Description_GameOptions");
    FJ_FuiNode* worldDescription = worldOptions->getChildByName("Description_WorldOptions");
    FJ_FuiNode* textPanel = getChildByName("TextPanelBackground");

    gameOptions->setVisible(tab == 0);
    gameList->setVisible(gameOptions->isVisible());
    worldOptions->setVisible(tab != 0);
    worldList->setVisible(worldOptions->isVisible());
    gotoAndStop(tab == 0 ? "GameOptions" : "WorldOptions");

    if (resize)
    {
        m_root = worldOptions;
        worldBackground->getChildAt(0)->setHeight(worldList->GetListHeight()
                                                  + (worldList->getY() - worldBackground->getY()) * 2);
        worldDescription->setHeight(worldBackground->getChildAt(0)->getHeight()
                                    + (worldOptions->getY() + worldBackground->getY() - textPanel->getY()) * 2
                                    - (worldDescription->getY() + worldOptions->getY() - textPanel->getY()) * 2);
        gameBackground->getChildAt(0)->setHeight(gameList->GetListHeight()
                                                 + (gameList->getY() - gameBackground->getY()) * 2);
        gameDescription->setHeight(gameBackground->getChildAt(0)->getHeight()
                                   + (gameOptions->getY() + gameBackground->getY() - textPanel->getY()) * 2
                                   - (gameDescription->getY() + gameOptions->getY() - textPanel->getY()) * 2);
        UpdateDescription();
        m_descriptionUpdated = true;
    }

    m_root = tab == 0 ? gameOptions : worldOptions;

    if (tab == 1)
    {
        SetFocus(m_worldOptionsFocus);
        if (m_worldOptionsFocus >= 0)
            OnFocusChange(true);
        textPanel->getChildAt(0)->setHeight(worldBackground->getChildAt(0)->getHeight()
                                            + (worldOptions->getY() + worldBackground->getY() - textPanel->getY()) * 2);
        m_activeTab = tab;
        m_previousTab = tab;
        return;
    }

    SetFocus(m_gameOptionsFocus);
    if (m_gameOptionsFocus >= 0)
        OnFocusChange(true);
    textPanel->getChildAt(0)->setHeight(gameBackground->getChildAt(0)->getHeight()
                                        + (gameOptions->getY() + gameBackground->getY() - textPanel->getY()) * 2);
    m_activeTab = tab;
    m_previousTab = tab;
}

void FJ_LaunchMoreOptionsMenu::ChangeTab()
{
    if (!m_worldTabHidden)
    {
        SetActiveTab(m_activeTab == 0, false);
        UpdateDescription();
    }
}

void FJ_LaunchMoreOptionsMenu::SetMenuType(int type)
{
    m_menuType = type;
    m_activeTab = type == 0;
    SetActiveTab(m_activeTab, true);
}

void FJ_LaunchMoreOptionsMenu::HideWorldTab()
{
    FJ_FuiNode* gameOptions = getChildByName("GameOptions");
    FJ_FuiNode* worldOptions = getChildByName("WorldOptions");
    FJ_List* worldList = static_cast<FJ_List*>(worldOptions->getChildByName("WorldOptionsList"));
    FJ_FuiNode* worldBackground = worldOptions->getChildByName("BackgroundPanel");
    FJ_Label* gameLabel = static_cast<FJ_Label*>(getChildByName("LabelGame"));
    FJ_Label* worldLabel = static_cast<FJ_Label*>(getChildByName("LabelWorld"));
    FJ_FuiNode* backTabs = getChildByName("BackTabs");
    FJ_FuiNode* gameBackTab = backTabs->getChildByName("BackTabOffGameOptions");
    FJ_FuiNode* worldBackTab = backTabs->getChildByName("BackTabOffWorldOptions");
    FJ_FuiNode* worldTab = worldOptions->getChildByName("Tab");
    FJ_FuiNode* gameTab = gameOptions->getChildByName("Tab");

    worldList->removeAllItems();
    SetActiveTab(0, false);
    gotoAndStop("WorldOptions");
    worldLabel->SetLabel(L"");
    gameLabel->SetLabel(L"");
    m_worldTabHidden = true;
    gameLabel->setVisible(false);
    gameBackTab->setVisible(false);
    worldBackTab->setVisible(false);
    worldOptions->setVisible(true);
    worldBackground->setVisible(false);
    worldList->setVisible(false);
    worldTab->setVisible(false);
    gameTab->setVisible(false);
}

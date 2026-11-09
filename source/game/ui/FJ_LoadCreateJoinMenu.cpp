#include "game/ui/FJ_LoadCreateJoinMenu.hpp"

#include <string>

#include "game/ui/FJ_Button.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Label_Blink.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/UIScene_LoadCreateJoinMenu.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_LoadCreateJoinMenu::FJ_LoadCreateJoinMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode),
      m_activeTab(0),
      m_currentTab(-1),
      m_loadTabFocus(-1),
      m_newTabFocus(-1),
      m_joinTabFocus(-1),
      m_loadListHeight(0),
      m_loadListRecessHeight(0),
      m_loadTimerY(0),
      m_touchEnabled(false),
      m_loadTabTitleY(0),
      m_joinTabTitleY(0),
      m_newTabX(0),
      m_joinTabX(0)
{
    getChildByName("LoadGame.TabLeft")->getChildAt(0)->gotoAndStop(1);
    getChildByName("NewGame.TabMiddle")->getChildAt(0)->gotoAndStop(2);
    getChildByName("JoinGame.TabRight")->getChildAt(0)->gotoAndStop(3);

    if (getChildByName("LoadGame")->getChildByName("LoadGameList") != NULL)
        m_loadListHeight = static_cast<int>(getChildByName("LoadGame")->getChildByName("LoadGameList")->getHeight());

    if (getChildByName("LoadGame")->getChildByName("LoadGameListRecessPanel") != NULL)
        m_loadListRecessHeight = static_cast<int>(
            getChildByName("LoadGame")->getChildByName("LoadGameListRecessPanel")->getChildAt(0)->getHeight());

    if (getChildByName("LoadGame")->getChildByName("LoadGameList") != NULL)
        m_loadTimerY = static_cast<int>(getChildByName("LoadGame.LoadGameTimer")->getY());

    m_loadTabTitleY = static_cast<int>(getChildByName("LoadGameTabTitle")->getY());
    m_joinTabTitleY = static_cast<int>(getChildByName("JoinGameTabTitle")->getY());

    m_newTabX = static_cast<int>(getChildByName("LoadGame")->getX()
                                 + getChildByName("LoadGame")->getChildAt(0)->getWidth() * 0.25f
                                 - getChildByName("NewGameTabTitle")->getWidth() * 0.5f);
    m_joinTabX = static_cast<int>(getChildByName("LoadGame")->getX()
                                  + getChildByName("LoadGame")->getChildAt(0)->getWidth() * 0.25f * 3.0f
                                  - getChildByName("JoinGameTabTitle")->getWidth() * 0.5f);
    SetActiveTab(m_activeTab);
}

FJ_LoadCreateJoinMenu* FJ_LoadCreateJoinMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_LoadCreateJoinMenu(renderNode);
}

unsigned int FJ_LoadCreateJoinMenu::getType()
{
    return 8;
}

void FJ_LoadCreateJoinMenu::SetActiveTab(int tab)
{
    if (tab == 0)
    {
        getChildByName("LoadGame")->setVisible(true);
        getChildByName("NewGame")->setVisible(false);
        getChildByName("JoinGame")->setVisible(false);
        getChildByName("LoadGame.LoadGameList")->setVisible(getChildByName("LoadGame")->isVisible());
        getChildByName("NewGame.NewGameList")->setVisible(getChildByName("NewGame")->isVisible());
        getChildByName("JoinGame.JoinGameGamesList")->setVisible(getChildByName("JoinGame")->isVisible());
        gotoAndStop("LoadGame");

        if (getStage()->getFocus()->isFJ_Base())
        {
            if (m_currentTab == 1)
                m_newTabFocus = getStage()->getFocus()->asFJ_Base()->getId();
            else if (m_currentTab == 2)
                m_joinTabFocus = getStage()->getFocus()->asFJ_Base()->getId();
        }

        m_root->removeEventListener(eFJEventType_Press, this, FJ_Document::_handlePress);
        m_root = getChildByName("LoadGame");
        m_root->addEventListener(eFJEventType_Press, this, FJ_Document::_handlePress, false);
        SetFocus(m_loadTabFocus);
        if (m_loadTabFocus >= 0)
            OnFocusChange(true);
    }
    else if (tab == 1)
    {
        getChildByName("LoadGame")->setVisible(false);
        getChildByName("NewGame")->setVisible(true);
        getChildByName("JoinGame")->setVisible(false);
        getChildByName("LoadGame.LoadGameList")->setVisible(getChildByName("LoadGame")->isVisible());
        getChildByName("NewGame.NewGameList")->setVisible(getChildByName("NewGame")->isVisible());
        getChildByName("JoinGame.JoinGameGamesList")->setVisible(getChildByName("JoinGame")->isVisible());
        gotoAndStop("NewGame");

        if (getStage()->getFocus()->isFJ_Base())
        {
            if (m_currentTab == 0)
                m_loadTabFocus = getStage()->getFocus()->asFJ_Base()->getId();
            else if (m_currentTab == 2)
                m_joinTabFocus = getStage()->getFocus()->asFJ_Base()->getId();
        }

        m_root->removeEventListener(eFJEventType_Press, this, FJ_Document::_handlePress);
        m_root = getChildByName("NewGame");
        m_root->addEventListener(eFJEventType_Press, this, FJ_Document::_handlePress, false);
        SetFocus(m_newTabFocus);
        if (m_newTabFocus >= 0)
            OnFocusChange(true);

        if (m_touchEnabled)
        {
            getChildByName("NewGameTabTitle")->setY(static_cast<float>(m_loadTabTitleY));
            getChildByName("JoinGameTabTitle")->setY(static_cast<float>(m_joinTabTitleY));
        }
    }
    else if (tab == 2)
    {
        getChildByName("LoadGame")->setVisible(false);
        getChildByName("NewGame")->setVisible(false);
        getChildByName("JoinGame")->setVisible(true);
        getChildByName("LoadGame.LoadGameList")->setVisible(getChildByName("LoadGame")->isVisible());
        getChildByName("NewGame.NewGameList")->setVisible(getChildByName("NewGame")->isVisible());
        getChildByName("JoinGame.JoinGameGamesList")->setVisible(getChildByName("JoinGame")->isVisible());
        gotoAndStop("JoinGame");

        if (getStage()->getFocus()->isFJ_Base())
        {
            if (m_currentTab == 0)
                m_loadTabFocus = getStage()->getFocus()->asFJ_Base()->getId();
            else if (m_currentTab == 1)
                m_newTabFocus = getStage()->getFocus()->asFJ_Base()->getId();
        }

        m_root->removeEventListener(eFJEventType_Press, this, FJ_Document::_handlePress);
        m_root = getChildByName("JoinGame");
        m_root->addEventListener(eFJEventType_Press, this, FJ_Document::_handlePress, false);
        SetFocus(m_joinTabFocus);
#if defined(RECON_NATIVE)
        if (m_joinTabFocus < 0 && getChildByName("JoinGame.JoinGameGamesList")->asFJ_List() != NULL)
            getStage()->setFocus(getChildByName("JoinGame.JoinGameGamesList"));
#endif
        if (m_joinTabFocus >= 0)
            OnFocusChange(true);

        getChildByName("JoinGameTabTitle")->asFJ_Label_Blink()->EnableBlink(false);

        if (m_touchEnabled)
        {
            getChildByName("NewGameTabTitle")->setY(static_cast<float>(m_joinTabTitleY));
            getChildByName("JoinGameTabTitle")->setY(static_cast<float>(m_loadTabTitleY));
        }
    }

    m_activeTab = tab;
    m_currentTab = tab;
}

void FJ_LoadCreateJoinMenu::handlePress(FJ_Event* event)
{
    FJ_Button* button = event->m_pTarget->asFJ_Button();
    if (button != NULL)
    {
        static_cast<UIScene_LoadCreateJoinMenu*>(m_pCallbackScene)
            ->handlePress(button->FJ_Base::getObjListParent()->asFJ_List()->getId(), button->getId());
        button->setButtonPressed(false);
    }
}

void FJ_LoadCreateJoinMenu::SetMatchesAvailable(bool available)
{
    if (m_activeTab != 2)
    {
        getChildByName(std::string("JoinGameTabTitle"))->asFJ_Label_Blink()->EnableBlink(available);
    }
    else
    {
        getChildByName(std::string("JoinGameTabTitle"))->asFJ_Label_Blink()->EnableBlink(false);
    }
}

void FJ_LoadCreateJoinMenu::ShowSaveSizeBar(bool show)
{
    if (getChildByName("LoadGame")->getChildByName("SaveSizeBar") != NULL)
    {
        getChildByName("LoadGame")->getChildByName("SaveSizeBar")->setVisible(show);
        if (show)
        {
            if (getChildByName("LoadGame.LoadGameList") != NULL)
                getChildByName("LoadGame.LoadGameList")->setHeight(static_cast<float>(m_loadListHeight));
            if (getChildByName("LoadGame.LoadGameListRecessPanel") != NULL)
                getChildByName("LoadGame.LoadGameListRecessPanel")
                    ->getChildAt(0)
                    ->setHeight(static_cast<float>(m_loadListRecessHeight));
            if (getChildByName("LoadGame.LoadGameList") != NULL)
                getChildByName("LoadGame.LoadGameTimer")->setY(static_cast<float>(m_loadTimerY));
        }
        else
        {
            if (getChildByName("LoadGame.LoadGameList") != NULL)
                getChildByName("LoadGame.LoadGameList")->setHeight(getChildByName("NewGame.NewGameList")->getHeight());
            if (getChildByName("LoadGame.LoadGameListRecessPanel") != NULL)
                getChildByName("LoadGame.LoadGameListRecessPanel")
                    ->getChildAt(0)
                    ->setHeight(getChildByName("NewGame.NewGameListRecessPanel")->getChildAt(0)->getHeight());
            if (getChildByName("LoadGame.LoadGameList") != NULL)
                getChildByName("LoadGame.LoadGameTimer")->setY(getChildByName("NewGame.NewGameTimer")->getY());
        }
    }
}

void FJ_LoadCreateJoinMenu::HideLoadTab()
{
    if (!m_touchEnabled)
    {
        getChildByName("LoadGame")->setVisible(false);
        getChildByName("LoadGameTabTitle")->setVisible(false);
        getChildByName("TabOff_Load")->setVisible(false);
        getChildByName("NewGame.TabMiddle")->getChildAt(0)->gotoAndStop(1);
        getChildByName("JoinGame.TabRight")->getChildAt(0)->gotoAndStop(3);

        getChildByName("TabOff_Create")->setX(getChildByName("LoadGame")->getX());
        getChildByName("TabOff_Create")
            ->getChildAt(0)
            ->setWidth(getChildByName("LoadGame")->getChildAt(0)->getWidth() * 0.5f);

        getChildByName("NewGame.TabMiddle")->setX(0.0f);
        getChildByName("NewGame.TabMiddle")
            ->getChildAt(0)
            ->setWidth(getChildByName("LoadGame")->getChildAt(0)->getWidth() * 0.5f);

        if (getChildByName("LoadGame.TouchTabLoad") != NULL)
        {
            getChildByName("LoadGame.TouchTabLoad")->setVisible(false);
            getChildByName("NewGame.TouchTabCreate")->setX(0.0f);
            getChildByName("NewGame.TouchTabCreate")
                ->setWidth(getChildByName("NewGame.TabMiddle")->getChildAt(0)->getWidth());
        }

        getChildByName("TabOff_Join")
            ->setX(getChildByName("LoadGame")->getX() + getChildByName("LoadGame")->getChildAt(0)->getWidth() * 0.5f);
        getChildByName("TabOff_Join")
            ->getChildAt(0)
            ->setWidth(getChildByName("LoadGame")->getChildAt(0)->getWidth() * 0.5f);

        getChildByName("JoinGame.TabRight")->setX(getChildByName("LoadGame")->getChildAt(0)->getWidth() * 0.5f);
        getChildByName("JoinGame.TabRight")
            ->getChildAt(0)
            ->setWidth(getChildByName("LoadGame")->getChildAt(0)->getWidth() * 0.5f);

        bool hasTouchLoadTab = getChildByName("LoadGame.TouchTabLoad") != NULL;
        if (hasTouchLoadTab)
        {
            getChildByName("JoinGame.TouchTabJoin")->setX(getChildByName("LoadGame")->getChildAt(0)->getWidth() * 0.5f);
            getChildByName("JoinGame.TouchTabJoin")
                ->setWidth(getChildByName("LoadGame")->getChildAt(0)->getWidth() * 0.5f);
        }
    }
}

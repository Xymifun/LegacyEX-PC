#include "game/ui/FJ_AvatarSelectMenu.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Button.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_AvatarSelectMenu::FJ_AvatarSelectMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode),
      m_activeTab(0),
      m_currentTab(-1),
      m_userTabFocus(-1),
      m_favouriteTabFocus(-1),
      m_guestTabFocus(-1)
{
    getChildByName("UserTab.TabLeft")->getChildAt(0)->gotoAndStop(1);
    getChildByName("FavouriteTab.TabMiddle")->getChildAt(0)->gotoAndStop(2);
    getChildByName("GuestTab.TabRight")->getChildAt(0)->gotoAndStop(3);
    getChildByName("TabLeftOff")->getChildAt(0)->gotoAndStop(1);
    getChildByName("TabMiddleOff")->getChildAt(0)->gotoAndStop(2);
    getChildByName("TabRightOff")->getChildAt(0)->gotoAndStop(3);
    SetActiveTab(m_activeTab);
}

FJ_AvatarSelectMenu* FJ_AvatarSelectMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_AvatarSelectMenu(renderNode);
}

unsigned int FJ_AvatarSelectMenu::getType()
{
    return 0x50;
}

void FJ_AvatarSelectMenu::SetActiveTab(int tab)
{
    FJ_FuiNode* userTab = getChildByName("UserTab");
    FJ_FuiNode* userList = userTab->getChildByName("UserList");
    FJ_FuiNode* favouriteTab = getChildByName("FavouriteTab");
    FJ_FuiNode* favouriteList = favouriteTab->getChildByName("FavouriteList");
    FJ_FuiNode* guestTab = getChildByName("GuestTab");
    FJ_FuiNode* guestList = guestTab->getChildByName("GuestTabList");

    if (tab == 0)
    {
        userTab->setVisible(true);
        favouriteTab->setVisible(false);
        guestTab->setVisible(false);
        userList->setVisible(userTab->isVisible());
        favouriteList->setVisible(favouriteTab->isVisible());
        guestList->setVisible(guestTab->isVisible());
        gotoAndStop("UserTab");

        if (getStage()->getFocus()->isFJ_Base())
        {
            if (m_currentTab == 1)
                m_favouriteTabFocus = getStage()->getFocus()->asFJ_Base()->getId();
            else if (m_currentTab == 2)
                m_guestTabFocus = getStage()->getFocus()->asFJ_Base()->getId();
        }

        m_root->removeEventListener(eFJEventType_Press, this, FJ_Document::_handlePress);
        m_root = userTab;
        m_root->addEventListener(eFJEventType_Press, this, FJ_Document::_handlePress, false);
        SetFocus(m_userTabFocus);
        if (m_userTabFocus >= 0)
            OnFocusChange(true);
    }
    else if (tab == 1)
    {
        userTab->setVisible(false);
        favouriteTab->setVisible(true);
        guestTab->setVisible(false);
        userList->setVisible(userTab->isVisible());
        favouriteList->setVisible(favouriteTab->isVisible());
        guestList->setVisible(guestTab->isVisible());
        gotoAndStop("FavouriteTab");

        if (getStage()->getFocus()->isFJ_Base())
        {
            if (m_currentTab == 0)
                m_userTabFocus = getStage()->getFocus()->asFJ_Base()->getId();
            else if (m_currentTab == 2)
                m_guestTabFocus = getStage()->getFocus()->asFJ_Base()->getId();
        }

        m_root->removeEventListener(eFJEventType_Press, this, FJ_Document::_handlePress);
        m_root = favouriteTab;
        m_root->addEventListener(eFJEventType_Press, this, FJ_Document::_handlePress, false);
        SetFocus(m_favouriteTabFocus);
        if (m_favouriteTabFocus >= 0)
            OnFocusChange(true);
    }
    else if (tab == 2)
    {
        userTab->setVisible(false);
        favouriteTab->setVisible(false);
        guestTab->setVisible(true);
        userList->setVisible(userTab->isVisible());
        favouriteList->setVisible(favouriteTab->isVisible());
        guestList->setVisible(guestTab->isVisible());
        gotoAndStop("GuestTab");

        if (getStage()->getFocus()->isFJ_Base())
        {
            if (m_currentTab == 0)
                m_userTabFocus = getStage()->getFocus()->asFJ_Base()->getId();
            else if (m_currentTab == 1)
                m_favouriteTabFocus = getStage()->getFocus()->asFJ_Base()->getId();
        }

        m_root->removeEventListener(eFJEventType_Press, this, FJ_Document::_handlePress);
        m_root = guestTab;
        m_root->addEventListener(eFJEventType_Press, this, FJ_Document::_handlePress, false);
        SetFocus(m_guestTabFocus);
        if (m_guestTabFocus >= 0)
            OnFocusChange(true);
    }

    m_activeTab = tab;
    m_currentTab = tab;
}

void FJ_AvatarSelectMenu::handlePress(FJ_Event* event)
{
    FJ_Button* button = event->m_pTarget->asFJ_Button();
    if (button != NULL)
    {
        m_pCallbackScene->handlePress(static_cast<FJ_Base*>(button->getObjListParent())->getId(), button->getId());
        button->setButtonPressed(false);
    }
}

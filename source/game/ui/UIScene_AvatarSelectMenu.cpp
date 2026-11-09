#include "game/ui/UIScene_AvatarSelectMenu.hpp"
#include "game/core/StringIds.hpp"
#include <cwchar>
#include <cstring>
#include <string>
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/FJ_AvatarSelectMenu.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/MiiData.hpp"
#include "game/core/GameSettings.hpp"

extern CInput* g_pInput;

extern ConsoleUIController gConsoleUIController;
extern C_4JProfile* gProfile;

UIScene_AvatarSelectMenu::TabControls::TabControls() : m_tab(), m_title(), m_timer(), m_miis()
{
}

void UIScene_AvatarSelectMenu::TabControls::ShowTimer(bool show)
{
    m_timer.setVisible(show);
}

UIScene_AvatarSelectMenu::UIScene_AvatarSelectMenu(int player, void* data, UILayer* layer)
    : UIScene(player, layer),
      m_userTabTouch(),
      m_favouriteTabTouch(),
      m_guestTabTouch(),
      m_userControls(),
      m_favouriteControls(),
      m_guestControls()
{
    initialiseMovie();
    gApp.SetLiveLinkRequired(true);
    m_pad = player;
    m_userControls.m_owner = this;
    m_favouriteControls.m_owner = this;
    m_guestControls.m_owner = this;
    m_userControls.m_list.init(0);
    m_favouriteControls.m_list.init(1);
    m_guestControls.m_list.init(2);
    m_userControls.m_title.init(UIString(0xc835d6e5));
    m_favouriteControls.m_title.init(UIString(IDS_FAVOURITE_MIIS));
    m_guestControls.m_title.init(UIString(IDS_GUEST_MIIS));
    m_userControls.m_timer.setVisible(false);
    m_favouriteControls.m_timer.setVisible(false);
    m_guestControls.m_timer.setVisible(false);
    m_initialised = false;
    m_unknown1 = false;
    m_navigateBack = false;
    m_currentTab = 0;
    m_miisAdded = false;

    int choice = 0;
    for (int pad = 0; pad < 4; ++pad)
    {
        if (!gProfile->IsSignedIn(pad))
        {
            choice = pad;
            break;
        }
    }
    m_choice = choice;
    gConsoleUIController.m_pressStartBlocked |= 1 << m_choice;
    gConsoleUIController.HidePressStart();
    m_userTabTouch.init(3);
    m_favouriteTabTouch.init(4);
    m_guestTabTouch.init(5);
    gConsoleUIController.TouchBoxRebuild(this);
    m_activeTab = 0;
}

UIScene_AvatarSelectMenu::~UIScene_AvatarSelectMenu()
{
    gConsoleUIController.m_pressStartBlocked &= ~(1 << m_choice);
}

unsigned int UIScene_AvatarSelectMenu::mapElementsAndNames()
{
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_userControls.m_tab.setupControl(this, currentNode, std::string("UserTab"));
        m_controls.push_back(&m_userControls.m_tab);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_userControls.m_tab.getFuiRenderNode();
        {
            m_userControls.m_list.setupControl(this, currentNode, std::string("UserList"));
            m_controls.push_back(&m_userControls.m_list);
        }
        {
            m_userControls.m_timer.setupControl(this, currentNode, std::string("UserTimer"));
            m_controls.push_back(&m_userControls.m_timer);
        }
        {
            m_userTabTouch.setupControl(this, currentNode, std::string("TouchTabUser"));
            m_controls.push_back(&m_userTabTouch);
        }
        currentNode = parentNode;
    }
    {
        m_favouriteControls.m_tab.setupControl(this, currentNode, std::string("FavouriteTab"));
        m_controls.push_back(&m_favouriteControls.m_tab);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_favouriteControls.m_tab.getFuiRenderNode();
        {
            m_favouriteControls.m_list.setupControl(this, currentNode, std::string("FavouriteList"));
            m_controls.push_back(&m_favouriteControls.m_list);
        }
        {
            m_favouriteControls.m_timer.setupControl(this, currentNode, std::string("FavouriteTimer"));
            m_controls.push_back(&m_favouriteControls.m_timer);
        }
        {
            m_favouriteTabTouch.setupControl(this, currentNode, std::string("TouchTabFavourite"));
            m_controls.push_back(&m_favouriteTabTouch);
        }
        currentNode = parentNode;
    }
    {
        m_guestControls.m_tab.setupControl(this, currentNode, std::string("GuestTab"));
        m_controls.push_back(&m_guestControls.m_tab);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_guestControls.m_tab.getFuiRenderNode();
        {
            m_guestControls.m_list.setupControl(this, currentNode, std::string("GuestTabList"));
            m_controls.push_back(&m_guestControls.m_list);
        }
        {
            m_guestControls.m_timer.setupControl(this, currentNode, std::string("GuestTimer"));
            m_controls.push_back(&m_guestControls.m_timer);
        }
        {
            m_guestTabTouch.setupControl(this, currentNode, std::string("TouchTabGuest"));
            m_controls.push_back(&m_guestTabTouch);
        }
        currentNode = parentNode;
    }
    {
        m_userControls.m_title.setupControl(this, currentNode, std::string("UserTabTitle"));
        m_controls.push_back(&m_userControls.m_title);
    }
    {
        m_favouriteControls.m_title.setupControl(this, currentNode, std::string("FavouriteTabTitle"));
        m_controls.push_back(&m_favouriteControls.m_title);
    }
    {
        m_guestControls.m_title.setupControl(this, currentNode, std::string("GuestTabTitle"));
        m_controls.push_back(&m_guestControls.m_title);
    }
    return 1;
}

void UIScene_AvatarSelectMenu::AddMiiItem(TabControls& controls, MiiDataEntry* mii)
{
    for (int i = 0; i < 4; ++i)
    {
        if (gProfile->IsSignedIn(i))
        {
            PlayerUID uid(0);
            gProfile->GetXUID(i, &uid, false);
            if (mii->mIsAccountMii)
            {
                if (uid.uuid == mii->mUuid)
                    return;
            }
            else if (std::memcmp(uid.mii.createId, mii->mCreatorId, 10) == 0)
            {
                return;
            }
        }
    }
    controls.m_list.addItem(reinterpret_cast<const wchar_t*>(mii->mName));
    wchar_t texture[32];
    std::swprintf(texture, 32, L"Mii_num_%d", mii->mIconTexture);
    registerSubstitutionTexture(texture, NULL, mii->mIconTexture, false);
    int index = controls.m_list.getItemCount() - 1;
    controls.m_list.setTextureName(index, texture);
    controls.m_miis.push_back(mii);
}

void UIScene_AvatarSelectMenu::AddMiiItems()
{
    int userCount = gProfile->GetNumSysUserMiis();
    for (int i = 0; i < userCount; ++i)
    {
        MiiDataEntry* mii = gProfile->GetSysUserMiiData(i);
        AddMiiItem(m_userControls, mii);
    }
    int favouriteCount = gProfile->GetNumFavouriteMiis();
    for (int i = 0; i < favouriteCount; ++i)
    {
        MiiDataEntry* mii = gProfile->GetFavouriteMiiData(i);
        AddMiiItem(m_favouriteControls, mii);
    }
    int guestCount = gProfile->GetNumGuestMiis();
    for (int i = 0; i < guestCount; ++i)
    {
        MiiDataEntry* mii = gProfile->GetGuestMiiData(i);
        AddMiiItem(m_guestControls, mii);
    }
}

void UIScene_AvatarSelectMenu::updateTooltips()
{
    hasFocus(m_playerId);
}
void UIScene_AvatarSelectMenu::updateComponents()
{
}
void UIScene_AvatarSelectMenu::handleDestroy()
{
}
void UIScene_AvatarSelectMenu::handleLoseFocus()
{
}

void UIScene_AvatarSelectMenu::handleGainFocus(bool hadFocus)
{
    updateTooltips();
    if (hadFocus)
    {
        gApp.SetLiveLinkRequired(true);
        m_initialised = false;
    }
}

void UIScene_AvatarSelectMenu::handleTimerComplete(int)
{
}

bool UIScene_AvatarSelectMenu::handleKeyDown(int, int action, bool)
{
    if (m_initialised)
        return false;
    bool consumed = false;
    switch (action)
    {
    case EControllerActions_MenuB:
    case EControllerActions_MenuCancel:
        gApp.RequestSignInUIWiiUReturnedSelected(NULL, m_choice, m_pad);
        gConsoleUIController.NavigateBack(GetPad(), false, EUIScene_None, EUILayer_Count);
        consumed = true;
        break;
    case EControllerActions_MenuRight:
    case EControllerActions_MenuRightScroll:
        m_currentTab = m_currentTab + 1;
        if (m_currentTab >= 3)
            m_currentTab = 0;
        SelectTab(m_currentTab);
        updateTooltips();
        break;
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuLeftScroll:
        m_currentTab = m_currentTab - 1;
        if (m_currentTab < 0)
            m_currentTab = 2;
        SelectTab(m_currentTab);
        updateTooltips();
        break;
    }
    return consumed;
}

void UIScene_AvatarSelectMenu::handleInput(int, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    if (m_initialised)
        return;
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    if (!pressed)
        return;
    switch (action)
    {
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        handled = true;
        break;
    default:
        handled = handleKeyDown(m_playerId, action, repeat);
        break;
    }
}

void UIScene_AvatarSelectMenu::handleFocusChange(int focus, int childFocus)
{
    switch (focus)
    {
    case 0:
        m_userFocus = childFocus;
        break;
    case 1:
        m_favouriteFocus = childFocus;
        break;
    case 2:
        m_guestFocus = childFocus;
        break;
    }
    updateTooltips();
}

void UIScene_AvatarSelectMenu::handleInitFocus(int, int)
{
}

void UIScene_AvatarSelectMenu::handleTouchInput(unsigned int playerId, int touchX, int touchY, int touchBox,
                                                bool pressed, bool repeat, bool released)
{
    if (!pressed)
        return;
    switch (touchBox)
    {
    case 3:
    case 4:
    case 5:
    {
        int tab = touchBox - 3;
        if (m_activeTab != tab)
        {
            m_activeTab = tab;
            updateTooltips();
            SelectTab(m_activeTab);
        }
        break;
    }
    }
}

UIControl* UIScene_AvatarSelectMenu::GetMainPanel()
{
    switch (m_activeTab)
    {
    case 1:
        return &m_favouriteControls.m_tab;
    case 2:
        return &m_guestControls.m_tab;
    default:
        return &m_userControls.m_tab;
    }
}

void UIScene_AvatarSelectMenu::tick()
{
    UIScene::tick();
    if (g_pInput->ButtonPressed(gProfile->GetPrimaryPad(), EControllerActions_MenuCancel, 0))
    {
        gApp.RequestSignInUIWiiUReturnedSelected(NULL, m_choice, m_pad);
        gConsoleUIController.NavigateBack(GetPad(), false, EUIScene_None, EUILayer_Count);
    }
    if (m_navigateBack)
        navigateBack();
    if (hasFocus(m_playerId) && !m_miisAdded)
    {
        AddMiiItems();
        m_miisAdded = true;
    }
}

void UIScene_AvatarSelectMenu::SelectTab(int tab)
{
    m_activeTab = tab;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    FJ_AvatarSelectMenu* menu = (FJ_AvatarSelectMenu*)m_movie->getRootNode()->getFuiNode();
    menu->SetActiveTab(tab);
    gConsoleUIController.TouchBoxRebuild(this);
}

std::wstring UIScene_AvatarSelectMenu::getMoviePath()
{
    return std::wstring(L"AvatarSelectMenu");
}

void UIScene_AvatarSelectMenu::handlePress(int controlId, int childId)
{
    MiiDataEntry* mii = NULL;
    switch (controlId)
    {
    case 0:
        mii = m_userControls.m_miis[childId];
        gApp.RequestSignInUIWiiUReturnedSelected(mii, m_choice, m_pad);
        navigateBack();
        break;
    case 1:
        mii = m_favouriteControls.m_miis[childId];
        gApp.RequestSignInUIWiiUReturnedSelected(mii, m_choice, m_pad);
        navigateBack();
        break;
    case 2:
        mii = m_guestControls.m_miis[childId];
        gApp.RequestSignInUIWiiUReturnedSelected(mii, m_choice, m_pad);
        navigateBack();
        break;
    default:
        gApp.RequestSignInUIWiiUReturnedSelected(mii, m_choice, m_pad);
        navigateBack();
        break;
    }
}

int UIScene_AvatarSelectMenu::getSceneType()
{
    return EUIScene_LoadCreateJoinMenu;
}
unsigned int UIScene_AvatarSelectMenu::GetPad()
{
    return (unsigned int)m_playerId;
}

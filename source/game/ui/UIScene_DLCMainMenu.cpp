#include "game/ui/UIScene_DLCMainMenu.hpp"
#include "game/core/StringIds.hpp"

#include <stdint.h>
#include <string>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/cCommerce.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_DynamicButtonList.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/core/GameSettings.hpp"

extern ConsoleUIController gConsoleUIController;
extern CConsoleMinecraftApp gApp;
extern C_4JProfile* gProfile;

UIScene_DLCMainMenu::UIScene_DLCMainMenu(int type, void* data, UILayer* layer)
    : UIScene(type, layer),
      m_titleLabel(),
      m_timer()
{
    initialiseMovie();
    gApp.SetLiveLinkRequired(true);
    m_playerId = type;
    m_titleLabel.init(UIString(IDS_DOWNLOADABLE_CONTENT_OFFERS));
    m_offerList.init(0);
    gApp.LoadLocalDLCImages();
    m_timer.setVisible(true);
    m_initialized = false;
    cCommerce* commerce = static_cast<cCommerce*>((void*)gApp.GetCommerceInstance());
    commerce->ShowStoreIcon();
    addTimer(0, 100);
}

UIScene_DLCMainMenu::~UIScene_DLCMainMenu()
{
    gApp.SetLiveLinkRequired(false);
    gApp.FreeLocalDLCImages();
    cCommerce* commerce = static_cast<cCommerce*>((void*)gApp.GetCommerceInstance());
    commerce->HideStoreIcon();
}

unsigned int UIScene_DLCMainMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    {
        m_offerList.setupControl(this, root, std::string("OffersList"));
        m_controls.push_back(&m_offerList);
    }
    {
        m_titleLabel.setupControl(this, root, std::string("OffersList_Title"));
        m_controls.push_back(&m_titleLabel);
    }
    {
        m_timer.setupControl(this, root, std::string("Timer"));
        m_controls.push_back(&m_timer);
    }
    return 1;
}

int UIScene_DLCMainMenu::ExitDLCMainMenu(void* context, int, C4JStorage::EMessageResult)
{
    cCommerce* commerce = static_cast<cCommerce*>((void*)gApp.GetCommerceInstance());
    commerce->HideStoreIcon();
    static_cast<UIScene*>(context)->navigateBack();
    return 0;
}

void UIScene_DLCMainMenu::handleTimerComplete(int timerId)
{
    if (timerId == 0 && !gProfile->IsSignedInLive(gProfile->GetPrimaryPad()))
    {
        int parameter = static_cast<int>(IDS_CONFIRM_OK);
        gConsoleUIController.RequestErrorMessage(
            IDS_CONNECTION_LOST, CGameNetworkManager::sInstance.CorrectErrorIDS(IDS_CONNECTION_LOST_LIVE_NO_EXIT),
            &parameter, 1, gProfile->GetPrimaryPad(), ExitDLCMainMenu, this, NULL, true);
    }
}

void UIScene_DLCMainMenu::handleGainFocus(bool hadFocus)
{
    updateTooltips();
    if (hadFocus)
        addTimer(0, 100);
}

void UIScene_DLCMainMenu::tick()
{
    UIScene::tick();
    if (!m_initialized)
    {
        m_initialized = true;
        m_timer.setVisible(false);
        m_offerList.addItem(UIString(IDS_MAP_PACKS), 0);
        m_offerList.addItem(UIString(IDS_DLC_MENU_SKINPACKS), 1);
        m_offerList.addItem(UIString(IDS_DLC_MENU_TEXTUREPACKS), 2);
        m_offerList.addItem(UIString(IDS_DLC_MENU_MASHUPPACKS), 3);
        m_offerList.addItem(UIString(IDS_DLC_MENU_BUNDLEPACKS), 4);
    }
}

void UIScene_DLCMainMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, false, false);
}

std::wstring UIScene_DLCMainMenu::getMoviePath()
{
    return std::wstring(L"DLCMainMenu");
}

void UIScene_DLCMainMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            cCommerce* commerce = static_cast<cCommerce*>((void*)gApp.GetCommerceInstance());
            commerce->HideStoreIcon();
            navigateBack();
        }
        break;
    case EControllerActions_MenuOk:
        if (pressed)
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        sendInputToMovie(action, repeat, pressed, released);
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuRight:
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}

void UIScene_DLCMainMenu::handlePress(int controlId, int childId)
{
    if (controlId == 0)
    {
        int index = childId;
        struct OffersData
        {
            int playerId;
            unsigned char reserved[4];
            int category;
        };
        OffersData* data = new OffersData();
        data->playerId = m_playerId;
        data->category = index;
        killTimer(0);
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_DLCOffersMenu, data, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
    }
}

int UIScene_DLCMainMenu::getSceneType()
{
    return EUIScene_DLCMainMenu;
}

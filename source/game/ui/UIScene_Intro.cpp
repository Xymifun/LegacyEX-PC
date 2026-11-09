#include "game/ui/UIScene_Intro.hpp"

#include <string>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/ui/FJ_Intro.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "platform/wiiu/CProfile.hpp"

uint8_t g_introRunning = 0;

UIScene_Intro::UIScene_Intro(int sceneType, void*, UILayer* layer) : UIScene(sceneType, layer)
{
    initialiseMovie();
    m_inputHandled = false;
    m_introFocus = false;
    setIntroPlatform();
    m_touchControl.init(0);
}

void UIScene_Intro::setIntroPlatform()
{
    fuiRenderNode* root = m_movie->getRootNode();
    FJ_Intro* intro = static_cast<FJ_Intro*>(root->getFuiNode());
    intro->SetIntroPlatform(6, true, false);
}

std::wstring UIScene_Intro::getMoviePath()
{
    return std::wstring(L"Intro");
}

void UIScene_Intro::handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    if (action != EControllerActions_MenuOk || m_inputHandled)
        return;

    m_inputHandled = true;
    if (gApp.HasNoSpaceForSaveQuota() == 0)
    {
        uint32_t optionsStatus = gApp.GetOptionsCallbackStatus(gProfile->GetPrimaryPad());
        switch (optionsStatus)
        {
        case C4JStorage::eOptions_Callback_Read:
        case C4JStorage::eOptions_Callback_Read_FileNotFound:
            if (GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_EULAAccepted) == 0)
            {
                gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_EULA, NULL, EUILayer_Scene,
                                                     EUIGroup_PlayerDefault);
                g_introRunning = 1;
                return;
            }
            gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_SaveMessage, NULL, EUILayer_Scene,
                                                 EUIGroup_PlayerDefault);
            g_introRunning = 1;
            return;
        case C4JStorage::eOptions_Callback_Idle:
            m_inputHandled = false;
            g_introRunning = 1;
            return;
        default:
            gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_EULA, NULL, EUILayer_Scene,
                                                 EUIGroup_PlayerDefault);
            break;
        }
    }
    g_introRunning = 1;
}

void UIScene_Intro::handleAnimationEnd()
{
    g_introRunning = 1;
    if (m_inputHandled)
        return;

    m_inputHandled = true;
    if (gApp.HasNoSpaceForSaveQuota() != 0)
        return;

    uint32_t optionsStatus = gApp.GetOptionsCallbackStatus(gProfile->GetPrimaryPad());
    switch (optionsStatus)
    {
    case C4JStorage::eOptions_Callback_Read:
    case C4JStorage::eOptions_Callback_Read_FileNotFound:
    {
        int eulaAccepted = GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_EULAAccepted);
        int primaryPlayer = gProfile->GetPrimaryPad();
        if (eulaAccepted == 0)
        {
            gConsoleUIController.NavigateToScene(primaryPlayer, EUIScene_EULA, NULL, EUILayer_Scene,
                                                 EUIGroup_PlayerDefault);
            return;
        }
        gConsoleUIController.NavigateToScene(primaryPlayer, EUIScene_SaveMessage, NULL, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        return;
    }
    case C4JStorage::eOptions_Callback_Idle:
        gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_SaveMessage, NULL, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        return;
    default:
        gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_EULA, NULL, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        return;
    }
}

void UIScene_Intro::handleGainFocus(bool)
{
    if (!m_introFocus)
        return;
    gConsoleUIController.NavigateToScene(0, EUIScene_MainMenu, NULL, EUILayer_Scene, EUIGroup_PlayerDefault);
}

void UIScene_Intro::handleReload()
{
    setIntroPlatform();
}

void UIScene_Intro::handleTouchInput(unsigned int playerId, int, int, int, bool, bool, bool released)
{
    if (released)
    {
        bool handled = false;
        handleInput(playerId, 20, false, true, false, handled);
    }
}

unsigned int UIScene_Intro::mapElementsAndNames()
{
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_touchControl.setupControl(this, currentNode, std::string("TouchToSkip"));
        m_controls.push_back(&m_touchControl);
    }
    return 1;
}

int UIScene_Intro::getSceneType()
{
    return EUIScene_Intro;
}

bool UIScene_Intro::hasFocus(int)
{
    return m_hasFocus;
}

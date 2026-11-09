#include "game/ui/UIScene_HelpAndOptionsMenu.hpp"
#include "game/core/StringIds.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameType.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/core/GameSettings.hpp"

extern ConsoleUIController gConsoleUIController;

UIScene_HelpAndOptionsMenu::UIScene_HelpAndOptionsMenu(int playerId, void*, UILayer* layer) : UIScene(playerId, layer)
{
    initialiseMovie();
    m_unknownFlag = Minecraft::GetInstance()->mLevel == NULL;
    m_buttons[0].init(UIString(IDS_CHANGE_SKIN), 0);
    m_buttons[1].init(UIString(IDS_HOW_TO_PLAY), 1);
    m_buttons[2].init(UIString(IDS_CONTROLS), 2);
    m_buttons[3].init(UIString(IDS_SETTINGS), 3);
    m_buttons[4].init(UIString(IDS_CREDITS), 4);
    m_buttons[6].init(UIString(IDS_DEBUG_SETTINGS), 6);
    removeControl(&m_buttons[7], false);
    removeControl(&m_buttons[5], false);
    removeControl(&m_buttons[6], false);
    if (m_playerId != gProfile->GetPrimaryPad() || !m_unknownFlag)
        removeControl(&m_buttons[5], false);
    if (gApp.GetLocalPlayerCount() > 1)
    {
        removeControl(&m_buttons[4], false);
        if (gProfile->GetPrimaryPad() != m_playerId)
            removeControl(&m_buttons[5], false);
    }
    boost::shared_ptr<MultiplayerLocalPlayer>& player = Minecraft::GetInstance()->mLocalPlayers[playerId];
    if (!gProfile->IsFullVersion()
        || (Minecraft::InMiniGame(eMiniGameId_Any, true) && player.get() != NULL
            && player.get()->GetGameType() != GameType::LOBBY && !gApp.DebugArtToolsOn()))
        removeControl(&m_buttons[0], false);
    doHorizontalResizeCheck();
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_HelpAndOptionsMenu::~UIScene_HelpAndOptionsMenu()
{
}

unsigned int UIScene_HelpAndOptionsMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    {
        m_buttons[0].setupControl(this, root, std::string("Button1"));
        m_controls.push_back(&m_buttons[0]);
    }
    {
        m_buttons[1].setupControl(this, root, std::string("Button2"));
        m_controls.push_back(&m_buttons[1]);
    }
    {
        m_buttons[2].setupControl(this, root, std::string("Button3"));
        m_controls.push_back(&m_buttons[2]);
    }
    {
        m_buttons[3].setupControl(this, root, std::string("Button4"));
        m_controls.push_back(&m_buttons[3]);
    }
    {
        m_buttons[4].setupControl(this, root, std::string("Button5"));
        m_controls.push_back(&m_buttons[4]);
    }
    {
        m_buttons[5].setupControl(this, root, std::string("Button6"));
        m_controls.push_back(&m_buttons[5]);
    }
    {
        m_buttons[6].setupControl(this, root, std::string("Button7"));
        m_controls.push_back(&m_buttons[6]);
    }
    {
        m_buttons[7].setupControl(this, root, std::string("ButtonUpsell"));
        m_controls.push_back(&m_buttons[7]);
    }
    return 1;
}

void UIScene_HelpAndOptionsMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_HelpAndOptionsMenu::updateComponents()
{
    bool showLogo;
    bool notInGame = Minecraft::GetInstance()->mLevel == NULL;
    if (notInGame)
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
        showLogo = true;
    }
    else
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, false);
        showLogo = false;
        if (gApp.GetLocalPlayerCount() == 1)
            showLogo = true;
    }
    m_layer->showComponent(m_playerId, EUIScene_Logo, showLogo);
}

std::wstring UIScene_HelpAndOptionsMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() > 1 ? std::wstring(L"HelpAndOptionsMenuSplit")
                                          : std::wstring(L"HelpAndOptionsMenu");
}

void UIScene_HelpAndOptionsMenu::handleReload()
{
    removeControl(&m_buttons[6], false);
    bool notInGame = Minecraft::GetInstance()->mLevel == NULL;
    if (m_playerId != gProfile->GetPrimaryPad() || !notInGame)
        removeControl(&m_buttons[5], false);
    if (gApp.GetLocalPlayerCount() > 1)
    {
        removeControl(&m_buttons[4], false);
        if (gProfile->GetPrimaryPad() != m_playerId)
            removeControl(&m_buttons[5], false);
    }
    gProfile->IsFullVersion();
    boost::shared_ptr<MultiplayerLocalPlayer>& player = Minecraft::GetInstance()->mLocalPlayers[m_playerId];
    if (!gProfile->IsFullVersion()
        || (Minecraft::InMiniGame(eMiniGameId_Any, true) && player->GetGameType() != GameType::LOBBY))
        removeControl(&m_buttons[0], false);
    doHorizontalResizeCheck();
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_HelpAndOptionsMenu::handleInput(int, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed && !repeat)
            navigateBack();
        break;
    case EControllerActions_MenuOk:
        if (pressed)
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        sendInputToMovie(action, repeat, pressed, released);
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}

void UIScene_HelpAndOptionsMenu::handlePress(int controlId, int)
{
    switch (controlId)
    {
    case 0:
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_SkinSelectMenu, NULL, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        break;
    case 1:
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_HowToPlayMenu, NULL, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        break;
    case 2:
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_ControlsMenu, NULL, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        break;
    case 3:
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_SettingsMenu, NULL, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        break;
    case 4:
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_Credits, NULL, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        break;
    case 5:
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_ReinstallMenu, NULL, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        break;
    case 6:
        gConsoleUIController.NavigateToScene(m_playerId, static_cast<EUIScene>(13), NULL, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        break;
    }
}

int UIScene_HelpAndOptionsMenu::getSceneType()
{
    return EUIScene_HelpAndOptionsMenu;
}

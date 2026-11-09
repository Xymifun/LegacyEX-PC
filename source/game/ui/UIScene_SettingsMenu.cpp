#include "game/ui/UIScene_SettingsMenu.hpp"
#include "game/core/StringIds.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/UIString.hpp"
#include "platform/wiiu/CProfile.hpp"
#ifdef MINECRAFT_PC
#include "game/network/ClientPacketListener.hpp"
#include "platform/pc/PcGoldenAge.hpp"
#include "platform/pc/PcSettings.hpp"
#endif

extern C4JStorage* gStorage;

UIScene_SettingsMenu::UIScene_SettingsMenu(int type, void*, UILayer* layer) : UIScene(type, layer)
{
    initialiseMovie();

    bool destroyControls = Minecraft::GetInstance()->mLevel == NULL;

    m_buttons[0].init(UIString((int)IDS_GAME_OPTIONS), 0);
    m_buttons[1].init(UIString((int)IDS_AUDIO), 1);
#ifdef MINECRAFT_PC
    m_buttons[2].init(UIString((int)IDS_GRAPHICS), 4);
    m_buttons[4].init(UIString(L"Golden Age"), 7);
#else
    m_buttons[4].init(UIString((int)IDS_GRAPHICS), 4);
#endif
    m_buttons[5].init(UIString((int)IDS_USER_INTERFACE), 5);
    m_buttons[6].init(UIString((int)IDS_RESET_TO_DEFAULTS), 6);

#ifdef MINECRAFT_PC
    RemoveButtons(destroyControls);
#else
    removeControl(&m_buttons[2], false);

    if (gProfile->GetPrimaryPad() != m_playerId)
    {
        removeControl(&m_buttons[1], destroyControls);
        if (!Minecraft::InMiniGame(eMiniGameId_Glide, true)
            || Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) == 0)
            removeControl(&m_buttons[4], destroyControls);
    }
#endif

    m_language = GameSettings::GetMinecraftLanguage(m_playerId);
    doHorizontalResizeCheck();
    gApp.GetLocalPlayerCount();
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_SettingsMenu::~UIScene_SettingsMenu()
{
}

unsigned int UIScene_SettingsMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
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
        m_buttons[4].setupControl(this, root, std::string("Button4"));
        m_controls.push_back(&m_buttons[4]);
    }
    {
        m_buttons[5].setupControl(this, root, std::string("Button5"));
        m_controls.push_back(&m_buttons[5]);
    }
    {
        m_buttons[6].setupControl(this, root, std::string("Button6"));
        m_controls.push_back(&m_buttons[6]);
    }

    return 1;
}

void UIScene_SettingsMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, (int)IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_SettingsMenu::updateComponents()
{
    bool showComponents;
    bool noLevel = Minecraft::GetInstance()->mLevel == NULL;
    if (noLevel)
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
        showComponents = true;
    }
    else
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, false);
        showComponents = false;
        if (gApp.GetLocalPlayerCount() == 1)
            showComponents = true;
    }
    m_layer->showComponent(m_playerId, EUIScene_Logo, showComponents);
}

#ifdef MINECRAFT_PC
void UIScene_SettingsMenu::RemoveButtons(bool destroyControls)
{
    if (gProfile->GetPrimaryPad() != m_playerId)
    {
        removeControl(&m_buttons[1], destroyControls);
        if (!Minecraft::InMiniGame(eMiniGameId_Glide, true)
            || Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) == 0)
            removeControl(&m_buttons[2], destroyControls);
        removeControl(&m_buttons[4], destroyControls);
    }
}
#endif

void UIScene_SettingsMenu::handleReload()
{
    bool destroyControls = Minecraft::GetInstance()->mLevel == NULL;
#ifdef MINECRAFT_PC
    RemoveButtons(destroyControls);
#else
    removeControl(&m_buttons[2], destroyControls);

    if (gProfile->GetPrimaryPad() != m_playerId)
    {
        removeControl(&m_buttons[1], destroyControls);
        if (!Minecraft::InMiniGame(eMiniGameId_Glide, true)
            || Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) == 0)
            removeControl(&m_buttons[4], destroyControls);
    }
#endif

    m_language = GameSettings::GetMinecraftLanguage(m_playerId);
    doHorizontalResizeCheck();
    gConsoleUIController.TouchBoxRebuild(this);
}

std::wstring UIScene_SettingsMenu::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1)
        return std::wstring(L"SettingsMenuSplit");
    return std::wstring(L"SettingsMenu");
}

void UIScene_SettingsMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
                                       bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            GameSettings::CheckGameSettingsChanged(true, playerId);
            navigateBack();
        }
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}

void UIScene_SettingsMenu::tick()
{
    UIScene::tick();
    if (gConsoleUIController.GetTopScene(m_playerId, EUILayer_Scene, EUIGroup_PlayerDefault) == this)
    {
        unsigned char language = GameSettings::GetMinecraftLanguage(m_playerId);
        if (m_language != language)
        {
            m_language = language;
            doHorizontalResizeCheck();
        }
    }
}

int UIScene_SettingsMenu::ResetDefaultsDialogReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    UIScene_SettingsMenu* scene = static_cast<UIScene_SettingsMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context))));
    if (scene != NULL && result == C4JStorage::EMessageResult_Decline)
    {
        GameSettings::SetDefaultGameOptions(gStorage->GetDashboardProfileSettings(scene->m_playerId), scene->m_playerId,
                                            true);
        GameSettings::CheckGameSettingsChanged(true, button);
#ifdef MINECRAFT_PC
        if (scene->m_playerId == gProfile->GetPrimaryPad())
        {
            const int goldenAge = PcSettingsGoldenAge();
            PcSettingsReset();
            ClientPacketListener::SendViewDistanceToAll();
            PcGoldenAgeRefresh(goldenAge);
        }
#endif
    }
    return 0;
}

void UIScene_SettingsMenu::handlePress(int controlId, int)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    switch (controlId)
    {
    case 0:
    {
        ListMenuInitData* data = new ListMenuInitData();
        data->mode = 0;
        data->playerId = m_playerId;
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_SettingsListMenu, data, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        resetHorizontalResizeCheck();
        break;
    }
    case 1:
    {
        ListMenuInitData* data = new ListMenuInitData();
        data->mode = 1;
        data->playerId = m_playerId;
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_SettingsListMenu, data, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        break;
    }
    case 4:
    {
        ListMenuInitData* data = new ListMenuInitData();
        data->mode = 2;
        data->playerId = m_playerId;
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_SettingsListMenu, data, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        break;
    }
    case 5:
    {
        ListMenuInitData* data = new ListMenuInitData();
        data->mode = 3;
        data->playerId = m_playerId;
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_SettingsListMenu, data, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        break;
    }
#ifdef MINECRAFT_PC
    case 7:
    {
        ListMenuInitData* data = new ListMenuInitData();
        data->mode = 4;
        data->playerId = m_playerId;
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_SettingsListMenu, data, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        break;
    }
#endif
    case 6:
    {
        int parameters[2] = {static_cast<int>(IDS_CONFIRM_CANCEL), static_cast<int>(IDS_CONFIRM_OK)};
        gConsoleUIController.RequestAlertMessage(IDS_DEFAULTS_TITLE, IDS_DEFAULTS_TEXT, parameters, 2, m_playerId,
                                                 ResetDefaultsDialogReturned,
                                                 (void*)static_cast<unsigned long>(GetCallbackUniqueId()), NULL, true);
        break;
    }
    }
}

int UIScene_SettingsMenu::getSceneType()
{
    return EUIScene_SettingsMenu;
}

#include "game/ui/UIScene_InGameHostOptionsMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>
#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/command/DefaultGameModeCommand.hpp"
#include "game/command/GameDifficultyCommand.hpp"
#include "game/command/SetDefaultSpawnpointCommand.hpp"
#include "game/command/TimeCommand.hpp"
#include "game/command/WeatherCommand.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameType.hpp"
#include "game/core/L10N.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/network/ServerSettingsChangedPacket.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_InGameHostOptions.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/GameSettings.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcSettings.hpp"

static const int PROXIMITY_CHAT_ITEM = 0x20;
static const int PROXIMITY_RANGE_ITEM = 0x21;

static void ProximityModeLabel(int mode, wchar_t* text)
{
    static const wchar_t* const names[kPcHostProximityCount] = {L"Users Choose", L"Force Proximity",
                                                                 L"Disable Proximity"};
    swprintf(text, 256, L"Proximity Chat: %ls", names[mode]);
}

static void ProximityRangeLabel(int blocks, wchar_t* text)
{
    if (blocks == 0)
        swprintf(text, 256, L"Proximity Range: Users Choose");
    else
        swprintf(text, 256, L"Proximity Range: %d blocks", blocks);
}
#endif

extern ConsoleUIController gConsoleUIController;

UIScene_InGameHostOptionsMenu::UIScene_InGameHostOptionsMenu(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer),
      m_hostOptions()
{
    initialiseMovie();
    Init();
}

unsigned int UIScene_InGameHostOptionsMenu::getWeatherId(int weatherState)
{
    unsigned int id = IDS_WEATHER_CLEAR;
    if (weatherState == 1)
        id = IDS_WEATHER_RAIN;
    else if (weatherState == 2)
        id = IDS_WEATHER_THUNDER;
    return id;
}

void UIScene_InGameHostOptionsMenu::Init()
{
    m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_FIRE_SPREADS)), 0,
                                 gApp.GetGameHostOption(eGameHostOption_FireSpreads) != 0);
    m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_TNT_EXPLODES)), 1,
                                 gApp.GetGameHostOption(eGameHostOption_TNT) != 0);
    if (gApp.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0)
    {
        m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_DAYLIGHT_CYCLE)), 2,
                                     gApp.GetGameHostOption(eGameHostOption_DoDaylightCycle) != 0);
        m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_KEEP_INVENTORY)), 3,
                                     gApp.GetGameHostOption(eGameHostOption_KeepInventory) != 0);
        m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MOB_SPAWNING)), 4,
                                     gApp.GetGameHostOption(eGameHostOption_DoMobSpawning) != 0);
        m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MOB_GRIEFING)), 5,
                                     gApp.GetGameHostOption(eGameHostOption_MobGriefing) != 0);
        m_hostOptions.AddNewButton(std::wstring(L10N::GetString(IDS_TIME_SET_DAY)), 0xc, true);
        m_hostOptions.AddNewButton(std::wstring(L10N::GetString(IDS_TIME_SET_NIGHT)), 0xd, true);

        unsigned int difficultyValue = gApp.GetGameHostOption(eGameHostOption_Difficulty);
        wchar_t difficultyLabel[256];
        swprintf(difficultyLabel, 256, L"%ls: %ls", L10N::GetString(IDS_SLIDER_DIFFICULTY),
                 L10N::GetString(Difficulty::byId(difficultyValue)->getKey()));
        m_hostOptions.AddNewSlider(std::wstring(difficultyLabel), 0xf, 0, 3, difficultyValue, false, 7, 3);

        unsigned int gameTypeValue = gApp.GetGameHostOption(eGameHostOption_GameType);
        const GameType* gameType = GameType::byId(gameTypeValue);
        m_hostOptions.AddNewSlider(std::wstring(L10N::GetString(gameType->getSliderLabel())), 0x10, 0,
                                   GameType::ADVENTURE->getId(), gameTypeValue, false, 7, 3);
        m_hostOptions.AddNewButton(std::wstring(L10N::GetString(IDS_SET_WORLD_SPAWN)), 0xe, true);

        Level* level = Minecraft::GetInstance()->getLevel(0);
        if (level != 0)
        {
            m_hostOptions.AddNewLabel(std::wstring(L" "), 0x12);
            m_hostOptions.AddNewLabel(std::wstring(L10N::GetString(IDS_WEATHER)), 0x12);
            int weatherState = Minecraft::GetInstance()->getLevel(0)->getWeatherState();
            m_hostOptions.AddNewSlider(std::wstring(L10N::GetString(getWeatherId(weatherState))), 0x11, 0, 2,
                                       weatherState, false, 7, 3);
        }
        m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_WEATHER_CYCLE)), 9,
                                     gApp.GetGameHostOption(eGameHostOption_DoWeatherCycle) != 0);
    }
    m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MOB_LOOT)), 6,
                                 gApp.GetGameHostOption(eGameHostOption_DoMobLoot) != 0);
    m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_TILE_DROPS)), 7,
                                 gApp.GetGameHostOption(eGameHostOption_DoTileDrops) != 0);
    m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_NATURAL_REGEN)), 8,
                                 gApp.GetGameHostOption(eGameHostOption_NaturalRegeneration) != 0);

#ifdef MINECRAFT_PC
    if (CGameNetworkManager::sInstance.IsHost() && !CGameNetworkManager::sInstance.IsLocalGame())
    {
        wchar_t label[256];
        ProximityModeLabel(PcSettingsHostProximity(), label);
        m_hostOptions.AddNewSlider(std::wstring(label), PROXIMITY_CHAT_ITEM, 0, kPcHostProximityCount - 1,
                                   PcSettingsHostProximity(), false, 7, 3);
        ProximityRangeLabel(PcSettingsHostProximityRange(), label);
        m_hostOptions.AddNewSlider(std::wstring(label), PROXIMITY_RANGE_ITEM, 0,
                                   kPcProximityRangeMaximum / kPcProximityRangeStep,
                                   PcSettingsHostProximityRange() / kPcProximityRangeStep, false, 7, 3);
    }
#endif
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    if (player != 0)
    {
        unsigned int privileges = gApp.GetPlayerPrivileges(player->GetSmallId());
        if (gApp.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0
            && Player::getPlayerGamePrivilege(privileges, Player::ePlayerGamePrivilege_CanTeleport) != 0
            && CGameNetworkManager::sInstance.GetPlayerCount() > 1)
        {
            m_hostOptions.AddNewButton(std::wstring(L10N::GetString(IDS_TELEPORT_TO_PLAYER)), 0xa, true);
            m_hostOptions.AddNewButton(std::wstring(L10N::GetString(IDS_TELEPORT_TO_ME)), 0xb, true);
        }
    }
    fuiRenderNode* root = m_movie->getRootNode();
    static_cast<FJ_InGameHostOptions*>(root->getFuiNode())->InitMenu();
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_InGameHostOptionsMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_InGameHostOptionsMenu::handleReload()
{
    Init();
    gConsoleUIController.TouchBoxRebuild(this);
}

std::wstring UIScene_InGameHostOptionsMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() > 1 ? std::wstring(L"InGameHostOptionsSplit")
                                          : std::wstring(L"InGameHostOptions");
}

void UIScene_InGameHostOptionsMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
                                                bool& handled)
{
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            bool settingsChanged = false;
            unsigned int options = gApp.GetGameHostOption(eGameHostOption_All);
            gApp.SetGameHostOption(options, eGameHostOption_FireSpreads, m_hostOptions.GetCheckboxValue(0));
            gApp.SetGameHostOption(options, eGameHostOption_TNT, m_hostOptions.GetCheckboxValue(1));
            if (gApp.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0)
            {
                gApp.SetGameHostOption(options, eGameHostOption_DoDaylightCycle, m_hostOptions.GetCheckboxValue(2));
                gApp.SetGameHostOption(options, eGameHostOption_KeepInventory, m_hostOptions.GetCheckboxValue(3));
                gApp.SetGameHostOption(options, eGameHostOption_DoMobSpawning, m_hostOptions.GetCheckboxValue(4));
                gApp.SetGameHostOption(options, eGameHostOption_MobGriefing, m_hostOptions.GetCheckboxValue(5));
                gApp.SetGameHostOption(options, eGameHostOption_DoWeatherCycle, m_hostOptions.GetCheckboxValue(9));
                unsigned int currentDifficulty = gApp.GetGameHostOption(eGameHostOption_Difficulty);
                unsigned int difficulty = m_hostOptions.GetSliderValue(0xf);
                if (currentDifficulty != difficulty)
                {
                    boost::shared_ptr<MultiplayerLocalPlayer> player =
                        Minecraft::GetInstance()->mLocalPlayers[m_playerId];
                    if (player != NULL && player->mPacketListener != NULL)
                    {
                        settingsChanged = true;
                        player->mPacketListener->send(GameDifficultyCommand::preparePacket(difficulty));
                    }
                }
                unsigned int currentGameType = gApp.GetGameHostOption(eGameHostOption_GameType);
                unsigned int gameType = m_hostOptions.GetSliderValue(0x10);
                if (currentGameType != gameType)
                {
                    boost::shared_ptr<MultiplayerLocalPlayer> player =
                        Minecraft::GetInstance()->mLocalPlayers[m_playerId];
                    if (player != NULL && player->mPacketListener != NULL)
                    {
                        settingsChanged = true;
                        player->mPacketListener->send(
                            DefaultGameModeCommand::preparePacket(static_cast<unsigned char>(gameType)));
                    }
                }
                if (Minecraft::GetInstance()->getLevel(0) != NULL)
                {
                    unsigned int weather = Minecraft::GetInstance()->getLevel(0)->getWeatherState();
                    unsigned int newWeather = m_hostOptions.GetSliderValue(0x11);
                    if (weather != newWeather)
                    {
                        boost::shared_ptr<MultiplayerLocalPlayer> player =
                            Minecraft::GetInstance()->mLocalPlayers[m_playerId];
                        if (player != NULL && player->mPacketListener != NULL)
                        {
                            player->mPacketListener->send(
                                WeatherCommand::preparePacket(static_cast<unsigned char>(newWeather)));
                        }
                    }
                }
            }
            gApp.SetGameHostOption(options, eGameHostOption_DoMobLoot, m_hostOptions.GetCheckboxValue(6));
            gApp.SetGameHostOption(options, eGameHostOption_DoTileDrops, m_hostOptions.GetCheckboxValue(7));
            gApp.SetGameHostOption(options, eGameHostOption_NaturalRegeneration, m_hostOptions.GetCheckboxValue(8));
            if (settingsChanged || options != gApp.GetGameHostOption(eGameHostOption_All))
            {
                boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[m_playerId];
                if (player->mPacketListener != NULL)
                {
                    player->mPacketListener->send(
                        boost::shared_ptr<ServerSettingsChangedPacket>(new ServerSettingsChangedPacket(1, options)));
                }
            }
            navigateBack();
            handled = true;
        }
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuRight:
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}

void UIScene_InGameHostOptionsMenu::handlePress(int, int childId)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    switch (childId)
    {
    case 10:
    case 11:
    {
        struct SceneData
        {
            int player;
            bool value;
            char padding[3];
        };
        SceneData* data = new SceneData();
        data->player = m_playerId;
        data->value = false;
        if (childId == 10)
            data->value = true;
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_TeleportMenu, data, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        break;
    }
    case 12:
    {
        ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
        connection->send(TimeCommand::preparePacket(1000));
        break;
    }
    case 13:
    {
        ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
        connection->send(TimeCommand::preparePacket(14000));
        break;
    }
    case 14:
    {
        ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
        connection->send(SetDefaultSpawnpointCommand::preparePacket());
        break;
    }
    }
}

void UIScene_InGameHostOptionsMenu::handleCheckboxElementToggled(int, int, bool)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
}

void UIScene_InGameHostOptionsMenu::handleSliderElementMove(int, int element, int value)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    switch (element)
    {
    case 0xf:
    {
        wchar_t label[256];
        swprintf(label, 256, L"%ls: %ls", L10N::GetString(IDS_SLIDER_DIFFICULTY),
                 L10N::GetString(Difficulty::byId(value)->getKey()));
        m_hostOptions.SetItemLabel(0xf, UIString(label), false);
        break;
    }
    case 0x10:
    {
        const GameType* gameType = GameType::byId(value);
        m_hostOptions.SetItemLabel(element, UIString(L10N::GetString(gameType->getSliderLabel())), false);
        break;
    }
    case 0x11:
        m_hostOptions.SetItemLabel(element, UIString(L10N::GetString(getWeatherId(value))), false);
        break;
#ifdef MINECRAFT_PC
    case PROXIMITY_CHAT_ITEM:
    {
        PcSettingsSetHostProximity(value);
        PcSettingsSave();
        wchar_t label[256];
        ProximityModeLabel(PcSettingsHostProximity(), label);
        m_hostOptions.SetItemLabel(element, UIString(label), false);
        break;
    }
    case PROXIMITY_RANGE_ITEM:
    {
        PcSettingsSetHostProximityRange(value * kPcProximityRangeStep);
        PcSettingsSave();
        wchar_t label[256];
        ProximityRangeLabel(PcSettingsHostProximityRange(), label);
        m_hostOptions.SetItemLabel(element, UIString(label), false);
        break;
    }
#endif
    }
}

unsigned int UIScene_InGameHostOptionsMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    {
        m_hostOptions.setupControl(this, root, std::string("HostOptionsList"));
        m_controls.push_back(&m_hostOptions);
    }
    return 1;
}

int UIScene_InGameHostOptionsMenu::getSceneType()
{
    return EUIScene_InGameHostOptionsMenu;
}

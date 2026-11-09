#include "game/ui/UIScene_InGamePlayerOptionsMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>
#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/command/GameModeCommand.hpp"
#include "game/command/SetSpawnpointCommand.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameType.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PlayerInfoPacket.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_InGamePlayerOptions.hpp"
#include "game/ui/UIScene_InGameInfoMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/IUIScene_PlayerList.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/GameSettings.hpp"

extern ConsoleUIController gConsoleUIController;

UIScene_InGamePlayerOptionsMenu::UIScene_InGamePlayerOptionsMenu(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer)
{
    initialiseMovie();
    const InGamePlayerOptionsData* optionsData = static_cast<const InGamePlayerOptionsData*>(data);
    m_privileges = optionsData->privileges;
    m_cheatsEnabled = false;
    m_navigatingBack = false;
    m_smallId = optionsData->smallId;

    INetworkPlayer* localPlayer = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(m_smallId);
    if (player != NULL)
    {
        std::wstring displayName = player->GetDisplayName();
        m_gamertag.init(UIString(displayName));
    }
    unsigned int trustPlayers = gApp.GetGameHostOption(eGameHostOption_TrustPlayers);
    bool cheatsEnabled = gApp.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;
    bool isHost = localPlayer != NULL && localPlayer == player;
    m_isHost = isHost;
    bool checked;

    if (isHost)
    {
        if (cheatsEnabled)
        {
            bool canToggleInvisible =
                Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleInvisible) != 0;
            checked = canToggleInvisible
                      && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Invisible) != 0
                      && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Invulnerable) != 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_INVISIBLE), ePlayerOption_Invisible, checked);
            m_playerOptions.EnableItem(ePlayerOption_Invisible, canToggleInvisible, true);
            if (Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CreativeMode) == 0)
            {
                bool canToggleFly =
                    Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleFly) != 0;
                bool canToggleExhaustion =
                    Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleExhaustion) != 0;
                checked = canToggleFly
                          && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanFly) != 0;
                m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_FLY), ePlayerOption_Fly, checked);
                m_playerOptions.EnableItem(ePlayerOption_Fly, canToggleFly, true);
                checked = canToggleExhaustion
                          && Player::getPlayerGamePrivilege(m_privileges,
                                                            Player::ePlayerGamePrivilege_ExhaustionDisabled)
                                 != 0;
                m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_DISABLE_EXHAUSTION), ePlayerOption_DisableExhaustion,
                                               checked);
                m_playerOptions.EnableItem(ePlayerOption_DisableExhaustion, canToggleExhaustion, true);
            }
            unsigned int gameType = gApp.GetPlayerGameType(m_smallId);
            m_playerOptions.AddNewSlider(L10N::GetString(GameType::byId(gameType)->getSliderLabel()),
                                         ePlayerOption_GameMode, 0, GameType::ADVENTURE->getId(), gameType, false, 7,
                                         3);
            m_playerOptions.AddNewButton(L10N::GetString(IDS_SET_PLAYER_SPAWN), ePlayerOption_SetSpawn, true);
        }
    }
    else
    {
        if (trustPlayers == 0 && player != NULL && !player->IsHost())
        {
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotMine) == 0
                      && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotBuild) == 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_BUILD_AND_MINE), ePlayerOption_BuildAndMine,
                                           checked);
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches)
                      != 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_USE_DOORS_AND_SWITCHES),
                                           ePlayerOption_UseDoorsAndSwitches, checked);
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanUseContainers) != 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_OPEN_CONTAINERS), ePlayerOption_OpenContainers,
                                           checked);
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotAttackPlayers)
                      == 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_ATTACK_PLAYERS), ePlayerOption_AttackPlayers,
                                           checked);
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotAttackAnimals)
                      == 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_ATTACK_ANIMALS), ePlayerOption_AttackAnimals,
                                           checked);
        }

        if (localPlayer->IsHost())
        {
            m_cheatsEnabled = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Op) != 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_MODERATOR), ePlayerOption_Moderator,
                                           Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Op)
                                               != 0);
            if (cheatsEnabled)
            {
                checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleInvisible)
                          != 0;
                m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_INVISIBLE), ePlayerOption_Invisible, checked);
                m_playerOptions.EnableItem(ePlayerOption_Invisible, true, true);
                if (Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CreativeMode) == 0)
                {
                    checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleFly)
                              != 0;
                    m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_FLY), ePlayerOption_Fly, checked);
                    m_playerOptions.EnableItem(ePlayerOption_Fly, true, true);
                    checked = Player::getPlayerGamePrivilege(m_privileges,
                                                             Player::ePlayerGamePrivilege_CanToggleExhaustion)
                              != 0;
                    m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_DISABLE_EXHAUSTION),
                                                   ePlayerOption_DisableExhaustion, checked);
                    m_playerOptions.EnableItem(ePlayerOption_DisableExhaustion, true, true);
                }
                checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanTeleport) != 0;
                m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_ENABLE_TELEPORT), ePlayerOption_Teleport, checked);
                unsigned int gameType = gApp.GetPlayerGameType(m_smallId);
                m_playerOptions.AddNewSlider(L10N::GetString(GameType::byId(gameType)->getSliderLabel()),
                                             ePlayerOption_GameMode, 0, GameType::ADVENTURE->getId(), gameType, false,
                                             7, 3);
                m_playerOptions.AddNewButton(L10N::GetString(IDS_SET_PLAYER_SPAWN), ePlayerOption_SetSpawn, true);
            }
            resetCheatCheckboxes();
        }

        if (player != NULL && !player->IsLocal()
            && !player->IsSameSystem(CGameNetworkManager::sInstance.GetHostPlayer()))
            m_playerOptions.AddNewButton(L10N::GetString(IDS_KICK_PLAYER), ePlayerOption_Kick, true);
    }

    int playerColour = gApp.GetPlayerColour(m_smallId);
    static_cast<FJ_InGamePlayerOptions*>(m_movie->getRootNode()->getFuiNode())->SetPlayerIcon(playerColour);
    resetCheatCheckboxes();
    addTimer(0, 100);
    CGameNetworkManager::sInstance.RegisterPlayerChangedCallback(m_playerId, OnPlayerChanged, this);
    static_cast<FJ_InGamePlayerOptions*>(m_movie->getRootNode()->getFuiNode())->InitMenu();
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_InGamePlayerOptionsMenu::resetCheatCheckboxes()
{
    bool cheatsEnabled = gApp.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;
    if (cheatsEnabled && !m_isHost)
    {
        bool cheats = false;
        if (m_playerOptions.IsCheckbox(ePlayerOption_Moderator))
        {
            cheats = m_playerOptions.GetCheckboxValue(ePlayerOption_Moderator);
        }
        if (m_playerOptions.IsCheckbox(ePlayerOption_Invisible))
        {
            m_playerOptions.EnableItem(ePlayerOption_Invisible, cheats, true);
            m_playerOptions.SetCheckboxValue(
                ePlayerOption_Invisible,
                cheats
                    && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleInvisible)
                           != 0,
                true);
        }
        bool creativeMode = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CreativeMode)
                            != 0;
        if (!creativeMode)
        {
            m_playerOptions.EnableItem(ePlayerOption_Fly, cheats, true);
            m_playerOptions.SetCheckboxValue(
                ePlayerOption_Fly,
                cheats && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleFly) != 0,
                true);
            m_playerOptions.EnableItem(ePlayerOption_DisableExhaustion, cheats, true);
            m_playerOptions.SetCheckboxValue(
                ePlayerOption_DisableExhaustion,
                cheats
                    && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleExhaustion)
                           != 0,
                true);
        }
        if (m_playerOptions.IsCheckbox(ePlayerOption_Invisible))
        {
            m_playerOptions.EnableItem(ePlayerOption_Teleport, cheats, true);
            m_playerOptions.SetCheckboxValue(
                ePlayerOption_Teleport,
                cheats && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanTeleport) != 0,
                true);
        }
    }
}

void UIScene_InGamePlayerOptionsMenu::OnPlayerChanged(void* context, INetworkPlayer* player, bool added)
{
    UIScene_InGamePlayerOptionsMenu* scene = static_cast<UIScene_InGamePlayerOptionsMenu*>(context);
    UIScene_InGameInfoMenu* infoMenu = static_cast<UIScene_InGameInfoMenu*>(scene->getBackScene());
    if (infoMenu != 0)
        IUIScene_PlayerList::OnPlayerChanged(static_cast<IUIScene_PlayerList*>(infoMenu), player, added);
    if (added && player != 0 && player->GetSmallId() == scene->m_smallId)
        scene->m_navigatingBack = true;
}

unsigned int UIScene_InGamePlayerOptionsMenu::mapElementsAndNames()
{
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_playerOptions.setupControl(this, currentNode, std::string("PlayerOptionsList"));
        m_controls.push_back(&m_playerOptions);
    }
    {
        m_gamertag.setupControl(this, currentNode, std::string("Gamertag"));
        m_controls.push_back(&m_gamertag);
    }
    return 1;
}

void UIScene_InGamePlayerOptionsMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_InGamePlayerOptionsMenu::handleReload()
{
    INetworkPlayer* localPlayer = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(m_smallId);
    bool trustPlayers = gApp.GetGameHostOption(eGameHostOption_TrustPlayers) != 0;
    bool cheatsEnabled = gApp.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;
    m_isHost = localPlayer != NULL && localPlayer == player;
    bool isHost = m_isHost;
    bool checked;

    if (isHost)
    {
        if (cheatsEnabled)
        {
            bool canToggleInvisible =
                Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleInvisible) != 0;
            checked = canToggleInvisible
                      && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Invisible) != 0
                      && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Invulnerable) != 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_INVISIBLE), ePlayerOption_Invisible, checked);
            m_playerOptions.EnableItem(ePlayerOption_Invisible, canToggleInvisible, true);
            bool creativeMode = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CreativeMode)
                                != 0;
            if (!creativeMode)
            {
                bool canToggleFly =
                    Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleFly) != 0;
                bool canToggleExhaustion =
                    Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleExhaustion) != 0;
                checked = canToggleFly
                          && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanFly) != 0;
                m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_FLY), ePlayerOption_Fly, checked);
                m_playerOptions.EnableItem(ePlayerOption_Fly, canToggleFly, true);
                checked = canToggleExhaustion
                          && Player::getPlayerGamePrivilege(m_privileges,
                                                            Player::ePlayerGamePrivilege_ExhaustionDisabled)
                                 != 0;
                m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_DISABLE_EXHAUSTION), ePlayerOption_DisableExhaustion,
                                               checked);
                m_playerOptions.EnableItem(ePlayerOption_DisableExhaustion, canToggleExhaustion, true);
            }
            unsigned int gameType = gApp.GetPlayerGameType(m_smallId);
            m_playerOptions.AddNewSlider(L10N::GetString(GameType::byId(gameType)->getSliderLabel()),
                                         ePlayerOption_GameMode, 0, GameType::ADVENTURE->getId(), gameType, false, 7,
                                         3);
            m_playerOptions.AddNewButton(L10N::GetString(IDS_SET_PLAYER_SPAWN), ePlayerOption_SetSpawn, true);
        }
    }
    else
    {
        if (trustPlayers == 0 && player != NULL && !player->IsHost())
        {
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotMine) == 0
                      && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotBuild) == 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_BUILD_AND_MINE), ePlayerOption_BuildAndMine,
                                           checked);
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches)
                      != 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_USE_DOORS_AND_SWITCHES),
                                           ePlayerOption_UseDoorsAndSwitches, checked);
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanUseContainers) != 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_OPEN_CONTAINERS), ePlayerOption_OpenContainers,
                                           checked);
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotAttackPlayers)
                      == 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_ATTACK_PLAYERS), ePlayerOption_AttackPlayers,
                                           checked);
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotAttackAnimals)
                      == 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_ATTACK_ANIMALS), ePlayerOption_AttackAnimals,
                                           checked);
        }

        if (localPlayer->IsHost())
        {
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_MODERATOR), ePlayerOption_Moderator,
                                           Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Op)
                                               != 0);
            if (cheatsEnabled)
            {
                checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleInvisible)
                          != 0;
                m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_INVISIBLE), ePlayerOption_Invisible, checked);
                m_playerOptions.EnableItem(ePlayerOption_Invisible, true, true);
                bool creativeMode =
                    Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CreativeMode) != 0;
                if (!creativeMode)
                {
                    checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleFly)
                              != 0;
                    m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_FLY), ePlayerOption_Fly, checked);
                    m_playerOptions.EnableItem(ePlayerOption_Fly, true, true);
                    checked = Player::getPlayerGamePrivilege(m_privileges,
                                                             Player::ePlayerGamePrivilege_CanToggleExhaustion)
                              != 0;
                    m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_DISABLE_EXHAUSTION),
                                                   ePlayerOption_DisableExhaustion, checked);
                    m_playerOptions.EnableItem(ePlayerOption_DisableExhaustion, true, true);
                }
                checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanTeleport) != 0;
                m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_ENABLE_TELEPORT), ePlayerOption_Teleport, checked);
                unsigned int gameType = gApp.GetPlayerGameType(m_smallId);
                m_playerOptions.AddNewSlider(L10N::GetString(GameType::byId(gameType)->getSliderLabel()),
                                             ePlayerOption_GameMode, 0, GameType::ADVENTURE->getId(), gameType, false,
                                             7, 3);
                m_playerOptions.AddNewButton(L10N::GetString(IDS_SET_PLAYER_SPAWN), ePlayerOption_SetSpawn, true);
            }
            resetCheatCheckboxes();
        }

        if (player != NULL && !player->IsLocal()
            && !player->IsSameSystem(CGameNetworkManager::sInstance.GetHostPlayer()))
            m_playerOptions.AddNewButton(L10N::GetString(IDS_KICK_PLAYER), ePlayerOption_Kick, true);
    }

    int playerColour = gApp.GetPlayerColour(m_smallId);
    static_cast<FJ_InGamePlayerOptions*>(m_movie->getRootNode()->getFuiNode())->SetPlayerIcon(playerColour);
    static_cast<FJ_InGamePlayerOptions*>(m_movie->getRootNode()->getFuiNode())->InitMenu();
    gConsoleUIController.TouchBoxRebuild(this);
}

std::wstring UIScene_InGamePlayerOptionsMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() > 1 ? std::wstring(L"InGamePlayerOptionsSplit")
                                          : std::wstring(L"InGamePlayerOptions");
}

void UIScene_InGamePlayerOptionsMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
                                                  bool& handled)
{
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            INetworkPlayer* localPlayer = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
            INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(m_smallId);
            m_privileges = gApp.GetPlayerPrivileges(m_smallId);
            bool sentGameModeChange = false;
            const bool hostOption = gApp.GetGameHostOption(eGameHostOption_TrustPlayers) != 0;
            const bool cheatsEnabled = gApp.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;
            bool host = localPlayer != 0 && localPlayer == player;
            m_isHost = host;

            if (host)
            {
                if (cheatsEnabled)
                {
                    bool canToggleInvisible = Player::getPlayerGamePrivilege(
                                                  m_privileges, Player::ePlayerGamePrivilege_CanToggleInvisible)
                                              != 0;
                    if (canToggleInvisible)
                    {
                        Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Invisible,
                                                       m_playerOptions.GetCheckboxValue(ePlayerOption_Invisible));
                        Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Invulnerable,
                                                       m_playerOptions.GetCheckboxValue(ePlayerOption_Invisible));
                    }
                    bool creativeMode =
                        Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CreativeMode) != 0;
                    if (!creativeMode)
                    {
                        bool canToggleFly = Player::getPlayerGamePrivilege(m_privileges,
                                                                           Player::ePlayerGamePrivilege_CanToggleFly)
                                            != 0;
                        bool canToggleExhaustion = Player::getPlayerGamePrivilege(
                                                       m_privileges, Player::ePlayerGamePrivilege_CanToggleExhaustion)
                                                   != 0;
                        if (canToggleFly)
                        {
                            Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanFly,
                                                           m_playerOptions.GetCheckboxValue(ePlayerOption_Fly));
                        }
                        if (canToggleExhaustion)
                        {
                            Player::setPlayerGamePrivilege(
                                m_privileges, Player::ePlayerGamePrivilege_ExhaustionDisabled,
                                m_playerOptions.GetCheckboxValue(ePlayerOption_DisableExhaustion));
                        }
                    }
                    unsigned int currentGameType = gApp.GetPlayerGameType(m_smallId);
                    unsigned int gameType = m_playerOptions.GetSliderValue(ePlayerOption_GameMode);
                    if (currentGameType != gameType)
                    {
                        boost::shared_ptr<MultiplayerLocalPlayer> localMultiplayerPlayer =
                            Minecraft::GetInstance()->mLocalPlayers[m_playerId];
                        if (localMultiplayerPlayer && localMultiplayerPlayer->mPacketListener != 0)
                        {
                            sentGameModeChange = true;
                            localMultiplayerPlayer->mPacketListener->send(
                                GameModeCommand::preparePacket(player->GetUID(), static_cast<unsigned char>(gameType)));
                        }
                    }
                }
            }
            else
            {
                if (!hostOption && player != 0 && !player->IsHost())
                {
                    Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotMine,
                                                   !m_playerOptions.GetCheckboxValue(ePlayerOption_BuildAndMine));
                    Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotBuild,
                                                   !m_playerOptions.GetCheckboxValue(ePlayerOption_BuildAndMine));
                    Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches,
                                                   m_playerOptions.GetCheckboxValue(ePlayerOption_UseDoorsAndSwitches));
                    Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanUseContainers,
                                                   m_playerOptions.GetCheckboxValue(ePlayerOption_OpenContainers));
                    Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotAttackPlayers,
                                                   !m_playerOptions.GetCheckboxValue(ePlayerOption_AttackPlayers));
                    Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotAttackAnimals,
                                                   !m_playerOptions.GetCheckboxValue(ePlayerOption_AttackAnimals));
                }

                if (localPlayer->IsHost())
                {
                    Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Op,
                                                   m_playerOptions.GetCheckboxValue(ePlayerOption_Moderator));
                    if (cheatsEnabled)
                    {
                        Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleInvisible,
                                                       m_playerOptions.GetCheckboxValue(ePlayerOption_Invisible));
                        bool creativeMode = Player::getPlayerGamePrivilege(m_privileges,
                                                                           Player::ePlayerGamePrivilege_CreativeMode)
                                            != 0;
                        if (!creativeMode)
                        {
                            Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleFly,
                                                           m_playerOptions.GetCheckboxValue(ePlayerOption_Fly));
                            Player::setPlayerGamePrivilege(
                                m_privileges, Player::ePlayerGamePrivilege_CanToggleExhaustion,
                                m_playerOptions.GetCheckboxValue(ePlayerOption_DisableExhaustion));
                        }
                        Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanTeleport,
                                                       m_playerOptions.GetCheckboxValue(ePlayerOption_Teleport));

                        unsigned int currentGameType = gApp.GetPlayerGameType(m_smallId);
                        unsigned int gameType = m_playerOptions.GetSliderValue(ePlayerOption_GameMode);
                        if (currentGameType != gameType)
                        {
                            boost::shared_ptr<MultiplayerLocalPlayer> localMultiplayerPlayer =
                                Minecraft::GetInstance()->mLocalPlayers[m_playerId];
                            if (localMultiplayerPlayer && localMultiplayerPlayer->mPacketListener != 0)
                            {
                                sentGameModeChange = true;
                                localMultiplayerPlayer->mPacketListener->send(GameModeCommand::preparePacket(
                                    player->GetUID(), static_cast<unsigned char>(gameType)));
                            }
                        }
                    }
                }
            }

            unsigned int currentPrivileges = gApp.GetPlayerPrivileges(m_smallId);
            if (sentGameModeChange || currentPrivileges != m_privileges)
            {
                boost::shared_ptr<MultiplayerLocalPlayer> localMultiplayerPlayer =
                    Minecraft::GetInstance()->mLocalPlayers[m_playerId];
                if (localMultiplayerPlayer && localMultiplayerPlayer->mPacketListener != 0)
                {
                    unsigned int gameTypeByte = gApp.GetPlayerGameType(m_smallId);
                    if (localPlayer->IsHost() && cheatsEnabled)
                        gameTypeByte = m_playerOptions.GetSliderValue(ePlayerOption_GameMode);
                    localMultiplayerPlayer->mPacketListener->send(boost::shared_ptr<PlayerInfoPacket>(
                        new PlayerInfoPacket(m_smallId, -1, m_privileges, gameTypeByte)));
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

void UIScene_InGamePlayerOptionsMenu::handleDestroy()
{
    CGameNetworkManager::sInstance.UnRegisterPlayerChangedCallback(m_playerId, OnPlayerChanged, this);
}

void UIScene_InGamePlayerOptionsMenu::handlePress(int, int childId)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    switch (childId)
    {
    case ePlayerOption_Kick:
    {
        unsigned char* data = new unsigned char();
        *data = m_smallId;
        int parameters[2] = {(int)IDS_CONFIRM_OK, (int)IDS_CONFIRM_CANCEL};
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
        {
            gConsoleUIController.RequestAlertMessage(IDS_UNLOCK_KICK_PLAYER_TITLE, IDS_UNLOCK_KICK_PLAYER_MINIGAMES,
                                                     parameters, 2, m_playerId, IUIScene_PlayerList::KickPlayerReturned,
                                                     data, 0, true);
        }
        else
        {
            gConsoleUIController.RequestAlertMessage(IDS_UNLOCK_KICK_PLAYER_TITLE, IDS_UNLOCK_KICK_PLAYER, parameters,
                                                     2, m_playerId, IUIScene_PlayerList::KickPlayerReturned, data, 0,
                                                     true);
        }
        break;
    }
    case ePlayerOption_SetSpawn:
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(m_smallId);
        ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
        connection->send(SetSpawnpointCommand::preparePacket(player->GetUID()));
        break;
    }
    }
}

void UIScene_InGamePlayerOptionsMenu::handleCheckboxToggled(int, bool)
{
}

void UIScene_InGamePlayerOptionsMenu::handleTimerComplete(int timerId)
{
    if (timerId == 0 && !m_isHost)
    {
        unsigned char cheats = 0;
        if (m_playerOptions.IsCheckbox(ePlayerOption_Moderator))
            cheats = m_playerOptions.GetCheckboxValue(ePlayerOption_Moderator);
        if (m_cheatsEnabled != cheats)
        {
            m_cheatsEnabled = cheats;
            resetCheatCheckboxes();
        }
    }
}

void UIScene_InGamePlayerOptionsMenu::tick()
{
    UIScene::tick();
    if (m_navigatingBack)
    {
        if (!gConsoleUIController.IsSceneInStack(0xff, EUIScene_MessageBox))
            m_navigatingBack = false;
        gConsoleUIController.NavigateBack(m_playerId, false, EUIScene_None, EUILayer_Count);
    }
}

void UIScene_InGamePlayerOptionsMenu::handleCheckboxElementToggled(int, int, bool)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
}

void UIScene_InGamePlayerOptionsMenu::handleSliderElementMove(int, int element, int value)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    if (element == ePlayerOption_GameMode)
    {
        m_playerOptions.SetItemLabel(ePlayerOption_GameMode,
                                     UIString(L10N::GetString(GameType::byId(value)->getSliderLabel())), false);
    }
}

int UIScene_InGamePlayerOptionsMenu::getSceneType()
{
    return EUIScene_InGamePlayerOptionsMenu;
}

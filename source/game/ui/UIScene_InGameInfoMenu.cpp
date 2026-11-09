#include "game/ui/UIScene_InGameInfoMenu.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcDedicated.hpp"
#include "platform/pc/PcVoice.hpp"
#endif
#include "game/core/StringIds.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/ui/UIControl_PlayerButtonList.hpp"
#include "game/ui/UIScene_InGamePlayerOptionsMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "vendor/_XPARTY_USER_INFO.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/generation/PlayerUID.hpp"

extern ConsoleUIController gConsoleUIController;

UIScene_InGameInfoMenu::UIScene_InGameInfoMenu(int type, void* data, UILayer* layer)
    : UIScene(type, layer),
      IUIScene_PlayerList(),
      m_players()
{
    initialiseMovie();
    m_optionsButton.init(UIString(L10N::GetString(IDS_HOST_OPTIONS)), 0);
    m_titleLabel.init(UIString(L10N::GetString(IDS_PLAYERS_INVITE)));
    m_playerList.init(1);
    m_players = std::vector<PlayerInfo*>();

    unsigned int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
    for (unsigned int i = 0; i < playerCount; ++i)
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerByIndex(i);
#ifdef MINECRAFT_PC
        if (PcIsHiddenHost(player))
            continue;
#endif
        if (player != NULL)
        {
            PlayerInfo* info = CreatePlayerInfo();
            BuildPlayerInfo(player, info);
            m_players.push_back(info);
            m_playerList.addItem(info->name, static_cast<int>(info->colour), static_cast<int>(info->voiceStatus), true);
        }
    }
    CGameNetworkManager::sInstance.RegisterPlayerChangedCallback(m_playerId, IUIScene_PlayerList::OnPlayerChanged,
                                                                 static_cast<IUIScene_PlayerList*>(this));
    INetworkPlayer* localNetworkPlayer = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    m_isHost = false;
    if (localNetworkPlayer != NULL)
        m_isHost = localNetworkPlayer->IsHost();
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[m_playerId];
    if (!m_isHost && !localPlayer->isModerator())
        removeControl(&m_optionsButton, false);
    updateTooltips();
    gConsoleUIController.HidePressStart();
}

UIScene_InGameInfoMenu::~UIScene_InGameInfoMenu()
{
    for (unsigned int i = 0; i < m_players.size(); ++i)
        delete m_players[i];
}

int UIScene_InGameInfoMenu::GetPlayerCount()
{
    return static_cast<int>(m_players.size());
}

PlayerInfo* UIScene_InGameInfoMenu::GetPlayer(int index)
{
    return m_players[index];
}

bool UIScene_InGameInfoMenu::RemovePlayer(unsigned char smallId)
{
    int index = FindPlayer(smallId);
    bool found = index >= 0;
    if (found)
    {
        delete m_players[index];
        m_players.erase(m_players.begin() + index);
    }
    return found;
}

void UIScene_InGameInfoMenu::AddPlayer(unsigned char smallId)
{
    PlayerInfo* info = CreatePlayerInfo();
    BuildPlayerInfo(CGameNetworkManager::sInstance.GetPlayerBySmallId(smallId), info);
    m_players.push_back(info);
}

UIControl_ButtonList* UIScene_InGameInfoMenu::GetPlayerList()
{
    return static_cast<UIControl_ButtonList*>(static_cast<void*>(&m_playerList));
}

void UIScene_InGameInfoMenu::UpdatePlayer(int index, PlayerInfo* oldInfo, PlayerInfo* newInfo)
{
    IUIScene_PlayerList::UpdatePlayer(index, oldInfo, newInfo);
    if (newInfo->voiceStatus != oldInfo->voiceStatus || hasAPlayerChanged())
    {
        oldInfo->voiceStatus = newInfo->voiceStatus;
        m_playerList.setVOIPIcon(index, static_cast<int>(oldInfo->voiceStatus));
    }
}

bool UIScene_InGameInfoMenu::hasSomethingToEdit(unsigned char smallId)
{
    bool result = false;
    INetworkPlayer* local = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(smallId);
    bool trustPlayers = gApp.GetGameHostOption(eGameHostOption_TrustPlayers) != 0;
    bool cheatsEnabled = gApp.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;
    if (local != NULL && local == player)
    {
        if (cheatsEnabled)
            result = true;
    }
    else
    {
        if (!trustPlayers && player != NULL && !player->IsHost())
            result = true;
        if (local->IsHost())
            result = true;
        if (!player->IsLocal() && !player->IsSameSystem(CGameNetworkManager::sInstance.GetHostPlayer()))
            result = true;
    }
    return result;
}

void UIScene_InGameInfoMenu::updateTooltips()
{
    int yTooltip = -1;
    int tooltip = -1;
    int partyTooltip = -1;
    _XPARTY_USER_LIST users;
    if (XPartyGetUserList(&users) != 0 && users.count > 1)
        partyTooltip = static_cast<int>(IDS_TOOLTIPS_INVITE_PARTY);
    if (CGameNetworkManager::sInstance.IsLocalGame())
        partyTooltip = -1;
    int selected = m_playerList.getCurrentSelection();
    if (selected < m_players.size())
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(m_players[selected]->smallId);
        boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[m_playerId];
        if (m_isHost || localPlayer->isModerator())
        {
            if (m_optionsButton.hasFocus())
            {
                tooltip = static_cast<int>(IDS_TOOLTIPS_SELECT);
            }
            else if (player != NULL)
            {
                if (hasSomethingToEdit(m_players[m_playerList.getCurrentSelection()]->smallId))
                    tooltip = static_cast<int>(IDS_TOOLTIPS_PRIVILEGES);
                else if (!player->IsLocal() && !player->IsSameSystem(CGameNetworkManager::sInstance.GetHostPlayer()))
                    tooltip = static_cast<int>(IDS_TOOLTIPS_KICK);
            }
        }
    }
#ifdef MINECRAFT_PC
    partyTooltip = -1;
    if (m_playerList.hasFocus() && selected < m_players.size())
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(m_players[selected]->smallId);
        if (player != NULL && !player->IsLocal() && player->HasVoice())
            partyTooltip = static_cast<int>(PcVoiceMuted(player->GetSmallId()) ? ePcString_Unmute : ePcString_Mute);
    }
#endif
    gConsoleUIController.SetTooltips(m_playerId, tooltip, static_cast<int>(IDS_TOOLTIPS_BACK), partyTooltip, yTooltip,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

unsigned int UIScene_InGameInfoMenu::mapElementsAndNames()
{
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_optionsButton.setupControl(this, currentNode, std::string("GameOptions"));
        m_controls.push_back(&m_optionsButton);
    }
    {
        m_playerList.setupControl(this, currentNode, std::string("GamePlayers"));
        m_controls.push_back(&m_playerList);
    }
    {
        m_titleLabel.setupControl(this, currentNode, std::string("Title"));
        m_controls.push_back(&m_titleLabel);
    }
    return 1;
}

void UIScene_InGameInfoMenu::handleReload()
{
    unsigned int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
    for (unsigned int i = 0; i < m_players.size(); ++i)
        delete m_players[i];
    m_players.clear();
    for (unsigned int i = 0; i < playerCount; ++i)
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerByIndex(i);
#ifdef MINECRAFT_PC
        if (PcIsHiddenHost(player))
            continue;
#endif
        if (player != NULL)
        {
            PlayerInfo* info = CreatePlayerInfo();
            BuildPlayerInfo(player, info);
            m_players.push_back(info);
            m_playerList.addItem(info->name, static_cast<int>(info->colour), static_cast<int>(info->voiceStatus), true);
        }
    }

    INetworkPlayer* localNetworkPlayer = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    m_isHost = false;
    if (localNetworkPlayer != NULL)
        m_isHost = localNetworkPlayer->IsHost();
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[m_playerId];
    if (!m_isHost && !localPlayer->isModerator())
        removeControl(&m_optionsButton, false);
    updateTooltips();
    if (controlHasFocus(1))
        m_playerList.setCurrentSelection(getControlChildFocus(), false, true);
}

std::wstring UIScene_InGameInfoMenu::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1)
        return std::wstring(L"InGameInfoMenuSplit");
    return std::wstring(L"InGameInfoMenu");
}

void UIScene_InGameInfoMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
                                         bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed && !repeat)
        {
            gConsoleUIController.PlayUISFX(SoundEvents::UI_BACK);
            navigateBack();
        }
        break;
    case EControllerActions_MenuY:
    {
        if (!pressed)
            return;
        if (!m_playerList.hasFocus())
            return;
        if (m_playerList.getItemCount() < 1)
            return;
        int playerCount = m_players.size();
        unsigned int selection = m_playerList.getCurrentSelection();
        if (selection >= playerCount)
            return;
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(m_players[selection]->smallId);
        if (player == NULL)
            return;
        PlayerUID uid = player->GetUID();
        uid != PlayerUID(0);
        return;
    }
    case EControllerActions_MenuX:
        if (!pressed || repeat)
            return;
#ifdef MINECRAFT_PC
        if (m_playerList.hasFocus() && m_playerList.getCurrentSelection() < m_players.size())
        {
            INetworkPlayer* player =
                CGameNetworkManager::sInstance.GetPlayerBySmallId(m_players[m_playerList.getCurrentSelection()]->smallId);
            if (player != NULL && !player->IsLocal() && player->HasVoice())
            {
                PcVoiceSetMuted(player->GetSmallId(), !PcVoiceMuted(player->GetSmallId()));
                gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
                updateTooltips();
            }
        }
#endif
        CGameNetworkManager::sInstance.IsLocalGame();
        return;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    default:
        break;
    }
}

void UIScene_InGameInfoMenu::handleGainFocus(bool hadFocus)
{
    if (hadFocus)
        CGameNetworkManager::sInstance.RegisterPlayerChangedCallback(m_playerId, IUIScene_PlayerList::OnPlayerChanged,
                                                                     static_cast<IUIScene_PlayerList*>(this));
}

void UIScene_InGameInfoMenu::handlePress(int controlId, int childId)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    switch (controlId)
    {
    case 0:
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_InGameHostOptionsMenu, NULL, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        break;
    case 1:
    {
        PlayerInfo* info = m_players[childId];
        INetworkPlayer* networkPlayer = CGameNetworkManager::sInstance.GetPlayerBySmallId(info->smallId);
        boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[m_playerId];
        if (m_isHost || localPlayer->isModerator())
        {
            if (networkPlayer != NULL)
            {
                if (hasSomethingToEdit(m_players[childId]->smallId))
                {
                    InGamePlayerOptionsData* data = new InGamePlayerOptionsData();
                    data->playerId = m_playerId;
                    data->smallId = m_players[childId]->smallId;
                    data->privileges = gApp.GetPlayerPrivileges(m_players[childId]->smallId);
                    gConsoleUIController.NavigateToScene(m_playerId, EUIScene_InGamePlayerOptionsMenu, data,
                                                         EUILayer_Scene, EUIGroup_PlayerDefault);
                }
                else if (!networkPlayer->IsLocal()
                         && !networkPlayer->IsSameSystem(CGameNetworkManager::sInstance.GetHostPlayer()))
                {
                    unsigned char* kickedSmallId = new unsigned char(0);
                    *kickedSmallId = m_players[childId]->smallId;
                    int parameters[2] = {static_cast<int>(IDS_CONFIRM_OK), static_cast<int>(IDS_CONFIRM_CANCEL)};
                    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
                        gConsoleUIController.RequestAlertMessage(
                            IDS_UNLOCK_KICK_PLAYER_TITLE, IDS_UNLOCK_KICK_PLAYER_MINIGAMES, parameters, 2, m_playerId,
                            IUIScene_PlayerList::KickPlayerReturned, kickedSmallId, 0, true);
                    else
                        gConsoleUIController.RequestAlertMessage(
                            IDS_UNLOCK_KICK_PLAYER_TITLE, IDS_UNLOCK_KICK_PLAYER, parameters, 2, m_playerId,
                            IUIScene_PlayerList::KickPlayerReturned, kickedSmallId, 0, true);
                }
            }
        }
        break;
    }
    }
}

void UIScene_InGameInfoMenu::handleDestroy()
{
    CGameNetworkManager::sInstance.UnRegisterPlayerChangedCallback(m_playerId, IUIScene_PlayerList::OnPlayerChanged,
                                                                   static_cast<IUIScene_PlayerList*>(this));
    m_layer->showComponent(m_playerId, EUIScene_MenuBackground, false);
}

void UIScene_InGameInfoMenu::handleFocusChange(int focus, int childFocus)
{
    if (focus == 1)
        m_playerList.updateChildFocus(childFocus);
    updateTooltips();
}

void UIScene_InGameInfoMenu::tick()
{
    UIScene::tick();
    IUIScene_PlayerList::tick();
}

int UIScene_InGameInfoMenu::GetPad()
{
    return m_playerId;
}

int UIScene_InGameInfoMenu::getSceneType()
{
    return EUIScene_InGameInfoMenu;
}

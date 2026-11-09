#include "game/ui/UIScene_GameModeScoreboardMenu.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/StringConversion.hpp"

#include <stdio.h>
#include <wchar.h>

#include "game/core/Minecraft.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/MG01ScoreboardLayout.hpp"
#include "game/core/MG02ScoreboardLayout.hpp"
#include "game/core/MG03ScoreboardLayout.hpp"
#include "game/core/MG04ScoreboardLayout.hpp"
#include "game/core/StatsUID.hpp"
#include "game/core/ColourTable.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/minigame/GameMedal.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/ui/FJ_GameModeScoreboard.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Item.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "vendor/_XPARTY_USER_INFO.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/PIXNamedEvents.hpp"

UIScene_GameModeScoreboardMenu::UIScene_GameModeScoreboardMenu(int type, void* data, UILayer* layer)
    : IUIScene_PlayerList(),
      UIScene(type, layer),
      m_awardPanel(),
      m_mainPanel(),
      m_killBar(),
      m_playerTitleLabel(),
      m_stat1Label(),
      m_stat2Label(),
      m_stat3Label(),
      m_stat4Label(),
      m_stat5Label(),
      m_killedByLabel(),
      m_gamertagLabel()
{
    m_statsPending = false;
    int* options = static_cast<int*>(data);
    initialiseMovie();
    m_scoreboardType = static_cast<EScoreboardType>(options[1]);
    delete options;
    m_canKickPlayers = !CGameNetworkManager::sInstance.IsInPublicJoinableGame()
                       || m_scoreboardType == EScoreboardType_Award;
    m_canVoteToKick = 0;
    if (m_scoreboardType == EScoreboardType_Award && !GetScoreboard()->HasStats())
        m_scoreboardType = EScoreboardType_Scoreboard;
    if (Minecraft::InMiniGame(eMiniGameId_Battle, true))
        m_scoreboardLayout = new MG01ScoreboardLayout(m_scoreboardType);
    else if (Minecraft::InMiniGame(eMiniGameId_Tumble, true))
        m_scoreboardLayout = new MG02ScoreboardLayout();
    else if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
        m_scoreboardLayout = new MG03ScoreboardLayout(m_scoreboardType);
    else if (Minecraft::InMiniGame(eMiniGameId_Build, true))
        m_scoreboardLayout = new MG04ScoreboardLayout();
    else
        m_scoreboardLayout = new MG01ScoreboardLayout(m_scoreboardType);
    CGameNetworkManager::sInstance.RegisterPlayerChangedCallback(m_playerId, IUIScene_PlayerList::OnPlayerChanged,
                                                                 this);
    handleReload();
}

UIScene_GameModeScoreboardMenu::~UIScene_GameModeScoreboardMenu()
{
    delete m_scoreboardLayout;
    m_scoreboardLayout = 0;
}

std::wstring UIScene_GameModeScoreboardMenu::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1)
        return std::wstring(L"GameModeScoreboardSplit");
    return std::wstring(L"GameModeScoreboard");
}

unsigned int UIScene_GameModeScoreboardMenu::mapElementsAndNames()
{
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_awardPanel.setupControl(this, currentNode, std::string("PanelAward"));
        m_controls.push_back(&m_awardPanel);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_awardPanel.getFuiRenderNode();
        {
            m_stat1Label.setupControl(this, currentNode, std::string("Stat1Label"));
            m_controls.push_back(&m_stat1Label);
        }
        {
            m_stat2Label.setupControl(this, currentNode, std::string("Stat2Label"));
            m_controls.push_back(&m_stat2Label);
        }
        {
            m_stat3Label.setupControl(this, currentNode, std::string("Stat3Label"));
            m_controls.push_back(&m_stat3Label);
        }
        {
            m_stat4Label.setupControl(this, currentNode, std::string("Stat4Label"));
            m_controls.push_back(&m_stat4Label);
        }
        {
            m_gamertagLabel.setupControl(this, currentNode, std::string("Stat5Label"));
            m_controls.push_back(&m_gamertagLabel);
        }
        {
            m_killBar.setupControl(this, currentNode, std::string("KillBar"));
            m_controls.push_back(&m_killBar);
        }
        {
            m_stat5Label.setupControl(this, currentNode, std::string("KilledByLabel"));
            m_controls.push_back(&m_stat5Label);
        }
        {
            m_killedByLabel.setupControl(this, currentNode, std::string("GamertagLabel"));
            m_controls.push_back(&m_killedByLabel);
        }
        currentNode = parentNode;
    }
    {
        m_mainPanel.setupControl(this, currentNode, std::string("PanelScoreboard"));
        m_controls.push_back(&m_mainPanel);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_mainPanel.getFuiRenderNode();
        {
            m_playerTitleLabel.setupControl(this, currentNode, std::string("PlayerTitle"));
            m_controls.push_back(&m_playerTitleLabel);
        }
        {
            m_playerList.setupControl(this, currentNode, std::string("Scoreboard"));
            m_controls.push_back(&m_playerList);
        }
        currentNode = parentNode;
    }
    return 1;
}

boost::shared_ptr<MultiplayerLocalPlayer> UIScene_GameModeScoreboardMenu::GetPlayer()
{
    return Minecraft::GetInstance()->mLocalPlayers[m_playerId];
}

ClientScoreboard* UIScene_GameModeScoreboardMenu::GetScoreboard()
{
    return GetPlayer()->GetLevel()->GetClientScoreboard();
}

void UIScene_GameModeScoreboardMenu::registerTexture(int index)
{
    if (index >= 119 || m_registeredTextures[index] != 0)
        return;
    wchar_t path[64];
    swprintf(path, 64, L"Graphics\\GameModeGraphics\\%ls.png", IScoreboardLayout::getIcon(index));
    if (gApp.hasArchiveFile(std::wstring(path), false))
    {
        arrayWithLength<unsigned char> data = gApp.getArchiveFile(std::wstring(path), false);
        if (data.length != 0 && data.data != 0)
        {
            registerSubstitutionTexture(std::wstring(IScoreboardLayout::getIcon(index)), data.data, data.length, true);
            m_registeredTextures[index] = 1;
        }
    }
}

void UIScene_GameModeScoreboardMenu::UpdateStats(const StatsUID& uid)
{
    StatsUID empty = StatsUID::EMPTY_SUID();
    if (uid != empty)
    {
        GameMedal* medal = GetScoreboard()->GetMedal(uid);
        if (medal != 0)
            m_stat1Label.setLabel(UIString(medal->GetName()), false, false);
        else
            m_stat1Label.setLabel(UIString(IDS_MG_NO_MEDAL), false, false);
        m_stat2Label.setLabel(UIString(m_scoreboardLayout->getLabel2(uid)), false, false);
        m_stat3Label.setLabel(UIString(m_scoreboardLayout->getLabel3(uid)), false, false);
        m_stat4Label.setLabel(UIString(m_scoreboardLayout->getLabel4(uid)), false, false);
        m_stat5Label.setLabel(UIString(m_scoreboardLayout->getLabel5(uid)), false, false);
        m_killedByLabel.setLabel(UIString(m_scoreboardLayout->getLabel6(uid)), false, false);
        m_gamertagLabel.setLabel(UIString(m_scoreboardLayout->getLabel7(uid)), false, false);
        m_statsPending = true;
        m_selectedUid = uid;
    }
    else
    {
        m_playerTitleLabel.init(UIString(IDS_MG_PLAYERS_TITLE));
        m_stat1Label.setLabel(UIString(L""), false, false);
        m_stat2Label.setLabel(UIString(L""), false, false);
        m_stat3Label.setLabel(UIString(L""), false, false);
        m_stat4Label.setLabel(UIString(L""), false, false);
        m_stat5Label.setLabel(UIString(L""), false, false);
        m_killedByLabel.setLabel(UIString(IDS_MG_KILLED_BY), false, false);
        m_gamertagLabel.setLabel(UIString(L""), false, false);
    }
}

void UIScene_GameModeScoreboardMenu::SetStatIconTextureName(int index, const std::wstring& name)
{
    FJ_GameModeScoreboard* document = static_cast<FJ_GameModeScoreboard*>(m_movie->getRootNode()->getFuiNode());
    document->SetStatIconTextureName(index, name);
}

void UIScene_GameModeScoreboardMenu::SetTitleIconTextureName(int index, const std::wstring& name)
{
    FJ_GameModeScoreboard* document = static_cast<FJ_GameModeScoreboard*>(m_movie->getRootNode()->getFuiNode());
    document->SetTitleIconTextureName(index, name);
}

void UIScene_GameModeScoreboardMenu::InitialiseScoreboard(INetworkPlayer* player)
{
    StatsUID uid = StatsUID::NETWORKPLAYER_SUID(player);
    m_scoreboardLayout->Init(m_playerId);
    if (m_scoreboardType == EScoreboardType_Scoreboard)
        m_playerTitleLabel.init(UIString(L10N::GetString(IDS_PLAYERS_INVITE)));
    else
    {
        m_playerTitleLabel.init(UIString(L10N::GetString(IDS_PLAYERS)));
        registerTexture(3);
        registerTexture(0x74);
        registerTexture(0x75);
        registerTexture(0x76);
        registerTexture(0x70);
        registerTexture(0x71);
        registerTexture(0x72);
        m_killBar.init(UIString(L""), 0, 0, 100, 0);
        m_killBar.setLabel(UIString(L""), false, false);
        m_killBar.setProgress(100);
        UpdateStats(uid);
    }
    SetStatIconTextureName(
        1, std::wstring(IScoreboardLayout::getIcon(m_scoreboardLayout->getStatIcon2(m_scoreboardType))));
    SetStatIconTextureName(
        2, std::wstring(IScoreboardLayout::getIcon(m_scoreboardLayout->getStatIcon3(m_scoreboardType))));
    SetStatIconTextureName(
        3, std::wstring(IScoreboardLayout::getIcon(m_scoreboardLayout->getStatIcon4(m_scoreboardType))));
    SetStatIconTextureName(
        4, std::wstring(IScoreboardLayout::getIcon(m_scoreboardLayout->getStatIcon5(m_scoreboardType))));
    for (int column = 0; column < m_columnCount; ++column)
        SetTitleIconTextureName(column, std::wstring(IScoreboardLayout::getIcon(static_cast<int>(
                                            m_scoreboardLayout->getTitleIcon(column, m_scoreboardType)))));

    int selectedPlayer = -1;
    for (unsigned int index = 0; index < static_cast<unsigned int>(m_scoreboardLayout->GetPlayerCount()); ++index)
    {
        INetworkPlayer* networkPlayer = CGameNetworkManager::sInstance.GetPlayerBySmallId(
            m_scoreboardLayout->getSmallIDOfPlayerAt(index));
        StatsUID playerUid = StatsUID::NETWORKPLAYER_SUID(networkPlayer);
        if (networkPlayer == 0)
            continue;
        ClientScoreboard* scoreboard = GetScoreboard();
        m_playerList.addItem(m_scoreboardLayout->getNameOfPlayerAt(index),
                             m_scoreboardLayout->getColorStateOfPlayerAt(index),
                             m_scoreboardLayout->getVoiceStatusOfPlayerAt(index), true);
        if (m_scoreboardType == EScoreboardType_Scoreboard)
        {
            int icon = m_scoreboardLayout->GetEndBarIcon(m_scoreboardLayout->GetPlayer(index));
            m_playerList.SetSlotTextureName(index, m_columnCount - 1, std::wstring(IScoreboardLayout::getIcon(icon)),
                                            true);
        }
        else
        {
            GameMedal* medal = scoreboard->GetMedal(playerUid);
            int medalIcon;
            if (medal != 0 && (medalIcon = medal->GetId() + 0xc) >= 0xc)
            {
                registerTexture(medalIcon);
                m_playerList.SetSlotTextureName(index, m_columnCount - 1,
                                                std::wstring(IScoreboardLayout::getIcon(medalIcon)), true);
            }
            else
            {
                m_playerList.SetSlotTextureName(index, m_columnCount - 1,
                                                std::wstring(IScoreboardLayout::getIcon(0x6e)), true);
            }
        }
        m_playerList.setSlotsText(index, m_scoreboardLayout->GetRowLabel(0, playerUid, m_scoreboardType),
                                  m_scoreboardLayout->GetRowLabel(1, playerUid, m_scoreboardType),
                                  m_scoreboardLayout->GetRowLabel(2, playerUid, m_scoreboardType),
                                  m_scoreboardLayout->GetRowLabel(3, playerUid, m_scoreboardType),
                                  m_scoreboardLayout->GetRowLabel(4, playerUid, m_scoreboardType),
                                  m_scoreboardLayout->GetRowLabel(5, playerUid, m_scoreboardType),
                                  m_scoreboardLayout->GetRowLabel(6, playerUid, m_scoreboardType), true);
        if (playerUid == uid)
            selectedPlayer = static_cast<int>(index);
    }
    m_playerList.setCurrentSelection(selectedPlayer, false, true);
    m_localPlayerCanModerate = 0;
    if (player != 0)
        m_localPlayerCanModerate = player->IsHost();
    updateTooltips();
    if (controlHasFocus(0))
        m_playerList.setCurrentSelection(static_cast<int>(getControlChildFocus()), false, true);
    GetMainPanel()->UpdateControl();
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_GameModeScoreboardMenu::SetHasPlayerBeenKilled(bool killed)
{
    FJ_GameModeScoreboard* document = static_cast<FJ_GameModeScoreboard*>(m_movie->getRootNode()->getFuiNode());
    document->SetHasPlayerBeenKilled(killed);
}

void UIScene_GameModeScoreboardMenu::tick()
{
    UIScene::tick();
    IUIScene_PlayerList::tick();
    if (m_initialisePending)
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
        if (player != 0)
        {
            m_initialisePending = 0;
            InitialiseScoreboard(player);
        }
    }
    if (m_statsPending)
    {
        ClientScoreboard* scoreboard = GetScoreboard();
        if (!m_scoreboardLayout->showLifeSpanProgress())
        {
            int progress = 0;
            std::wstring details = m_scoreboardLayout->GetCustomProgressDetails(m_selectedUid, progress);
            wchar_t format[64];
            swprintf(format, 64, L"<font color=\"#%08x\"><p align=center>",
                     gApp.GetHTMLColour(eMinecraftColour_HTMLColor_White));
            {
                std::wstring html = std::wstring(format).append(details);
                details = html + L"</p>";
            }
            m_killBar.setLabel(UIString(details), false, false);
            m_killBar.setProgress(progress);
        }
        else
        {
            int rounds = Minecraft::GetInstance()->GetMiniGame()->GetRoundCount();
            unsigned int totalRoundLength = 0;
            unsigned int totalLifeSpan = 0;
            unsigned int roundsSurvived = 0;
            for (int round = 0; round < rounds; ++round)
            {
                totalRoundLength += scoreboard->GetRoundLength(round);
                totalLifeSpan += scoreboard->GetLifeSpan(m_selectedUid, round);
                if (scoreboard->GetSurvived(m_selectedUid, round))
                    ++roundsSurvived;
            }
            int progress = 100;
            if (totalRoundLength != 0)
                progress = static_cast<int>(static_cast<float>(totalLifeSpan) / static_cast<float>(totalRoundLength)
                                            * 100.0f);
            std::wstring details = L10N::GetString(0x6f5446df);
            details += _toString(progress);
            details += L"%";
            wchar_t format[64];
            swprintf(format, 64, L"<font color=\"#%08x\"><p align=center>",
                     gApp.GetHTMLColour(eMinecraftColour_HTMLColor_White));
            {
                std::wstring html = std::wstring(format).append(details);
                details = html + L"</p>";
            }
            m_killBar.setLabel(UIString(details), false, false);
            m_killBar.setProgress(progress);
            const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
            if (!miniGame->HasMultipleRounds() && miniGame->GetLivesPerRound() == 1)
            {
                std::wstring killer = scoreboard->GetKiller(m_selectedUid, -1, -1);
                SetHasPlayerBeenKilled(!scoreboard->GetSurvived(m_selectedUid, -1) && !killer.empty());
            }
            else
            {
                SetHasPlayerBeenKilled(roundsSurvived != rounds);
            }
        }
        GameMedal* medal = scoreboard->GetMedal(m_selectedUid);
        int icon;
        if (medal == 0 || (icon = medal->GetId() + 0xc) < 0xc)
        {
            SetStatIconTextureName(0, std::wstring(IScoreboardLayout::getIcon(0x6e)));
            m_statsPending = false;
        }
        else
        {
            SetStatIconTextureName(0, std::wstring(IScoreboardLayout::getIcon(icon)));
            m_statsPending = false;
        }
    }
}

bool UIScene_GameModeScoreboardMenu::CanKickPlayer(INetworkPlayer* player)
{
    if (player == 0)
        return false;
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[m_playerId];
    bool permitted = m_localPlayerCanModerate != 0 || localPlayer->isModerator();
    bool protectedPlayer = player->IsLocal() || player->IsSameSystem(CGameNetworkManager::sInstance.GetHostPlayer());
    if (m_canKickPlayers != 0)
        return permitted && !protectedPlayer;
    if (m_canVoteToKick != 0)
    {
        unsigned char smallId = player->GetSmallId();
        int kickVote = Minecraft::GetInstance()->GetClientMasterGameMode()->GetKickVote(m_playerId, smallId);
        return !protectedPlayer && kickVote == 0;
    }
    return false;
}

void UIScene_GameModeScoreboardMenu::updateTooltips()
{
    int partyTooltip = -1;
    int viewTooltip = -1;
    int kickTooltip = -1;
    _XPARTY_USER_LIST users;
    if (XPartyGetUserList(&users) != 0 && users.count > 1)
        partyTooltip = static_cast<int>(IDS_TOOLTIPS_INVITE_PARTY);
    if (CGameNetworkManager::sInstance.IsLocalGame())
        partyTooltip = -1;

    int selected = static_cast<int>(m_playerList.getCurrentSelection());
    if (selected >= 0 && selected < m_scoreboardLayout->GetPlayerCount()
        && m_scoreboardLayout->doesExist(static_cast<unsigned int>(selected)))
    {
        unsigned int index = static_cast<unsigned int>(selected);
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(
            m_scoreboardLayout->getSmallIDOfPlayerAt(index));
        if (CanKickPlayer(player))
            kickTooltip = static_cast<int>(IDS_TOOLTIPS_KICK);
        if (player != 0 && player->IsLocal() && player->GetUserIndex() == m_playerId)
            viewTooltip = static_cast<int>(IDS_TOOLTIPS_VIEW_GAMERPROFILE);
        else
            viewTooltip = static_cast<int>(IDS_TOOLTIPS_VIEW_GAMERCARD);
    }
    gConsoleUIController.SetTooltips(m_playerId, kickTooltip, IDS_TOOLTIPS_BACK, partyTooltip, viewTooltip, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_GameModeScoreboardMenu::handleGainFocus(bool hadFocus)
{
    if (hadFocus)
        CGameNetworkManager::sInstance.RegisterPlayerChangedCallback(m_playerId, IUIScene_PlayerList::OnPlayerChanged,
                                                                     this);
}

UIControl* UIScene_GameModeScoreboardMenu::GetMainPanel()
{
    return &m_mainPanel;
}

void UIScene_GameModeScoreboardMenu::customDraw(const char* name, fuiRect* rect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mLocalPlayers[m_playerId] == NULL || minecraft->mGameModes[m_playerId] == NULL)
        return;
    wchar_t slotName[64];
    mbstowcs(slotName, name, 64);
    IScoreboardLayout* layout = m_scoreboardLayout;
    not_null_ptr<ItemInstance> item = layout->getItemForSlot(slotName, m_playerList.getCurrentSelection(),
                                                             m_scoreboardType);
    if (!item->isEmpty())
        customDrawSlotControlFui(rect, m_playerId, item, 1.0f, item->getItem()->isFoil(item), false, false);
}

void UIScene_GameModeScoreboardMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
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
        int selected = static_cast<int>(m_playerList.getCurrentSelection());
        if (pressed && m_playerList.hasFocus() && m_playerList.getItemCount() > 0 && selected >= 0)
        {
            if (selected < m_scoreboardLayout->GetPlayerCount())
            {
                INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(
                    m_scoreboardLayout->getSmallIDOfPlayerAt(static_cast<unsigned int>(selected)));
                if (player != 0)
                    player->GetUID() != PlayerUID(0);
            }
        }
        break;
    }
    case EControllerActions_MenuX:
        if (pressed && !repeat)
            CGameNetworkManager::sInstance.IsLocalGame();
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}

void UIScene_GameModeScoreboardMenu::handleDestroy()
{
    CGameNetworkManager::sInstance.UnRegisterPlayerChangedCallback(m_playerId, IUIScene_PlayerList::OnPlayerChanged,
                                                                   this);
    m_layer->showComponent(m_playerId, EUIScene_MenuBackground, false);
}

void UIScene_GameModeScoreboardMenu::setScoreboardType(int type)
{
    static_cast<FJ_GameModeScoreboard*>(m_movie->getRootNode()->getFuiNode())->SetScoreboardType(type);
}

void UIScene_GameModeScoreboardMenu::initNumColumns(int columns)
{
    static_cast<FJ_GameModeScoreboard*>(m_movie->getRootNode()->getFuiNode())->InitNumColumns(columns);
}

void UIScene_GameModeScoreboardMenu::handleReload()
{
    for (int index = 0; index < 119; ++index)
        m_registeredTextures[index] = 0;
    m_scoreboardLayout->ClearPlayersInfo();
    m_playerList.init(0);
    setScoreboardType(m_scoreboardType);
    registerTexture(0);
    registerTexture(1);
    registerTexture(0x6e);
    registerTexture(4);
    registerTexture(6);
    registerTexture(7);
    registerTexture(5);
    registerTexture(0x73);
    registerTexture(0x6f);
    m_columnCount = static_cast<int>(m_scoreboardLayout->getNumberOfColumns());
    initNumColumns(m_columnCount);
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    m_initialisePending = 0;
    if (player == 0)
        m_initialisePending = 1;
    else
        InitialiseScoreboard(player);
}

void UIScene_GameModeScoreboardMenu::handlePress(int controlId, int childId)
{
    if (controlId == 0 && (m_canKickPlayers || m_canVoteToKick) && childId >= 0
        && childId < m_scoreboardLayout->GetPlayerCount())
    {
        if (m_scoreboardLayout->doesExist(static_cast<unsigned int>(childId)))
        {
            INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(
                m_scoreboardLayout->getSmallIDOfPlayerAt(static_cast<unsigned int>(childId)));
            if (CanKickPlayer(player))
            {
                unsigned char* smallId = new unsigned char(0);
                *smallId = m_scoreboardLayout->getSmallIDOfPlayerAt(static_cast<unsigned int>(childId));
                int parameters[2] = {static_cast<int>(IDS_CONFIRM_OK), static_cast<int>(IDS_CONFIRM_CANCEL)};
                int message = IDS_UNLOCK_KICK_PLAYER;
                if (Minecraft::InMiniGame(eMiniGameId_Any, true))
                    message = IDS_UNLOCK_KICK_PLAYER_MINIGAMES;
                gConsoleUIController.RequestAlertMessage(IDS_UNLOCK_KICK_PLAYER_TITLE, message, parameters, 2,
                                                         m_playerId, IUIScene_PlayerList::KickPlayerReturned, smallId,
                                                         0, true);
            }
        }
    }
}

void UIScene_GameModeScoreboardMenu::handleFocusChange(int focus, int childFocus)
{
    if (focus == 0)
        m_playerList.updateChildFocus(childFocus);
    updateTooltips();
    if (m_scoreboardType == EScoreboardType_Award && m_scoreboardLayout->GetPlayerCount() > 0)
    {
        int selected = static_cast<int>(m_playerList.getCurrentSelection());
        if (selected >= 0 && selected < m_scoreboardLayout->GetPlayerCount()
            && m_scoreboardLayout->doesExist(static_cast<unsigned int>(selected)))
        {
            INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(
                m_scoreboardLayout->getSmallIDOfPlayerAt(static_cast<unsigned int>(selected)));
            if (player != 0)
                UpdateStats(StatsUID::NETWORKPLAYER_SUID(player));
        }
    }
}

void UIScene_GameModeScoreboardMenu::CheckReloadMovie()
{
    if (m_scoreboardLayout != 0 && m_columnCount != static_cast<int>(m_scoreboardLayout->getNumberOfColumns()))
        reloadMovie(false);
}

void UIScene_GameModeScoreboardMenu::BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info)
{
    m_scoreboardLayout->BuildPlayerInfo(player, info);
}

PlayerInfo* UIScene_GameModeScoreboardMenu::CreatePlayerInfo()
{
    return m_scoreboardLayout->CreatePlayerInfo();
}

int UIScene_GameModeScoreboardMenu::GetPlayerCount()
{
    return m_scoreboardLayout->GetPlayerCount();
}

PlayerInfo* UIScene_GameModeScoreboardMenu::GetPlayer(int index)
{
    return m_scoreboardLayout->GetPlayer(index);
}

bool UIScene_GameModeScoreboardMenu::RemovePlayer(unsigned char smallId)
{
    int index = FindPlayer(smallId);
    return m_scoreboardLayout->RemovePlayer(index);
}

void UIScene_GameModeScoreboardMenu::AddPlayer(unsigned char smallId)
{
    m_scoreboardLayout->AddPlayer(smallId);
}

UIControl_ButtonList* UIScene_GameModeScoreboardMenu::GetPlayerList()
{
    return &m_playerList;
}

void UIScene_GameModeScoreboardMenu::UpdatePlayer(int index, PlayerInfo* oldInfo, PlayerInfo* newInfo)
{
    IUIScene_PlayerList::UpdatePlayer(index, oldInfo, newInfo);
    if (hasAPlayerChanged())
    {
        if (m_scoreboardType == EScoreboardType_Scoreboard)
        {
            m_playerList.SetSlotTextureName(
                index, m_columnCount - 1,
                std::wstring(IScoreboardLayout::getIcon(m_scoreboardLayout->GetEndBarIcon(newInfo))), true);
            INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(newInfo->smallId);
            if (player != 0)
            {
                StatsUID uid = StatsUID::NETWORKPLAYER_SUID(player);
                m_playerList.setSlotsText(index, m_scoreboardLayout->GetRowLabel(0, uid, m_scoreboardType),
                                          m_scoreboardLayout->GetRowLabel(1, uid, m_scoreboardType),
                                          m_scoreboardLayout->GetRowLabel(2, uid, m_scoreboardType),
                                          m_scoreboardLayout->GetRowLabel(3, uid, m_scoreboardType),
                                          m_scoreboardLayout->GetRowLabel(4, uid, m_scoreboardType),
                                          m_scoreboardLayout->GetRowLabel(5, uid, m_scoreboardType),
                                          m_scoreboardLayout->GetRowLabel(6, uid, m_scoreboardType), true);
            }
            else
            {
                m_playerList.setSlotsText(index, std::wstring(L""), std::wstring(L""), std::wstring(L""),
                                          std::wstring(L""), std::wstring(L""), std::wstring(L""), std::wstring(L""),
                                          true);
            }
        }
        else
        {
            ClientScoreboard* scoreboard =
                Minecraft::GetInstance()->mLocalPlayers[m_playerId]->GetLevel()->GetClientScoreboard();
            std::wstring iconName(L"");
            INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(newInfo->smallId);
            if (player != 0)
            {
                GameMedal* medal = scoreboard->GetMedal(StatsUID::NETWORKPLAYER_SUID(player));
                int medalIcon;
                if (medal != 0 && (medalIcon = medal->GetId() + 0xc) >= 0xc)
                {
                    registerTexture(medalIcon);
                    iconName = IScoreboardLayout::getIcon(medalIcon);
                }
                else
                {
                    iconName = IScoreboardLayout::getIcon(0x6e);
                }
                m_playerList.SetSlotTextureName(index, m_columnCount - 1, iconName, true);
                m_playerList.setSlotsText(
                    index, m_scoreboardLayout->GetRowLabel(0, StatsUID::NETWORKPLAYER_SUID(player), m_scoreboardType),
                    m_scoreboardLayout->GetRowLabel(1, StatsUID::NETWORKPLAYER_SUID(player), m_scoreboardType),
                    m_scoreboardLayout->GetRowLabel(2, StatsUID::NETWORKPLAYER_SUID(player), m_scoreboardType),
                    m_scoreboardLayout->GetRowLabel(3, StatsUID::NETWORKPLAYER_SUID(player), m_scoreboardType),
                    m_scoreboardLayout->GetRowLabel(4, StatsUID::NETWORKPLAYER_SUID(player), m_scoreboardType),
                    m_scoreboardLayout->GetRowLabel(5, StatsUID::NETWORKPLAYER_SUID(player), m_scoreboardType),
                    m_scoreboardLayout->GetRowLabel(6, StatsUID::NETWORKPLAYER_SUID(player), m_scoreboardType), true);
            }
        }
    }
    if (newInfo->colour != oldInfo->colour || hasAPlayerChanged())
    {
        oldInfo->colour = newInfo->colour;
        m_playerList.setPlayerIcon(index, newInfo->colour);
    }
    if (newInfo->voiceStatus != oldInfo->voiceStatus || hasAPlayerChanged())
    {
        oldInfo->voiceStatus = newInfo->voiceStatus;
        m_playerList.setVOIPIcon(index, newInfo->voiceStatus);
    }
    m_scoreboardLayout->UpdatePlayer(index, oldInfo, newInfo, hasAPlayerChanged());
}

int UIScene_GameModeScoreboardMenu::getSceneType()
{
    return EUIScene_GameModeScoreboardMenu;
}

int UIScene_GameModeScoreboardMenu::GetPad()
{
    return m_playerId;
}

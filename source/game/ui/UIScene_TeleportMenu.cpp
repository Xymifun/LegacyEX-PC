#include "game/ui/UIScene_TeleportMenu.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcDedicated.hpp"
#endif
#include "game/core/StringIds.hpp"

#include <string>

#include "game/command/TeleportCommand.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/core/GameSettings.hpp"

UIScene_TeleportMenu::UIScene_TeleportMenu(int sceneType, void* data, UILayer* layer) : UIScene(sceneType, layer)
{
    initialiseMovie();
    m_splitScreen = static_cast<bool*>(data)[4];
    delete static_cast<bool*>(data);

    if (m_splitScreen)
        m_title.init(UIString(L10N::GetString(IDS_TELEPORT_TO_PLAYER)));
    else
        m_title.init(UIString(L10N::GetString(IDS_TELEPORT_TO_ME)));
    m_playerList.init(0);

    m_playerInfo = std::vector<_PlayerInfo*>();
    INetworkPlayer* localPlayer = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    unsigned int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
    for (unsigned int i = 0; i < playerCount; ++i)
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerByIndex(i);
#ifdef MINECRAFT_PC
        if (PcIsHiddenHost(player))
            continue;
#endif
        if (player != 0 && player != localPlayer)
        {
            _PlayerInfo* info = BuildPlayerInfo(player);
            m_playerInfo.push_back(info);
            m_playerList.addItem(info->name, info->colour, info->voiceStatus, true);
        }
    }
    CGameNetworkManager::sInstance.RegisterPlayerChangedCallback(m_playerId, OnPlayerChanged, this);
    layer->addComponent(sceneType, EUIScene_MenuBackground, 0);
    updateTooltips();
    gConsoleUIController.HidePressStart();
}

UIScene_TeleportMenu::~UIScene_TeleportMenu()
{
    for (unsigned int i = 0; i < m_playerInfo.size(); ++i)
        delete m_playerInfo[i];
}

unsigned int UIScene_TeleportMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    {
        m_playerList.setupControl(this, root, "GamePlayers");
        m_controls.push_back(&m_playerList);
    }
    {
        m_title.setupControl(this, root, "Title");
        m_controls.push_back(&m_title);
    }
    return 1;
}

UIScene_TeleportMenu::_PlayerInfo* UIScene_TeleportMenu::BuildPlayerInfo(INetworkPlayer* player)
{
    _PlayerInfo* info = new _PlayerInfo();
    info->smallId = player->GetSmallId();
    std::wstring name(L"");
    name = player->GetDisplayName();
    char voiceStatus = 0;
    if (player->HasVoice())
    {
        if (player->IsMutedByLocalUser(m_playerId))
        {
            voiceStatus = 3;
        }
        else
        {
            voiceStatus = 1;
            if (player->IsTalking())
                voiceStatus = 2;
        }
    }
    info->voiceStatus = voiceStatus;
    info->colour = static_cast<short>(gApp.GetPlayerColour(info->smallId));
    info->name = name;
    return info;
}

void UIScene_TeleportMenu::OnPlayerChanged(void* context, INetworkPlayer* player, bool removed)
{
    bool found = false;
    unsigned int foundIndex = 0;
    UIScene_TeleportMenu* scene = static_cast<UIScene_TeleportMenu*>(context);
    for (unsigned int i = 0; i < scene->m_playerInfo.size(); ++i)
    {
        if (!found)
        {
            if (scene->m_playerInfo[i]->smallId == player->GetSmallId())
            {
                if (scene->m_playerList.getCurrentSelection() == scene->m_playerList.getItemCount() - 1)
                    scene->m_playerList.setCurrentSelection(scene->m_playerList.getItemCount() - 2, false, true);
                found = true;
                foundIndex = i;
            }
        }
    }
    if (found && scene->m_playerList.getItemCount() > 0)
    {
        delete scene->m_playerInfo[foundIndex];
        scene->m_playerInfo.erase(scene->m_playerInfo.begin() + foundIndex);
        scene->m_playerList.removeItem(foundIndex, true);
    }
    if (removed)
        return;
    _PlayerInfo* info = scene->BuildPlayerInfo(player);
    scene->m_playerInfo.push_back(info);
    scene->m_playerList.addItem(info->name, info->colour, info->voiceStatus, true);
}

void UIScene_TeleportMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_TeleportMenu::handleReload()
{
    unsigned int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
    for (unsigned int i = 0; i < m_playerInfo.size(); ++i)
        delete m_playerInfo[i];
    m_playerInfo.clear();
    INetworkPlayer* localPlayer = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    for (unsigned int i = 0; i < playerCount; ++i)
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerByIndex(i);
#ifdef MINECRAFT_PC
        if (PcIsHiddenHost(player))
            continue;
#endif
        if (player != 0 && player != localPlayer)
        {
            _PlayerInfo* info = BuildPlayerInfo(player);
            m_playerInfo.push_back(info);
            m_playerList.addItem(info->name, info->colour, info->voiceStatus, true);
        }
    }
    if (controlHasFocus(0))
        m_playerList.setCurrentSelection(getControlChildFocus(), false, true);
}

void UIScene_TeleportMenu::tick()
{
    UIScene::tick();
    INetworkPlayer* localPlayer = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    unsigned int item = 0;
#ifdef MINECRAFT_PC
    const int playerCount = static_cast<int>(CGameNetworkManager::sInstance.GetPlayerCount());
    for (int playerIndex = 0; item < m_playerInfo.size() && playerIndex < playerCount; ++playerIndex)
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerByIndex(playerIndex);
        if (player == 0 || player == localPlayer || PcIsHiddenHost(player))
            continue;
        const unsigned int current = item++;
        _PlayerInfo* info = BuildPlayerInfo(player);
        m_playerInfo[current]->smallId = info->smallId;
        if (info->colour != m_playerInfo[current]->colour)
        {
            m_playerInfo[current]->colour = info->colour;
            m_playerList.setPlayerIcon(current, info->colour);
        }
        if (info->name.compare(m_playerInfo[current]->name) != 0)
        {
            m_playerList.setButtonLabel(current, info->name);
            m_playerInfo[current]->name = info->name;
        }
        delete info;
    }
#else
    for (int playerIndex = 0; item < m_playerInfo.size(); ++playerIndex, ++item)
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerByIndex(playerIndex);
        if (player == localPlayer)
            ++playerIndex;
        player = CGameNetworkManager::sInstance.GetPlayerByIndex(playerIndex);
        if (player == 0)
            continue;
        _PlayerInfo* info = BuildPlayerInfo(player);
        m_playerInfo[item]->smallId = info->smallId;
        if (info->colour != m_playerInfo[item]->colour)
        {
            m_playerInfo[item]->colour = info->colour;
            m_playerList.setPlayerIcon(item, info->colour);
        }
        if (info->name.compare(m_playerInfo[item]->name) != 0)
        {
            m_playerList.setButtonLabel(item, info->name);
            m_playerInfo[item]->name = info->name;
        }
        delete info;
    }
#endif
}

std::wstring UIScene_TeleportMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() > 1 ? std::wstring(L"InGameTeleportMenuSplit")
                                          : std::wstring(L"InGameTeleportMenu");
}

void UIScene_TeleportMenu::handleInput(int, int action, bool repeat, bool pressed, bool released, bool& handled)
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
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}

void UIScene_TeleportMenu::handleGainFocus(bool hadFocus)
{
    UIScene::handleGainFocus(hadFocus);
    if (hadFocus)
        CGameNetworkManager::sInstance.RegisterPlayerChangedCallback(m_playerId, OnPlayerChanged, this);
}

void UIScene_TeleportMenu::handlePress(int controlId, int itemIndex)
{
    if (controlId != 0)
        return;
    _PlayerInfo* info = m_playerInfo[itemIndex];
    INetworkPlayer* target = CGameNetworkManager::sInstance.GetPlayerBySmallId(info->smallId);
    INetworkPlayer* local = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    boost::shared_ptr<GameCommandPacket> packet;
    if (m_splitScreen)
        packet = TeleportCommand::preparePacket(local->GetUID(), target->GetUID());
    else
        packet = TeleportCommand::preparePacket(target->GetUID(), local->GetUID());
    ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
    connection->send(packet);
}

void UIScene_TeleportMenu::handleDestroy()
{
    CGameNetworkManager::sInstance.UnRegisterPlayerChangedCallback(m_playerId, OnPlayerChanged, this);
    m_layer->showComponent(m_playerId, EUIScene_MenuBackground, false);
}

int UIScene_TeleportMenu::getSceneType()
{
    return EUIScene_TeleportMenu;
}

unsigned int UIScene_TeleportMenu::getSubSceneType() const
{
    return m_splitScreen ? 1 : 0;
}

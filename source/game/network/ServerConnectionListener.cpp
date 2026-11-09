#include "game/network/ServerConnectionListener.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/network/Connection.hpp"
#include "game/network/ClientboundLoginPacket.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientboundPreLoginPacket.hpp"
#include "game/network/ServerboundPreLoginPacket.hpp"
#include "game/network/INetworkPlayer.hpp"
#ifdef MINECRAFT_PC
#include "game/core/Socket.hpp"
#include "platform/pc/PcNetworkSession.hpp"
#endif
#include "game/world/LevelData.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"
#include <cstring>
#include <vector>

ServerConnectionListener::ServerConnectionListener(MinecraftServer* server, Socket* socket, const std::wstring& name)
    : PacketListener()
{
    m_disconnected = false;
    m_tickCount = 0;
    m_name = L"";
    m_acceptedLogin = boost::shared_ptr<ClientboundLoginPacket>();
    m_loginKey = L"";
    m_server = server;
    m_connection = new Connection(socket, name, this);
    m_connection->m_fakeLag = 0;
}

ServerConnectionListener::~ServerConnectionListener()
{
    delete m_connection;
}

void ServerConnectionListener::disconnect(DisconnectPacket::eDisconnectReason reason)
{
    m_connection->send(boost::shared_ptr<DisconnectPacket>(new DisconnectPacket(reason)));
    m_connection->sendAndQuit();
    m_disconnected = true;
}

void ServerConnectionListener::tick()
{
    if (m_acceptedLogin != NULL)
    {
        handleAcceptedLogin(m_acceptedLogin);
        m_acceptedLogin = boost::shared_ptr<ClientboundLoginPacket>();
    }

    if (m_tickCount++ == 600)
    {
        disconnect(DisconnectPacket::eDisconnectReason_LoginTooLong);
    }
    else
    {
        m_connection->tick();
    }
}

void ServerConnectionListener::sendPreLoginResponse()
{
    PlayerUID* playerUids = new PlayerUID[8];
    unsigned int playerCount = 0;
    unsigned int hostPlayerIndex = 0;
    unsigned int guestFlags = 0;
    char uniqueMapName[16];
    std::memset(uniqueMapName, 0, 14);
    gStorage->GetSaveUniqueFilename(uniqueMapName);

    std::vector<boost::shared_ptr<ServerPlayer> >* players = gMinecraftServer->getPlayers()->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players->begin(); it != players->end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> serverPlayer = *it;
        if (serverPlayer != NULL && serverPlayer->mConnection->mPlayerUID != PlayerUID(0)
            && serverPlayer->mConnection->mOldPlayerUID != PlayerUID(0))
        {
            if (serverPlayer->mConnection->mFriendsOnly)
                guestFlags = guestFlags | ((1 << playerCount) & 0xFF);
            playerUids[playerCount] = serverPlayer->mConnection->mOldPlayerUID;
            if (serverPlayer->mConnection->getNetworkPlayer() != NULL)
            {
                if (serverPlayer->mConnection->getNetworkPlayer()->IsHost())
                    hostPlayerIndex = playerCount;
            }
            ++playerCount;
        }
    }

    std::wstring accountName(L"-");
    accountName = convStringToWstring(std::string(gProfile->GetNetworkAccountID()));

    m_connection->send(boost::shared_ptr<ClientboundPreLoginPacket>(new ClientboundPreLoginPacket(
        accountName, playerUids, playerCount, guestFlags, m_server->ugcPlayersVersion, uniqueMapName,
        gApp.GetGameHostOption(eGameHostOption_All),
        Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_All), hostPlayerIndex,
        m_server->getGameModeId(), CGameNetworkManager::sInstance.IsMatchmakingEnabled(),
        Minecraft::GetInstance()->GetMiniGame()->GetId())));
}

void ServerConnectionListener::handlePreLogin(boost::shared_ptr<ServerboundPreLoginPacket> packet)
{
    if (packet->protocolVersion != SharedConstants::NETWORK_PROTOCOL_VERSION)
    {
        if (packet->protocolVersion > SharedConstants::NETWORK_PROTOCOL_VERSION)
        {
            disconnect(DisconnectPacket::eDisconnectReason_HostOutOfDate);
            return;
        }
        disconnect(DisconnectPacket::eDisconnectReason_ClientOutOfDate);
        return;
    }

    m_name = packet->name;
    sendPreLoginResponse();
}

void ServerConnectionListener::handleLogin(boost::shared_ptr<ClientboundLoginPacket> packet)
{
    if (packet->mEntityId != 316)
    {
        if (packet->mEntityId > 316)
        {
            disconnect(DisconnectPacket::eDisconnectReason_HostOutOfDate);
            return;
        }
        disconnect(DisconnectPacket::eDisconnectReason_ClientOutOfDate);
        return;
    }

    if (m_server->getPlayers()->isXuidBanned(packet->mPlayerUID2))
    {
        disconnect(DisconnectPacket::eDisconnectReason_BannedByHost);
        return;
    }

    handleAcceptedLogin(packet);
}

void ServerConnectionListener::handleAcceptedLogin(boost::shared_ptr<ClientboundLoginPacket> packet)
{
    if (packet->mViewDistance != m_server->ugcPlayersVersion)
    {
        sendPreLoginResponse();
        return;
    }

    PlayerUID uid = packet->mPlayerUID;
    if (uid == PlayerUID(0))
        uid = packet->mPlayerUID2;

#ifdef MINECRAFT_PC
    {
        INetworkPlayer* networkPlayer = m_connection->getSocket() != 0 ? m_connection->getSocket()->getPlayer() : 0;
        PlayerUID joined;
        if (networkPlayer != 0 && !networkPlayer->IsLocal()
            && PcNetworkSession::VerifiedJoinUid(networkPlayer->GetSmallId(), &joined) && !(joined == uid))
        {
            printf("[pc] login: a verified guest logged in as a different player; refused\n");
            fflush(stdout);
            disconnect(DisconnectPacket::eDisconnectReason_NoUserContentPrivilege);
            return;
        }
    }
#endif

    boost::shared_ptr<ServerPlayer> player = m_server->getPlayers()->getPlayerForLogin(this, m_name, uid,
                                                                                       packet->mPlayerUID2);
    if (player)
    {
        player->setMainArm(packet->mHasAchievements ? HumanoidArm::LEFT : HumanoidArm::RIGHT);
        m_server->getPlayers()->placeNewPlayer(m_connection, player, packet);
        m_connection = 0;
    }
    if (packet->mDifficulty != -1)
        gApp.SetFirstMiniGameType(static_cast<EMiniGameId>(packet->mDifficulty));
    m_disconnected = true;
}

void ServerConnectionListener::onDisconnect(DisconnectPacket::eDisconnectReason, void*)
{
    m_disconnected = true;
}

void ServerConnectionListener::handleGetInfo(boost::shared_ptr<GetInfoPacket>)
{
    m_connection->send(
        boost::shared_ptr<DisconnectPacket>(new DisconnectPacket(DisconnectPacket::eDisconnectReason_GameFull)));
    m_connection->sendAndQuit();
    m_disconnected = true;
}

void ServerConnectionListener::handleKeepAlive(boost::shared_ptr<ServerboundKeepAlivePacket>)
{
}

void ServerConnectionListener::onUnhandledPacket(boost::shared_ptr<Packet>)
{
    disconnect(DisconnectPacket::eDisconnectReason_UnexpectedPacket);
}

void ServerConnectionListener::send(boost::shared_ptr<Packet> packet)
{
    m_connection->send(packet);
}

bool ServerConnectionListener::isServerPacketListener()
{
    return true;
}

bool ServerConnectionListener::isDisconnected()
{
    return m_disconnected;
}

std::wstring ServerConnectionListener::getName()
{
    return m_name;
}

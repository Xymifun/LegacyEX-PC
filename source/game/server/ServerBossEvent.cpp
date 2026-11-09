#include "game/server/ServerBossEvent.hpp"

#include <map>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/Mth.hpp"
#include "game/network/ClientboundBossEventPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerPlayer.hpp"

ServerBossEvent::ServerBossEvent(int nameId, const std::wstring& name, BossBarColor color, BossBarOverlay overlay)
    : BossEvent(Mth::createInsecureUUID(), nameId, name, color, overlay),
      visible(true)
{
}

void ServerBossEvent::broadcast(ClientboundBossEventPacket::Operation operation)
{
    if (!visible)
    {
        return;
    }

    boost::shared_ptr<Packet> packet(new ClientboundBossEventPacket(operation, this));
    for (std::map<boost::shared_ptr<ServerPlayer>, int>::iterator it = players.begin(); it != players.end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> player = it->first;
        if (player->mConnection != 0)
        {
            player->mConnection->send(packet);
        }
    }
}

void ServerBossEvent::setPercent(float percent)
{
    if (percent != this->percent)
    {
        BossEvent::setPercent(percent);
        broadcast(ClientboundBossEventPacket::UPDATE_PROGRESS);
    }
}

void ServerBossEvent::setColor(BossBarColor color)
{
    if (color != this->color)
    {
        BossEvent::setColor(color);
        broadcast(ClientboundBossEventPacket::UPDATE_STYLE);
    }
}

void ServerBossEvent::setOverlay(BossBarOverlay overlay)
{
    if (overlay != this->overlay)
    {
        BossEvent::setOverlay(overlay);
        broadcast(ClientboundBossEventPacket::UPDATE_STYLE);
    }
}

BossEvent* ServerBossEvent::setDarkenScreen(bool darkenScreen)
{
    if (darkenScreen != this->darkenScreen)
    {
        BossEvent::setDarkenScreen(darkenScreen);
        broadcast(ClientboundBossEventPacket::UPDATE_PROPERTIES);
    }
    return this;
}

BossEvent* ServerBossEvent::setPlayBossMusic(bool playBossMusic)
{
    if (playBossMusic != this->playBossMusic)
    {
        BossEvent::setPlayBossMusic(playBossMusic);
        broadcast(ClientboundBossEventPacket::UPDATE_PROPERTIES);
    }
    return this;
}

BossEvent* ServerBossEvent::setCreateWorldFog(bool createWorldFog)
{
    if (createWorldFog != this->createWorldFog)
    {
        BossEvent::setCreateWorldFog(createWorldFog);
        broadcast(ClientboundBossEventPacket::UPDATE_PROPERTIES);
    }
    return this;
}

void ServerBossEvent::setName(const std::wstring& name)
{
    if (name.compare(this->name) != 0)
    {
        BossEvent::setName(name);
        broadcast(ClientboundBossEventPacket::UPDATE_NAME);
    }
}

void ServerBossEvent::addPlayer(boost::shared_ptr<ServerPlayer> player, int party)
{
    bool existed = players.count(player) != 0;
    players[player] = party;
    if (!existed && visible && player->mConnection != 0)
    {
        ServerGamePacketListenerImpl* connection = player->mConnection.operator->();
        boost::shared_ptr<Packet> packet(new ClientboundBossEventPacket(ClientboundBossEventPacket::ADD, this));
        connection->send(packet);
    }
}

void ServerBossEvent::removePlayer(boost::shared_ptr<ServerPlayer> player)
{
    if (players.count(player) != 0)
    {
        players.erase(player);
        if (visible && player->mConnection != 0)
        {
            ServerGamePacketListenerImpl* connection = player->mConnection.operator->();
            boost::shared_ptr<Packet> packet(new ClientboundBossEventPacket(ClientboundBossEventPacket::REMOVE, this));
            connection->send(packet);
        }
    }
}

void ServerBossEvent::removeAllPlayers()
{
    while (players.size() != 0)
    {
        boost::shared_ptr<ServerPlayer> player = players.begin()->first;
        removePlayer(player);
    }
}

void ServerBossEvent::setVisible(bool visible)
{
    if (visible != this->visible)
    {
        this->visible = visible;
        for (std::map<boost::shared_ptr<ServerPlayer>, int>::iterator it = players.begin(); it != players.end(); ++it)
        {
            boost::shared_ptr<ServerPlayer> player = it->first;
            if (player->mConnection != 0)
            {
                ServerGamePacketListenerImpl* connection = player->mConnection.operator->();
                boost::shared_ptr<ClientboundBossEventPacket> packet(new ClientboundBossEventPacket(
                    visible ? ClientboundBossEventPacket::ADD : ClientboundBossEventPacket::REMOVE, this));
                connection->send(packet);
            }
        }
    }
}

std::vector<boost::shared_ptr<ServerPlayer> >* ServerBossEvent::getPlayers()
{
    std::vector<boost::shared_ptr<ServerPlayer> >* result = new std::vector<boost::shared_ptr<ServerPlayer> >();
    for (std::map<boost::shared_ptr<ServerPlayer>, int>::iterator it = players.begin(); it != players.end(); ++it)
    {
        result->push_back(it->first);
    }
    return result;
}

std::map<boost::shared_ptr<ServerPlayer>, int>& ServerBossEvent::getPartyPlayers()
{
    return players;
}

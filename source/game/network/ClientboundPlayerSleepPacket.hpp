#ifndef GAME_NETWORK_CLIENTBOUNDPLAYERSLEEPPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDPLAYERSLEEPPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/block/BlockPos.hpp"

class DataInputStream;
class DataOutputStream;
class Level;
class PacketListener;
class Player;

class ClientboundPlayerSleepPacket : public Packet, public boost::enable_shared_from_this<ClientboundPlayerSleepPacket>
{
public:
    ClientboundPlayerSleepPacket();
    ClientboundPlayerSleepPacket(boost::shared_ptr<Player> player, const BlockPos& pos);
    virtual ~ClientboundPlayerSleepPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    boost::shared_ptr<Player> getPlayer(Level* level);
    int getPlayerId();
    BlockPos getPos();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundPlayerSleepPacket()); }

private:
    int playerId;
    BlockPos pos;
};

#endif

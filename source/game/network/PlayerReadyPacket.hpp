#ifndef GAME_NETWORK_PLAYERREADYPACKET_HPP
#define GAME_NETWORK_PLAYERREADYPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;
class ServerGamePacketListenerImpl;
class ClientPacketListener;

class PlayerReadyPacket : public Packet, public boost::enable_shared_from_this<PlayerReadyPacket>
{
public:
    PlayerReadyPacket();
    PlayerReadyPacket(unsigned int playerId, bool ready);
    virtual ~PlayerReadyPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new PlayerReadyPacket()); }

private:
    friend class ServerGamePacketListenerImpl;
    friend class ClientPacketListener;

    unsigned int mPlayerId;
    bool mReady;
};

#endif

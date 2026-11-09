#ifndef GAME_NETWORK_CLIENTBOUNDCOOLDOWNPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDCOOLDOWNPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class Item;
class PacketListener;

class ClientboundCooldownPacket : public Packet, public boost::enable_shared_from_this<ClientboundCooldownPacket>
{
public:
    ClientboundCooldownPacket();
    ClientboundCooldownPacket(Item* item, int duration);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    Item* getItem();
    int getDuration();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundCooldownPacket()); }

private:
    Item* item;
    int duration;
};

#endif

#ifndef GAME_NETWORK_CLIENTBOUNDDAMAGEINDICATORPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDDAMAGEINDICATORPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundDamageIndicatorPacket : public Packet,
                                         public boost::enable_shared_from_this<ClientboundDamageIndicatorPacket>
{
public:
    ClientboundDamageIndicatorPacket();
    ClientboundDamageIndicatorPacket(float x, float z);
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    double getX();
    double getZ();
    unsigned char isAllDirections();
    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundDamageIndicatorPacket());
    }

private:
    float x;
    float z;
    bool allDirections;
};

#endif

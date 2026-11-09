#ifndef GAME_NETWORK_CLIENTBOUNDPOWERUPPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDPOWERUPPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundPowerupPacket : public Packet, public boost::enable_shared_from_this<ClientboundPowerupPacket>
{
public:
    ClientboundPowerupPacket();
    ClientboundPowerupPacket(int powerupID, int powerupTime);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    int getPowerupID();
    int getPowerupTime();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundPowerupPacket()); }

private:
    int powerupID;
    int powerupTime;
};

#endif

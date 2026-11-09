#ifndef GAME_NETWORK_CLIENTBOUNDSETTIMEPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDSETTIMEPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundSetTimePacket : public Packet, public boost::enable_shared_from_this<ClientboundSetTimePacket>
{
public:
    ClientboundSetTimePacket();
    ClientboundSetTimePacket(long long gameTime, long long dayTime, bool daylightCycle);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(boost::shared_ptr<Packet> packet);
    virtual bool isAync();

    long long getGameTime();
    long long getDayTime();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundSetTimePacket()); }

private:
    long long gameTime;
    long long dayTime;
};

#endif

#ifndef GAME_NETWORK_CLIENTBOUNDSETHEALTHPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDSETHEALTHPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/core/CTelemetryManager.hpp"
#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundSetHealthPacket : public Packet, public boost::enable_shared_from_this<ClientboundSetHealthPacket>
{
public:
    ClientboundSetHealthPacket();
    ClientboundSetHealthPacket(float health, int food, float saturation, TelemetryDamageSource damageSource);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(boost::shared_ptr<Packet> packet);

    float getHealth();
    int getFood();
    float getSaturation();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundSetHealthPacket()); }

    TelemetryDamageSource damageSource;

private:
    float health;
    int food;
    float saturation;
};

#endif

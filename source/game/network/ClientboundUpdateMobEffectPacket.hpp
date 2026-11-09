#ifndef GAME_NETWORK_CLIENTBOUNDUPDATEMOBEFFECTPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDUPDATEMOBEFFECTPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class MobEffectInstance;
class PacketListener;

class ClientboundUpdateMobEffectPacket : public Packet,
                                         public boost::enable_shared_from_this<ClientboundUpdateMobEffectPacket>
{
public:
    ClientboundUpdateMobEffectPacket();
    ClientboundUpdateMobEffectPacket(int entityId, MobEffectInstance* effect);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(boost::shared_ptr<Packet> packet);
    virtual bool isSuperLongDuration();

    unsigned int getEntityId();
    unsigned char getEffectId();
    unsigned char getEffectAmplifier();
    unsigned int getEffectDurationTicks();
    unsigned char isEffectVisible();
    unsigned char isEffectAmbient();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundUpdateMobEffectPacket());
    }

private:
    int entityId;
    unsigned char effectId;
    unsigned char effectAmplifier;
    int effectDurationTicks;
    unsigned char effectFlags;
};

#endif

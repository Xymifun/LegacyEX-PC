#ifndef GAME_NETWORK_CLIENTBOUNDREMOVEMOBEFFECTPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDREMOVEMOBEFFECTPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class Level;
class MobEffect;
class PacketListener;

class ClientboundRemoveMobEffectPacket : public Packet,
                                         public boost::enable_shared_from_this<ClientboundRemoveMobEffectPacket>
{
public:
    ClientboundRemoveMobEffectPacket();
    ClientboundRemoveMobEffectPacket(int entityId, MobEffect* effect);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    boost::shared_ptr<Entity> getEntity(Level* level);
    MobEffect* getEffect();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundRemoveMobEffectPacket());
    }

private:
    int entityId;
    MobEffect* effect;
};

#endif

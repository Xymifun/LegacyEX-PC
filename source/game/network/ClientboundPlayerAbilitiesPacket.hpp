#ifndef GAME_NETWORK_CLIENTBOUNDPLAYERABILITIESPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDPLAYERABILITIESPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class Abilities;
class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundPlayerAbilitiesPacket : public Packet,
                                         public boost::enable_shared_from_this<ClientboundPlayerAbilitiesPacket>
{
public:
    ClientboundPlayerAbilitiesPacket();
    ClientboundPlayerAbilitiesPacket(Abilities* abilities, int playerId);
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(boost::shared_ptr<Packet> packet);
    void setInvulnerable(bool value);
    void setFlying(bool value);
    void setCanFly(bool value);
    void setInstabuild(bool value);
    void setFlyingSpeed(float value);
    void setWalkingSpeed(float value);
    void setCanBuild(bool value);
    bool isInvulnerable();
    bool isFlying();
    bool canFly();
    bool canInstabuild();
    bool canBuild();
    float getFlyingSpeed();
    float getWalkingSpeed();
    int getPlayerId();
    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundPlayerAbilitiesPacket());
    }

private:
    bool mInvulnerable;
    bool mFlying;
    bool mCanFly;
    bool mInstabuild;
    float mFlyingSpeed;
    float mWalkingSpeed;
    bool mCanBuild;
    int mPlayerId;
};

#endif

#ifndef GAME_NETWORK_SERVERBOUNDPLAYERABILITIESPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDPLAYERABILITIESPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class Abilities;
class DataInputStream;
class DataOutputStream;
class PacketListener;

class ServerboundPlayerAbilitiesPacket : public Packet,
                                         public boost::enable_shared_from_this<ServerboundPlayerAbilitiesPacket>
{
public:
    ServerboundPlayerAbilitiesPacket();
    ServerboundPlayerAbilitiesPacket(Abilities* abilities);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    bool isInvulnerable();
    void setInvulnerable(bool invulnerable);
    bool isFlying();
    void setFlying(bool flying);
    bool canFly();
    void setCanFly(bool mayFly);
    bool canInstabuild();
    void setInstabuild(bool instabuild);
    float getFlyingSpeed();
    void setFlyingSpeed(float flyingSpeed);
    float getWalkingSpeed();
    void setWalkingSpeed(float walkingSpeed);

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ServerboundPlayerAbilitiesPacket());
    }

private:
    bool invulnerable;
    bool flying;
    bool mayFly;
    bool instabuild;
    float flyingSpeed;
    float walkingSpeed;
};

#endif

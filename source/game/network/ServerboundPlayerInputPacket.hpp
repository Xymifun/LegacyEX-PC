#ifndef GAME_NETWORK_SERVERBOUNDPLAYERINPUTPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDPLAYERINPUTPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ServerboundPlayerInputPacket : public Packet, public boost::enable_shared_from_this<ServerboundPlayerInputPacket>
{
public:
    ServerboundPlayerInputPacket();
    ServerboundPlayerInputPacket(float xxa, float zza, bool jumping, bool sneaking);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    float getXxa();
    float getZza();
    bool isJumping();
    bool isSneaking();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ServerboundPlayerInputPacket()); }

private:
    float xxa;
    float zza;
    bool jumping;
    bool sneaking;
};

#endif

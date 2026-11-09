#ifndef GAME_NETWORK_CLIENTBOUNDSETBORDERPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDSETBORDERPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;
class WorldBorder;

class ClientboundSetBorderPacket : public Packet, public boost::enable_shared_from_this<ClientboundSetBorderPacket>
{
public:
    enum Type
    {
        SET_SIZE,
        LERP_SIZE,
        SET_CENTER,
        INITIALIZE,
        SET_WARNING_TIME,
        SET_WARNING_BLOCKS
    };

    ClientboundSetBorderPacket();
    ClientboundSetBorderPacket(WorldBorder* border, Type type);
    virtual ~ClientboundSetBorderPacket();
    virtual void handle(PacketListener* listener);
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    void applyChanges(WorldBorder* border);
    virtual int getPacketId();
    static boost::shared_ptr<Packet> create();

private:
    Type type;
    int absoluteMaxSize;
    double centerX;
    double centerZ;
    double newSize;
    double oldSize;
    long long lerpTime;
    int warningTime;
    int warningBlocks;
};

#endif

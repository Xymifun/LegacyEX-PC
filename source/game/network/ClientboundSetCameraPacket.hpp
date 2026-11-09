#ifndef GAME_NETWORK_CLIENTBOUNDSETCAMERAPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDSETCAMERAPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class PacketListener;

class ClientboundSetCameraPacket : public Packet, public boost::enable_shared_from_this<ClientboundSetCameraPacket>
{
public:
    ClientboundSetCameraPacket();
    ClientboundSetCameraPacket(boost::shared_ptr<Entity> camera);
    virtual ~ClientboundSetCameraPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    int getCameraId();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundSetCameraPacket()); }

private:
    int cameraId;
};

#endif

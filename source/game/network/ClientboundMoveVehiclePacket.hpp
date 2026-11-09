#ifndef GAME_NETWORK_CLIENTBOUNDMOVEVEHICLEPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDMOVEVEHICLEPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class PacketListener;

class ClientboundMoveVehiclePacket : public Packet, public boost::enable_shared_from_this<ClientboundMoveVehiclePacket>
{
public:
    ClientboundMoveVehiclePacket();
    ClientboundMoveVehiclePacket(boost::shared_ptr<Entity> vehicle);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    double getX();
    double getY();
    double getZ();
    double getYRot();
    double getXRot();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundMoveVehiclePacket()); }

private:
    double x;
    double y;
    double z;
    float yRot;
    float xRot;
};

#endif

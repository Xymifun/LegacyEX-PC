#ifndef GAME_NETWORK_SERVERBOUNDMOVEVEHICLEPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDMOVEVEHICLEPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class PacketListener;

class ServerboundMoveVehiclePacket : public Packet, public boost::enable_shared_from_this<ServerboundMoveVehiclePacket>
{
public:
    ServerboundMoveVehiclePacket();
    ServerboundMoveVehiclePacket(boost::shared_ptr<Entity> vehicle);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    double getX();
    double getY();
    double getZ();
    float getYRot();
    float getXRot();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ServerboundMoveVehiclePacket()); }

private:
    double x;
    double y;
    double z;
    float yRot;
    float xRot;
};

#endif

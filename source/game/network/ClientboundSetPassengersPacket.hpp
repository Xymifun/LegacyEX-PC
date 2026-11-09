#ifndef GAME_NETWORK_CLIENTBOUNDSETPASSENGERSPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDSETPASSENGERSPACKET_HPP

#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class Entity;

class ClientboundSetPassengersPacket : public Packet,
                                       public boost::enable_shared_from_this<ClientboundSetPassengersPacket>
{
public:
    ClientboundSetPassengersPacket();
    ClientboundSetPassengersPacket(boost::shared_ptr<Entity> vehicle);
    virtual ~ClientboundSetPassengersPacket();
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    arrayWithLength<int> getPassengers();
    int getVehicle();
    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundSetPassengersPacket());
    }

private:
    int mVehicle;
    arrayWithLength<int> mPassengers;
};

#endif

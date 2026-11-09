#include "game/network/ClientboundSetPassengersPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/PacketListener.hpp"

ClientboundSetPassengersPacket::ClientboundSetPassengersPacket()
{
}

ClientboundSetPassengersPacket::ClientboundSetPassengersPacket(boost::shared_ptr<Entity> vehicle)
{
    mVehicle = vehicle->getId();
    const std::vector<boost::weak_ptr<Entity> >* riders = vehicle->getPassengers();
    std::vector<boost::shared_ptr<Entity> > riderList;

    for (unsigned int i = 0; i < riders->size(); i++)
    {
        boost::shared_ptr<Entity> rider = riders->at(i).lock();

        if (rider)
        {
            riderList.push_back(rider);
        }
    }

    unsigned int riderCount = riderList.size();
    if (riderCount > 0)
    {
        mPassengers = arrayWithLength<int>(riderCount, true);
    }

    for (unsigned int i = 0; i < riderList.size(); i++)
    {
        mPassengers.data[i] = riderList.at(i)->getId();
    }
}

ClientboundSetPassengersPacket::~ClientboundSetPassengersPacket()
{
    delete mPassengers.data;
}

int ClientboundSetPassengersPacket::getPacketId()
{
    return 134;
}

void ClientboundSetPassengersPacket::read(DataInputStream* input)
{
    mVehicle = input->readVarInt();
    mPassengers = input->readVarIntArray();
}

void ClientboundSetPassengersPacket::write(DataOutputStream* output)
{
    output->writeVarInt(mVehicle);
    output->writeVarIntArray(mPassengers);
}

void ClientboundSetPassengersPacket::handle(PacketListener* listener)
{
    listener->handleSetEntityPassengersPacket(shared_from_this());
}

arrayWithLength<int> ClientboundSetPassengersPacket::getPassengers()
{
    return mPassengers;
}

int ClientboundSetPassengersPacket::getVehicle()
{
    return mVehicle;
}

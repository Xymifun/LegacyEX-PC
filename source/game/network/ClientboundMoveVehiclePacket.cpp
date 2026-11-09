#include "game/network/ClientboundMoveVehiclePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/PacketListener.hpp"

ClientboundMoveVehiclePacket::ClientboundMoveVehiclePacket()
{
}

ClientboundMoveVehiclePacket::ClientboundMoveVehiclePacket(boost::shared_ptr<Entity> vehicle)
{
    x = vehicle->mX;
    y = vehicle->mY;
    z = vehicle->mZ;
    yRot = vehicle->mYRot;
    xRot = vehicle->mXRot;
}

int ClientboundMoveVehiclePacket::getPacketId()
{
    return 133;
}

void ClientboundMoveVehiclePacket::read(DataInputStream* input)
{
    x = input->readDouble();
    y = input->readDouble();
    z = input->readDouble();
    yRot = input->readFloat();
    xRot = input->readFloat();
}

void ClientboundMoveVehiclePacket::write(DataOutputStream* output)
{
    output->writeDouble(x);
    output->writeDouble(y);
    output->writeDouble(z);
    output->writeFloat(yRot);
    output->writeFloat(xRot);
}

void ClientboundMoveVehiclePacket::handle(PacketListener* listener)
{
    listener->handleMoveVehicle(shared_from_this());
}

double ClientboundMoveVehiclePacket::getX()
{
    return x;
}

double ClientboundMoveVehiclePacket::getY()
{
    return y;
}

double ClientboundMoveVehiclePacket::getZ()
{
    return z;
}

double ClientboundMoveVehiclePacket::getYRot()
{
    return yRot;
}

double ClientboundMoveVehiclePacket::getXRot()
{
    return xRot;
}

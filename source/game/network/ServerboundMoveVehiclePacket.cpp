#include "game/network/ServerboundMoveVehiclePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ServerboundMoveVehiclePacket::ServerboundMoveVehiclePacket()
{
}

ServerboundMoveVehiclePacket::ServerboundMoveVehiclePacket(boost::shared_ptr<Entity> vehicle)
{
    x = vehicle->mX;
    y = vehicle->mY;
    z = vehicle->mZ;
    yRot = vehicle->mYRot;
    xRot = vehicle->mXRot;
}

int ServerboundMoveVehiclePacket::getPacketId()
{
    return 136;
}

void ServerboundMoveVehiclePacket::read(DataInputStream* input)
{
    x = input->readDouble();
    y = input->readDouble();
    z = input->readDouble();
    yRot = input->readFloat();
    xRot = input->readFloat();
}

void ServerboundMoveVehiclePacket::write(DataOutputStream* output)
{
    output->writeDouble(x);
    output->writeDouble(y);
    output->writeDouble(z);
    output->writeFloat(yRot);
    output->writeFloat(xRot);
}

void ServerboundMoveVehiclePacket::handle(PacketListener* listener)
{
    listener->handleMoveVehicle(shared_from_this());
}

double ServerboundMoveVehiclePacket::getX()
{
    return x;
}

double ServerboundMoveVehiclePacket::getY()
{
    return y;
}

double ServerboundMoveVehiclePacket::getZ()
{
    return z;
}

float ServerboundMoveVehiclePacket::getYRot()
{
    return yRot;
}

float ServerboundMoveVehiclePacket::getXRot()
{
    return xRot;
}

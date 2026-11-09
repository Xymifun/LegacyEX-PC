#include "game/network/ServerboundInteractPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"

ServerboundInteractPacket::ServerboundInteractPacket()
{
    entityId = 0;
    action = ATTACK;
    hand = InteractionHand::eInteractionHand_MainHand;
    location = NULL;
}

ServerboundInteractPacket::ServerboundInteractPacket(boost::shared_ptr<Entity> target)
{
    entityId = target->getId();
    action = ATTACK;
    hand = InteractionHand::eInteractionHand_MainHand;
    location = NULL;
}

ServerboundInteractPacket::ServerboundInteractPacket(boost::shared_ptr<Entity> target,
                                                     InteractionHand::EInteractionHand hand)
{
    entityId = target->getId();
    action = INTERACT;
    this->hand = hand;
    location = NULL;
}

ServerboundInteractPacket::ServerboundInteractPacket(boost::shared_ptr<Entity> target,
                                                     InteractionHand::EInteractionHand hand, Vec3* location)
{
    entityId = target->getId();
    action = INTERACT_AT;
    this->hand = hand;
    this->location = Vec3::newPermanent(location->x, location->y, location->z);
}

ServerboundInteractPacket::~ServerboundInteractPacket()
{
    if (location != NULL)
    {
        delete location;
    }
}

int ServerboundInteractPacket::getPacketId()
{
    return 7;
}

void ServerboundInteractPacket::read(DataInputStream* input)
{
    entityId = input->readVarInt();
    action = (Action)input->readInt();

    if (action == INTERACT_AT)
    {
        float x = input->readFloat();
        float y = input->readFloat();
        float z = input->readFloat();

        location = Vec3::newPermanent(x, y, z);
    }

    if (action == INTERACT || action == INTERACT_AT)
    {
        hand = (InteractionHand::EInteractionHand)input->readByte();
    }
}

void ServerboundInteractPacket::write(DataOutputStream* output)
{
    output->writeVarInt(entityId);
    output->writeInt(action);

    if (action == INTERACT_AT)
    {
        output->writeFloat((float)location->x);
        output->writeFloat((float)location->y);
        output->writeFloat((float)location->z);
    }

    if (action == INTERACT || action == INTERACT_AT)
    {
        output->writeByte((unsigned char)hand);
    }
}

void ServerboundInteractPacket::handle(PacketListener* listener)
{
    listener->handleInteract(shared_from_this());
}

int ServerboundInteractPacket::getEstimatedSize()
{
    return 9;
}

boost::shared_ptr<Entity> ServerboundInteractPacket::getTarget(Level* level)
{
    return level->getEntity(entityId);
}

ServerboundInteractPacket::Action ServerboundInteractPacket::getAction()
{
    return action;
}

InteractionHand::EInteractionHand ServerboundInteractPacket::getHand()
{
    return hand;
}

Vec3* ServerboundInteractPacket::getLocation()
{
    return Vec3::newTemp(location);
}

#include "game/network/ServerboundPlayerCommandPacket.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/PacketListener.hpp"

ServerboundPlayerCommandPacket::ServerboundPlayerCommandPacket() : id(-1), action(PRESS_SHIFT_KEY), data(0)
{
}
ServerboundPlayerCommandPacket::ServerboundPlayerCommandPacket(boost::shared_ptr<Entity> entity, Action action)
    : id(entity->getId()),
      action(action),
      data(0)
{
}
ServerboundPlayerCommandPacket::ServerboundPlayerCommandPacket(boost::shared_ptr<Entity> entity, Action action,
                                                               int data)
    : id(entity->getId()),
      action(action),
      data(data)
{
}
void ServerboundPlayerCommandPacket::read(DataInputStream* input)
{
    id = input->readVarInt();
    action = (Action)input->readInt();
    data = input->readVarInt();
}
void ServerboundPlayerCommandPacket::write(DataOutputStream* output)
{
    output->writeVarInt(id);
    output->writeInt((int)action);
    output->writeVarInt(data);
}
void ServerboundPlayerCommandPacket::handle(PacketListener* listener)
{
    listener->handlePlayerCommand(shared_from_this());
}
int ServerboundPlayerCommandPacket::getEstimatedSize()
{
    return 9;
}
ServerboundPlayerCommandPacket::Action ServerboundPlayerCommandPacket::getAction()
{
    return action;
}
int ServerboundPlayerCommandPacket::getData()
{
    return data;
}
int ServerboundPlayerCommandPacket::getPacketId()
{
    return 19;
}
boost::shared_ptr<Packet> ServerboundPlayerCommandPacket::create()
{
    return boost::shared_ptr<Packet>(new ServerboundPlayerCommandPacket());
}

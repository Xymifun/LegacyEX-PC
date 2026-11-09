#include "game/network/ServerboundContainerClickPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ServerboundContainerClickPacket::ServerboundContainerClickPacket()
{
    containerId = 0;
    slotNum = 0;
    buttonNum = 0;
    uid = 0;
    clickType = ClickType::eClickType_Pickup;
    numSlots = 0;
}

ServerboundContainerClickPacket::ServerboundContainerClickPacket(int containerId, int slotNum, int buttonNum,
                                                                 ClickType::eClickType clickType,
                                                                 not_null_ptr<ItemInstance> item, short uid)
{
    this->containerId = containerId;
    this->slotNum = slotNum;
    this->buttonNum = buttonNum;
    this->uid = uid;
    this->clickType = clickType;
    items.push_back(item);
    numSlots = 1;
}

ServerboundContainerClickPacket::ServerboundContainerClickPacket(int containerId, int slotNum, int buttonNum,
                                                                 ClickType::eClickType clickType,
                                                                 std::vector<not_null_ptr<ItemInstance> > items,
                                                                 short uid, int numSlots)
{
    this->containerId = containerId;
    this->slotNum = slotNum;
    this->buttonNum = buttonNum;
    this->clickType = clickType;
    this->uid = uid;
    this->items = items;
    this->numSlots = numSlots;
}

ServerboundContainerClickPacket::~ServerboundContainerClickPacket()
{
    items.clear();
}

int ServerboundContainerClickPacket::getPacketId()
{
    return 102;
}

void ServerboundContainerClickPacket::read(DataInputStream* input)
{
    containerId = input->readByte();
    slotNum = input->readShort();
    buttonNum = input->readByte();
    uid = input->readShort();
    clickType = (ClickType::eClickType)input->readByte();
    numSlots = input->readByte();

    items.reserve(numSlots);

    for (int i = 0; i < numSlots; i++)
    {
        items.push_back(input->readItem());
    }
}

void ServerboundContainerClickPacket::write(DataOutputStream* output)
{
    output->writeByte(containerId);
    output->writeShort(slotNum);
    output->writeByte(buttonNum);
    output->writeShort(uid);
    output->writeByte(clickType);
    output->writeByte(numSlots);

    for (std::vector<not_null_ptr<ItemInstance> >::iterator it = items.begin(); it != items.end(); it++)
    {
        output->writeItem(*it);
    }
}

void ServerboundContainerClickPacket::handle(PacketListener* listener)
{
    listener->handleContainerClick(shared_from_this());
}

int ServerboundContainerClickPacket::getEstimatedSize()
{
    return 12;
}

int ServerboundContainerClickPacket::getContainerId()
{
    return containerId;
}

int ServerboundContainerClickPacket::getSlotNum()
{
    return slotNum;
}

int ServerboundContainerClickPacket::getButtonNum()
{
    return buttonNum;
}

int ServerboundContainerClickPacket::getUid()
{
    return uid;
}

not_null_ptr<ItemInstance> ServerboundContainerClickPacket::getItem()
{
    return items.front();
}

not_null_ptr<ItemInstance> ServerboundContainerClickPacket::getItemAt(unsigned int index)
{
    return items.at(index);
}

std::vector<not_null_ptr<ItemInstance> > ServerboundContainerClickPacket::getItems()
{
    return items;
}

ClickType::eClickType ServerboundContainerClickPacket::getClickType()
{
    return clickType;
}

int ServerboundContainerClickPacket::getNumSlots()
{
    return numSlots;
}

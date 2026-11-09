#include "game/network/ServerboundSetCreativeModeSlotPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/network/PacketListener.hpp"

ServerboundSetCreativeModeSlotPacket::ServerboundSetCreativeModeSlotPacket()
{
    slot = 0;
    item = ItemInstance::EMPTY;
}

ServerboundSetCreativeModeSlotPacket::ServerboundSetCreativeModeSlotPacket(int slot, not_null_ptr<ItemInstance> item)
{
    this->slot = slot;
    this->item = item->isEmpty() ? ItemInstance::EMPTY : item->copy();
}

void ServerboundSetCreativeModeSlotPacket::handle(PacketListener* listener)
{
    listener->handleSetCreativeModeSlot(shared_from_this());
}

void ServerboundSetCreativeModeSlotPacket::read(DataInputStream* input)
{
    slot = input->readShort();
    item = input->readItem();
}

void ServerboundSetCreativeModeSlotPacket::write(DataOutputStream* output)
{
    output->writeShort(static_cast<short>(slot));
    output->writeItem(item);
}

int ServerboundSetCreativeModeSlotPacket::getEstimatedSize()
{
    return 8;
}

int ServerboundSetCreativeModeSlotPacket::getSlotNum()
{
    return slot;
}

not_null_ptr<ItemInstance> ServerboundSetCreativeModeSlotPacket::getItem()
{
    return item;
}

int ServerboundSetCreativeModeSlotPacket::getPacketId()
{
    return 107;
}

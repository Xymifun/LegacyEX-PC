#include "game/network/ClientboundContainerSetContentPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientboundContainerSetContentPacket::ClientboundContainerSetContentPacket()
{
    containerId = 0;
}

ClientboundContainerSetContentPacket::ClientboundContainerSetContentPacket(
    int containerId, std::vector<not_null_ptr<ItemInstance> >* itemList)
{
    unsigned int count = itemList->size();

    this->containerId = containerId;
    items = NonNullList<not_null_ptr<ItemInstance> >::withSize(count, ItemInstance::EMPTY);

    for (unsigned int i = 0; i < count; ++i)
    {
        not_null_ptr<ItemInstance> item = itemList->at(i);
        if (!item->isEmpty())
            items[i] = item->copy();
    }
}

ClientboundContainerSetContentPacket::~ClientboundContainerSetContentPacket()
{
    delete[] items.items.data;
}

void ClientboundContainerSetContentPacket::read(DataInputStream* input)
{
    containerId = input->readByte();

    int count = input->readShort();
    delete[] items.items.data;
    items = NonNullList<not_null_ptr<ItemInstance> >::withSize(count, ItemInstance::EMPTY);

    for (int i = 0; i < count; ++i)
        items.set(i, input->readItem());
}

void ClientboundContainerSetContentPacket::write(DataOutputStream* output)
{
    output->writeByte(containerId);
    output->writeShort(static_cast<short>(items.size()));

    for (unsigned int i = 0; i < items.size(); ++i)
        output->writeItem(items[i]);
}

void ClientboundContainerSetContentPacket::handle(PacketListener* listener)
{
    listener->handleContainerContent(shared_from_this());
}

int ClientboundContainerSetContentPacket::getEstimatedSize()
{
    return items.size() * 5 + 3;
}

int ClientboundContainerSetContentPacket::getContainerId()
{
    return containerId;
}

NonNullList<not_null_ptr<ItemInstance> > ClientboundContainerSetContentPacket::getItems()
{
    return items;
}

int ClientboundContainerSetContentPacket::getPacketId()
{
    return 104;
}

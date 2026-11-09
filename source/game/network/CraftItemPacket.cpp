#include "game/network/CraftItemPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

CraftItemPacket::CraftItemPacket() : mStackedItems(), mItemIds(), mContainerId(0), mCraftType(0)
{
}

CraftItemPacket::CraftItemPacket(int recipe, int craftType, short containerId) : mStackedItems(), mItemIds()
{
    mStackedItems.push_back(std::make_pair(recipe, -1));
    mContainerId = containerId;
    mCraftType = static_cast<unsigned char>(craftType);
}

CraftItemPacket::CraftItemPacket(std::vector<std::pair<int, int> >& stackedItems, int craftType, short containerId)
    : mStackedItems(stackedItems),
      mItemIds(),
      mContainerId(containerId),
      mCraftType(static_cast<unsigned char>(craftType))
{
}

CraftItemPacket::CraftItemPacket(std::vector<int>& itemIds, short containerId) : mStackedItems(), mItemIds()
{
    for (std::vector<int>::iterator it = itemIds.begin(); it != itemIds.end(); ++it)
    {
        mItemIds.push_back(*it);
    }
    mContainerId = containerId;
}

CraftItemPacket::~CraftItemPacket()
{
}

int CraftItemPacket::getPacketId()
{
    return 150;
}

void CraftItemPacket::read(DataInputStream* input)
{
    mContainerId = input->readShort();
    mCraftType = input->read();
    const bool stackedItems = input->readBoolean();
    unsigned int itemCount = input->readInt();
    for (unsigned int index = 0; index < itemCount; ++index)
    {
        if (!stackedItems)
        {
            mItemIds.push_back(input->readInt());
        }
        else
        {
            int itemId = input->readInt();
            int itemCountValue = input->readInt();
            mStackedItems.push_back(std::make_pair(itemId, itemCountValue));
        }
    }
}

void CraftItemPacket::write(DataOutputStream* output)
{
    output->writeShort(mContainerId);
    output->write(mCraftType);
    unsigned int count = mStackedItems.size();
    if (count > 0)
    {
        output->writeBoolean(true);
        output->writeInt(count);
        for (std::vector<std::pair<int, int> >::iterator it = mStackedItems.begin(); it != mStackedItems.end(); ++it)
        {
            output->writeInt(it->first);
            output->writeInt(it->second);
        }
    }
    else
    {
        count = mItemIds.size();
        output->writeBoolean(false);
        output->writeInt(count);
        for (std::vector<int>::iterator it = mItemIds.begin(); it != mItemIds.end(); ++it)
        {
            output->writeInt(*it);
        }
    }
}

void CraftItemPacket::handle(PacketListener* listener)
{
    listener->handleCraftItem(shared_from_this());
}

int CraftItemPacket::getEstimatedSize()
{
    return 7;
}

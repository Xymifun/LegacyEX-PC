#include "game/network/MapSelectInfoPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

MapSelectInfoPacket::MapSelectInfoPacket() : mData()
{
    mOwnsData = false;
    mSize = 0;
}

MapSelectInfoPacket::MapSelectInfoPacket(arrayWithLength<unsigned char> data, bool ownsData) : mData()
{
    mOwnsData = ownsData;
    mData = data;
    mSize = 0;
    if (data.data != NULL)
    {
        mSize = data.length;
    }
}

MapSelectInfoPacket::~MapSelectInfoPacket()
{
    if (mOwnsData)
        delete[] mData.data;
}

void MapSelectInfoPacket::read(DataInputStream* input)
{
    mSize = static_cast<unsigned int>(input->readShort());
    if (mSize - 1 < 0x7ffe)
    {
        if (mData.data != NULL)
            delete[] mData.data;
        mData = arrayWithLength<unsigned char>(mSize, true);
        input->readBytes(mData);
    }
}

void MapSelectInfoPacket::write(DataOutputStream* output)
{
    output->writeShort(static_cast<short>(mSize));
    if (mData.data != NULL)
    {
        output->writeBytes(mData);
    }
}

void MapSelectInfoPacket::handle(PacketListener* listener)
{
    listener->handleMapSelectInfo(shared_from_this());
}

int MapSelectInfoPacket::getEstimatedSize()
{
    return static_cast<int>(mSize) + 2;
}

int MapSelectInfoPacket::getPacketId()
{
    return 0xa8;
}

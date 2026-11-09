#include "game/network/ClientboundChunkBlocksUpdatePacket.hpp"

#include <cstdlib>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/chunk/LevelChunk.hpp"

ClientboundChunkBlocksUpdatePacket::ClientboundChunkBlocksUpdatePacket()
{
    shouldDelay = true;
    z = 0;
    x = 0;
    count = 0;
}

ClientboundChunkBlocksUpdatePacket::ClientboundChunkBlocksUpdatePacket(int x, int z, arrayWithLength<short> changes,
                                                                       unsigned char count, Level* level)
{
    shouldDelay = true;
    this->x = x;
    this->z = z;
    this->count = count;
    positions = arrayWithLength<short>(count, true);
    elements = arrayWithLength<unsigned int>(count, true);

    LevelChunk* chunk = level->getChunk(x, z);

    short* sortedPositions = positions.data;
    for (int i = 0; i < count; ++i)
    {
        sortedPositions[i] = changes[i];
    }

    qsort(sortedPositions, count, sizeof(short), PositionSortFunction);

    for (int i = 0; i < count; ++i)
    {
        int packedPosition = positions[i];
        int localX = (packedPosition >> 12) & 15;
        int localZ = (packedPosition >> 8) & 15;
        int y = packedPosition & 255;
        unsigned int element = chunk->getElement(localX, y, localZ);
        int extraElement = chunk->getExtraElement(localX, y, localZ);
        elements[i] = element | (extraElement << 16);
    }

    if (level->mDimension->getType() == DimensionType::OVERWORLD)
    {
        dimension = 0;
    }
    else if (level->mDimension->getType() == DimensionType::NETHER)
    {
        dimension = 1;
    }
    else
    {
        dimension = 2;
    }
}

ClientboundChunkBlocksUpdatePacket::~ClientboundChunkBlocksUpdatePacket()
{
    delete[] elements.data;
    delete[] positions.data;
}

int ClientboundChunkBlocksUpdatePacket::PositionSortFunction(const void* first, const void* second)
{
    const unsigned char* firstPosition = static_cast<const unsigned char*>(first);
    const unsigned char* secondPosition = static_cast<const unsigned char*>(second);
    return firstPosition[1] - secondPosition[1];
}

void ClientboundChunkBlocksUpdatePacket::read(DataInputStream* input)
{
    unsigned int header = input->readInt();
    x = static_cast<int>(header << 12) >> 22;
    z = static_cast<int>(header << 22) >> 22;
    count = (header >> 20) & 255;
    dimension = static_cast<int>(header) >> 28;

    positions = arrayWithLength<short>(count, true);
    elements = arrayWithLength<unsigned int>(count, true);

    unsigned int previousElement = 0;
    unsigned short previousPosition = 0;

    for (int i = 0; i < static_cast<int>(count); ++i)
    {
        unsigned int value = input->readByte();
        unsigned int newY = (value >> 7) & 1;

        if (((value >> 6) & 1) != 0)
        {
            unsigned int shift = 6;
            previousElement = value & 63;
            unsigned int part;
            do
            {
                part = input->readByte();
                previousElement |= (part & 127) << shift;
                shift += 7;
            } while ((part & 128) != 0);
        }
        else
        {
            if ((value >> 5) & 1)
                previousElement = 0;
            previousElement = (previousElement & 0xfffffff0) | (value & 15);
        }

        elements.data[i] = previousElement;

        if (newY)
        {
            previousPosition = input->readByte() << 8;
            previousPosition |= input->readByte();
        }
        else
        {
            unsigned int xz = input->readByte();
            previousPosition = static_cast<unsigned short>((xz << 8) | (previousPosition & 255));
        }
        positions.data[i] = static_cast<short>(previousPosition);
    }
}

void ClientboundChunkBlocksUpdatePacket::write(DataOutputStream* output)
{
    int header = ((x & 1023) << 10) | (z & 1023) | (count << 20) | (dimension << 28);
    output->writeInt(header);

    unsigned int previousY = static_cast<unsigned int>(-1);
    unsigned int previousElement = static_cast<unsigned int>(-1);

    for (int i = 0; i < static_cast<int>(count); ++i)
    {
        int packedPosition = positions.data[i];
        unsigned int y = packedPosition & 255;
        unsigned int xz = (packedPosition >> 8) & 255;
        unsigned int element = elements.data[i];
        bool newY = y != previousY;
        unsigned int value = 0;
        if (newY)
        {
            value = 128;
        }

        bool sameHighBits = ((element ^ previousElement) & 0xfffffff0) == 0;
        if (element == 0 || sameHighBits)
        {
            if (element == 0)
            {
                value |= 32;
            }

            output->write(value | (element & 15));
            output->write(xz);
        }
        else
        {
            output->write(value | 64 | (element & 63));
            unsigned int remaining = element >> 6;
            do
            {
                unsigned int part = remaining & 127;
                remaining >>= 7;
                if (remaining != 0)
                {
                    part |= 128;
                }
                output->write(part);
            } while (remaining != 0);
            output->write(xz);
        }

        if (newY)
        {
            output->write(y);
        }

        previousY = y;
        previousElement = element;
    }
}

void ClientboundChunkBlocksUpdatePacket::handle(PacketListener* listener)
{
    listener->handleChunkBlocksUpdate(shared_from_this());
}

int ClientboundChunkBlocksUpdatePacket::getEstimatedSize()
{
    return count * 3 + 4;
}

int ClientboundChunkBlocksUpdatePacket::getPacketId()
{
    return 52;
}

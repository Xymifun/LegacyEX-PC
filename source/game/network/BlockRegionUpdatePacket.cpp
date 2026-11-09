#include "game/network/BlockRegionUpdatePacket.hpp"

#include <cstring>

#include "game/core/Compression.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/core/MemSect.hpp"

BlockRegionUpdatePacket::BlockRegionUpdatePacket()
{
    mDepth = 0;
    mY = 0;
    mX = 0;
    mZ = 0;
    mWidth = 0;
    mHeight = 0;
    mFullChunk = false;
    shouldDelay = true;
    mChanges = new std::vector<LevelChunk::BlockChange>;
}

BlockRegionUpdatePacket::BlockRegionUpdatePacket(int x, int y, int z, int width, int height, int depth, Level* level,
                                                 bool hasBlockChanges, const std::vector<short>* blockChanges)
    : mX(x),
      mY(y),
      mZ(z),
      mWidth(width),
      mHeight(height),
      mDepth(depth),
      mFullChunk(false)
{
    shouldDelay = true;
    mChanges = new std::vector<LevelChunk::BlockChange>;
    mHasBlockChanges = false;

    if (level->mDimension->getType() == DimensionType::OVERWORLD)
        mDimension = 0;
    else if (level->mDimension->getType() == DimensionType::NETHER)
        mDimension = 1;
    else
        mDimension = 2;

    if (hasBlockChanges)
    {
        mSize = 0;
        mData.data = NULL;
        mData.length = 0;
        if (width == 16 && height == 256 && depth == 16 && (x & 15) == 0 && y == 0 && (z & 15) == 0)
            mFullChunk = true;
        mHasBlockChanges = true;

        if (blockChanges != NULL)
        {
            for (std::vector<short>::const_iterator it = blockChanges->begin(); it != blockChanges->end(); ++it)
            {
                BlockPos pos = LevelChunk::UnpackBlockPos(*it);
                LevelChunk* chunk = level->getChunkAt(x, z);
                unsigned int element = Block::blockStateRegistry->getId(chunk->getBlockState(pos));
                unsigned int extraElement = Block::blockStateRegistry->getId(chunk->getExtraBlockState(pos));
                LevelChunk::BlockChange change(*it, element | (extraElement << 16));
                mChanges->push_back(change);
            }
        }
    }
    else
    {
        arrayWithLength<unsigned char> rawData;
        if (width == 16 && height == 256 && depth == 16 && (x & 15) == 0 && y == 0 && (z & 15) == 0)
        {
            mFullChunk = true;
            rawData = level->getChunkAt(BlockPos(x, 0, z))->getRawData();
        }
        else
        {
            MemSect(0x32);
            rawData = level->getBlocksAndData(x, y, z, width, height, depth, false);
            MemSect(0);
        }

        if (rawData.length == 0)
        {
            mSize = 0;
            mData.data = NULL;
            mData.length = 0;
        }
        else
        {
            unsigned int compressedSize = 0x30000;
            unsigned char* compressedData = new unsigned char[compressedSize];
            unsigned int rleSize = 0;
            Compression::getCompression()->CompressLZXRLE(compressedData, &compressedSize, rawData.data, rawData.length,
                                                          &rleSize);
            unsigned char* copy = new unsigned char[compressedSize];
            std::memcpy(copy, compressedData, compressedSize);
            delete[] compressedData;
            mData.data = copy;
            mData.length = compressedSize;
            delete[] rawData.data;
            mSize = compressedSize;
        }
    }
}

BlockRegionUpdatePacket::~BlockRegionUpdatePacket()
{
    delete[] mData.data;
    delete mChanges;
}

void BlockRegionUpdatePacket::read(DataInputStream* input)
{
    unsigned int flags = input->readByte();
    mX = input->readInt();
    mY = input->readShort();
    mZ = input->readInt();
    mWidth = input->readByte() + 1;
    mHeight = input->readByte() + 1;
    mDepth = input->readByte() + 1;
    mFullChunk = (flags & 1) != 0;
    mHasBlockChanges = (flags & 4) != 0;
    if ((flags & 2) != 0)
        mHeight = 0;
    unsigned int packedSize = (unsigned int)input->readInt();
    mDimension = packedSize >> 30;
    mSize = packedSize & 0x3fffffff;

    int changeCount;
    if (mSize == 0)
    {
        mData.data = NULL;
        mData.length = 0;
        changeCount = input->readChar();
        mChanges->reserve(changeCount);
    }
    else
    {
        arrayWithLength<unsigned char> compressedData(mSize, true);
        bool readComplete = input->readBytes(compressedData);
        unsigned int rawSize = mWidth * mHeight * mDepth * 5;
        if (mFullChunk)
            rawSize = LevelChunk::getRawDataMaxBytes();
        mData = arrayWithLength<unsigned char>(rawSize, true);
        unsigned int outputSize = rawSize;
        if (readComplete)
            Compression::getCompression()->DecompressLZXRLE(mData.data, &outputSize, compressedData.data, mSize);
        delete[] compressedData.data;
        changeCount = input->readChar();
        mChanges->reserve(changeCount);
    }

    for (int i = 0; i < changeCount; ++i)
    {
        short packedPos = input->readShort();
        unsigned int element = (unsigned int)input->readInt();
        mChanges->push_back(LevelChunk::BlockChange(packedPos, element));
    }
}

void BlockRegionUpdatePacket::write(DataOutputStream* output)
{
    unsigned char flags = 0;
    if (mFullChunk)
        flags = 1;
    if (mHasBlockChanges)
        flags |= 4;
    if (mHeight == 0)
        flags |= 2;
    output->writeByte(flags);
    output->writeInt(mX);
    output->writeShort(static_cast<short>(mY));
    output->writeInt(mZ);
    output->writeByte(static_cast<unsigned char>((mWidth - 1) & 0xff));
    output->writeByte(static_cast<unsigned char>((mHeight - 1) & 0xff));
    output->writeByte(static_cast<unsigned char>((mDepth - 1) & 0xff));
    output->writeInt(mSize | (mDimension << 30));
    output->write(mData, 0, mSize);
    output->writeChar(static_cast<char>(mChanges->size()));
    for (std::vector<LevelChunk::BlockChange>::const_iterator it = mChanges->begin(); it != mChanges->end(); ++it)
    {
        output->writeShort(it->GetPackedPos());
        output->writeInt((int)it->GetElement());
    }
}

void BlockRegionUpdatePacket::handle(PacketListener* listener)
{
    listener->handleBlockRegionUpdate(shared_from_this());
}

int BlockRegionUpdatePacket::getEstimatedSize()
{
    return mSize + 17;
}

void BlockRegionUpdatePacket::SetChanges(std::vector<LevelChunk::BlockChange>* changes)
{
    mChanges = changes;
}

std::vector<LevelChunk::BlockChange>* BlockRegionUpdatePacket::GetChanges() const
{
    return mChanges;
}

int BlockRegionUpdatePacket::getPacketId()
{
    return 0x33;
}

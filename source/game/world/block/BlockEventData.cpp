#include "game/world/block/BlockEventData.hpp"
#include "game/world/block/Block.hpp"

BlockEventData::BlockEventData(const BlockPos& pos, Block* blockValue, int eventIdValue, int eventParamValue)
    : BlockPos()
{
    Vec3i::operator=(pos);
    eventId = eventIdValue;
    eventParam = eventParamValue;
    block = blockValue;
}

BlockPos BlockEventData::getPos()
{
    return BlockPos(*this);
}

int BlockEventData::getParamA()
{
    return eventId;
}

int BlockEventData::getParamB()
{
    return eventParam;
}

Block* BlockEventData::getBlock()
{
    return block;
}

bool BlockEventData::equals(BlockEventData& other)
{
    return BlockPos::equals(other) && eventId == other.eventId && eventParam == other.eventParam
           && block == other.block;
}

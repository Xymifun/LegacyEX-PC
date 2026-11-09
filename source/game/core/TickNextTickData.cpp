#include "game/core/TickNextTickData.hpp"

#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

static long long nextTickEntryId = 0;

TickNextTickData::TickNextTickData(const BlockPos& pos, Block* block)
{
    this->triggerTick = 0;
    this->tickEntryId = nextTickEntryId++;
    this->pos = pos;
    this->block = block;
    this->priorityTilt = 0;
}

Block* TickNextTickData::getBlock()
{
    return block;
}

bool TickNextTickData::equals(const TickNextTickData* other) const
{
    if (other == NULL)
    {
        return false;
    }

    return pos.equals(other->pos) && Block::isMatching(block, other->block);
}

int TickNextTickData::hashCode() const
{
    return this->pos.hashCode();
}

void TickNextTickData::delay(long long triggerTick)
{
    this->triggerTick = triggerTick;
}

void TickNextTickData::setPriorityTilt(int priorityTilt)
{
    this->priorityTilt = priorityTilt;
}

int TickNextTickData::compareTo(const TickNextTickData* other) const
{
    if (triggerTick < other->triggerTick)
    {
        return -1;
    }

    if (triggerTick > other->triggerTick)
    {
        return 1;
    }

    if (priorityTilt != other->priorityTilt)
    {
        return priorityTilt - other->priorityTilt;
    }

    if (tickEntryId < other->tickEntryId)
    {
        return -1;
    }

    if (tickEntryId > other->tickEntryId)
    {
        return 1;
    }

    return 0;
}

bool TickNextTickData::compare_fnct(const TickNextTickData& first, const TickNextTickData& second)
{
    return first.compareTo(&second) < 0;
}

int TickNextTickData::hash_fnct(const TickNextTickData& data)
{
    return data.hashCode();
}

bool TickNextTickData::eq_test(const TickNextTickData& first, const TickNextTickData& second)
{
    return first.equals(&second);
}

bool TickNextTickData::operator==(const TickNextTickData& other)
{
    return this->equals(&other);
}

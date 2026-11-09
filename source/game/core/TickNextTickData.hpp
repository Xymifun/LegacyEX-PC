#ifndef GAME_CORE_TICKNEXTTICKDATA_HPP
#define GAME_CORE_TICKNEXTTICKDATA_HPP

#include "game/world/block/BlockPos.hpp"

class Block;

class TickNextTickData
{
public:
    TickNextTickData(const BlockPos& pos, Block* block);

    bool equals(const TickNextTickData* other) const;
    int hashCode() const;
    void delay(long long triggerTick);
    void setPriorityTilt(int priorityTilt);
    int compareTo(const TickNextTickData* other) const;
    Block* getBlock();

    static bool compare_fnct(const TickNextTickData& first, const TickNextTickData& second);
    static int hash_fnct(const TickNextTickData& data);
    static bool eq_test(const TickNextTickData& first, const TickNextTickData& second);

    bool operator==(const TickNextTickData& other);

    Block* block;
    BlockPos pos;
    long long triggerTick;
    int priorityTilt;

private:
    long long tickEntryId;
};

#endif

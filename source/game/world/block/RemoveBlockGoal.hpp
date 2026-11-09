#ifndef GAME_WORLD_BLOCK_REMOVEBLOCKGOAL_HPP
#define GAME_WORLD_BLOCK_REMOVEBLOCKGOAL_HPP

#include "game/world/block/MoveToBlockGoal.hpp"

class Block;
class Level;
class PathfinderMob;
class Random;
class SoundEvent;

class RemoveBlockGoal : public MoveToBlockGoal
{
public:
    RemoveBlockGoal(Block* block, PathfinderMob* mob, double speedModifier, int searchRange, int verticalSearchStart,
                    double maxStayDistance, const SoundEvent* startSound, const SoundEvent* endSound);
    virtual ~RemoveBlockGoal();

    virtual bool canUse();
    virtual int GetNextStartTick(PathfinderMob* mob);
    virtual bool canContinueToUse();
    virtual void stop();
    virtual void start();
    BlockPos* getPosWithBlock(BlockPos pos, Level* level);
    virtual void tick();
    virtual bool isValidTarget(Level* level, const BlockPos& pos);
    virtual bool isValidTargetArea(Level* level, const BlockPos& from, const BlockPos& to);

private:
    Block* mBlock;
    PathfinderMob* mMob;
    int mTicks;
    const SoundEvent* mStartSound;
    const SoundEvent* mEndSound;
};

#endif

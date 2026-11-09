#ifndef GAME_WORLD_BLOCK_MOVETOBLOCKGOAL_HPP
#define GAME_WORLD_BLOCK_MOVETOBLOCKGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"
#include "game/world/block/BlockPos.hpp"

class Level;
class PathfinderMob;

class MoveToBlockGoal : public Goal
{
public:
    MoveToBlockGoal(PathfinderMob* mob, double speedModifier, int searchRange, int verticalSearchStart,
                    double maxStayDistance);
    virtual ~MoveToBlockGoal() {}

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void tick();
    virtual std::wstring getName() { return L"MoveToBlock"; }
    virtual int GetNextStartTick(PathfinderMob* mob);
    virtual bool canReach(const BlockPos& pos);
    virtual bool isValidTarget(Level* level, const BlockPos& pos) = 0;
    virtual bool isValidTargetArea(Level* level, const BlockPos& from, const BlockPos& to);

protected:
    bool findNearestBlock();
    int yOff();
    bool isReachedTarget();

    PathfinderMob* mMob;
    int mNextStartTick;
    int mTicksStayingStill;
    int mMaxStayDuration;
    BlockPos mBlockPos;
    bool mReachedTarget;
    int mSearchRange;
    int mVerticalSearchStart;
    double mSpeedModifierSquared;
    double mSpeedModifier;
};

#endif

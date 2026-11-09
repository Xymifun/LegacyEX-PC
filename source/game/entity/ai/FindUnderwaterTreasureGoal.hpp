#ifndef GAME_ENTITY_AI_FINDUNDERWATERTREASUREGOAL_HPP
#define GAME_ENTITY_AI_FINDUNDERWATERTREASUREGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"

class BlockPos;
class Entity;
class PathfinderMob;

class FindUnderwaterTreasureGoal : public Goal
{
public:
    FindUnderwaterTreasureGoal(PathfinderMob* mob, float speedModifier, int searchRadius, int maxDistance);
    virtual ~FindUnderwaterTreasureGoal();

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual bool canInterrupt();
    virtual void start();
    virtual void stop();
    virtual void tick();
    void setBribed();
    bool getBribed();
    virtual std::wstring getName();

private:
    bool mBribed;
    BlockPos* mTreasurePos;
    int mTick;
    BlockPos* mStartPos;
    int mDistanceTravelled;
    int mMaxDistance;
    int mStuck;
    int mDirectionChanges;
    int mSearchRadius;
    float mSpeedModifier;
    PathfinderMob* mMob;
};

#endif

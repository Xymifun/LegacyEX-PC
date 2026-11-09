#ifndef GAME_ENTITY_AI_BREATHAIRGOAL_HPP
#define GAME_ENTITY_AI_BREATHAIRGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"

class BlockPos;
class Dolphin;

class BreathAirGoal : public Goal
{
public:
    BreathAirGoal(Dolphin* dolphin);
    virtual ~BreathAirGoal();

    BlockPos* findAirInArea(const BlockPos& minPos, const BlockPos& maxPos);
    void findAirPosition();
    virtual void start();
    virtual void tick();
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual bool canInterrupt();
    virtual std::wstring getName();

private:
    int mAirSupplyThreshold;
    Dolphin* mDolphin;
};

#endif

#ifndef GAME_ENTITY_AI_PANICGOAL_HPP
#define GAME_ENTITY_AI_PANICGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"

class Entity;
class Level;
class PathfinderMob;
class BlockPos;

class PanicGoal : public Goal
{
public:
    PanicGoal(PathfinderMob* mob, double speedModifier);
    virtual ~PanicGoal() {}

    BlockPos* lookForWater(Level* level, Entity* entity, int range, int verticalRange);
    bool findRandomPosition();
    virtual bool canUse();
    virtual void start();
    virtual bool canContinueToUse();
    virtual std::wstring getName() { return L"PanicGoal"; }

protected:
    PathfinderMob* mMob;
    double mSpeedModifier;
    double mWantedX;
    double mWantedY;
    double mWantedZ;
};

#endif

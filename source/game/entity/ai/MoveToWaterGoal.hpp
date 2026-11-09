#ifndef GAME_ENTITY_AI_MOVETOWATERGOAL_HPP
#define GAME_ENTITY_AI_MOVETOWATERGOAL_HPP

#include <string>

#include "game/entity/mob/PathfinderMob.hpp"
#include "game/world/block/MoveToBlockGoal.hpp"

class Entity;
class Level;

class MoveToWaterGoal : public MoveToBlockGoal
{
public:
    MoveToWaterGoal(PathfinderMob* mob, double speedModifier, int searchRange, int verticalSearchStart,
                    double maxStayDistance);
    virtual ~MoveToWaterGoal();

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName();
    virtual int GetNextStartTick(PathfinderMob* mob);
    virtual bool isValidTarget(Level* level, const BlockPos& pos);

private:
    double mSpeedModifierSquared;
    PathfinderMob* mEntity;
};

#endif

#ifndef GAME_ENTITY_AI_SINKINWATERRANDOMSTROLLGOAL_HPP
#define GAME_ENTITY_AI_SINKINWATERRANDOMSTROLLGOAL_HPP

#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"

class PathfinderMob;
class Vec3;

class SinkInWaterRandomStrollGoal : public WaterAvoidingRandomStrollGoal
{
public:
    SinkInWaterRandomStrollGoal(PathfinderMob* mob, double speedModifier);
    virtual ~SinkInWaterRandomStrollGoal();
    virtual void tick();
    virtual Vec3* getPosition();
};

#endif

#ifndef GAME_ENTITY_AI_WATERAVOIDINGRANDOMFLYINGGOAL_HPP
#define GAME_ENTITY_AI_WATERAVOIDINGRANDOMFLYINGGOAL_HPP

#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"

class PathfinderMob;
class Vec3;

class WaterAvoidingRandomFlyingGoal : public WaterAvoidingRandomStrollGoal
{
public:
    WaterAvoidingRandomFlyingGoal(PathfinderMob* mob, double speedModifier);
    virtual ~WaterAvoidingRandomFlyingGoal();

    Vec3* getTreePos();
    virtual Vec3* getPosition();
};

#endif

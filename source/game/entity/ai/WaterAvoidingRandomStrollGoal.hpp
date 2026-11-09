#ifndef GAME_ENTITY_AI_WATERAVOIDINGRANDOMSTROLLGOAL_HPP
#define GAME_ENTITY_AI_WATERAVOIDINGRANDOMSTROLLGOAL_HPP

#include "game/entity/ai/RandomStrollGoal.hpp"

class PathfinderMob;
class Vec3;

class WaterAvoidingRandomStrollGoal : public RandomStrollGoal
{
public:
    WaterAvoidingRandomStrollGoal(PathfinderMob* mob, double speedModifier);
    WaterAvoidingRandomStrollGoal(PathfinderMob* mob, double speedModifier, float probability);
    virtual ~WaterAvoidingRandomStrollGoal() {}
    virtual Vec3* getPosition();

    static const float PROBABILITY;

protected:
    float mProbability;
};

#endif

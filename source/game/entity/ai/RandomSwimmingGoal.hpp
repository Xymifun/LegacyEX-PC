#ifndef GAME_ENTITY_AI_RANDOMSWIMMINGGOAL_HPP
#define GAME_ENTITY_AI_RANDOMSWIMMINGGOAL_HPP

#include <string>

#include "game/entity/ai/RandomStrollGoal.hpp"

class MutableBlockPos;
class PathfinderMob;
class Vec3;

class RandomSwimmingGoal : public RandomStrollGoal
{
public:
    RandomSwimmingGoal(PathfinderMob* mob, double speedModifier, int interval, int xz, int y);
    virtual ~RandomSwimmingGoal();

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual std::wstring getName() { return L"RandomSwimming"; }
    virtual Vec3* getPosition();

    void getWaterHeights(MutableBlockPos pos, int& topY, float& waterY, float& minY, float& maxY);

private:
    int mXz;
    int mY;
};

#endif

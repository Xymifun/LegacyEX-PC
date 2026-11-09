#ifndef GAME_ENTITY_AI_FLEESUNGOAL_HPP
#define GAME_ENTITY_AI_FLEESUNGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"

class Level;
class PathfinderMob;
class Vec3;

class FleeSunGoal : public Goal
{
public:
    FleeSunGoal(PathfinderMob* mob, double speedModifier);
    virtual ~FleeSunGoal();

    Vec3* getHidePos();
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void setLevel(Level* level);
    virtual std::wstring getName() { return L"FleeSun"; }

private:
    PathfinderMob* mMob;
    double mWantedX;
    double mWantedY;
    double mWantedZ;
    double mSpeedModifier;
    Level* mLevel;
};

#endif

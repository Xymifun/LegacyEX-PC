#ifndef GAME_ENTITY_AI_CIRCLEAROUNDANCHORGOAL_HPP
#define GAME_ENTITY_AI_CIRCLEAROUNDANCHORGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"

class Mob;
class Vec3;

class CircleAroundAnchorGoal : public Goal
{
public:
    CircleAroundAnchorGoal(Mob* mob, float speedModifier, Goal::FloatRange distanceRange, int distanceInterval,
                           Goal::FloatRange heightRange, Goal::FloatRange verticalRange, int verticalInterval,
                           float minDistance);
    virtual ~CircleAroundAnchorGoal();
    virtual bool canUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName() { return L"CircleAroundAnchorGoal"; }

    float calculateHeightOffset() const;
    void setAnchorAboveTarget();
    void selectNext();
    bool touchingTarget() const;

private:
    Mob* mMob;
    float mSpeedModifier;
    Goal::FloatRange mDistanceRange;
    int mDistanceInterval;
    Goal::FloatRange mHeightRange;
    Goal::FloatRange mVerticalRange;
    int mVerticalInterval;
    float mMinDistanceSquared;
    Vec3* mAnchor;
    float mCurrentDistance;
    float mCurrentHeight;
    float mDirection;
    float mAngle;
};

#endif

#ifndef GAME_CORE_BODYCONTROL_HPP
#define GAME_CORE_BODYCONTROL_HPP

#include "game/entity/ai/Control.hpp"

class LivingEntity;

class BodyControl : public Control
{
public:
    BodyControl(LivingEntity* mob);

    virtual ~BodyControl() {}
    virtual void clientTick();

protected:
    float clamp(float base, float value, float maxDifference);

    LivingEntity* mMob;
    int mHeadStableTime;
    float mLastStableYHeadRot;
};

#endif

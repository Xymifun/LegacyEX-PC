#ifndef GAME_ENTITY_AI_JUMPGOAL_HPP
#define GAME_ENTITY_AI_JUMPGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"

class Dolphin;

class JumpGoal : public Goal
{
public:
    JumpGoal(Dolphin* mob, int interval);
    virtual ~JumpGoal();
    bool isObstacle();
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual bool canInterrupt();
    virtual void start();
    virtual void stop();
    float rotlerp(float from, float to, float amount);
    virtual void tick();
    virtual std::wstring getName();

private:
    int mInterval;
    Dolphin* mMob;
    bool mCanJump;
};

#endif

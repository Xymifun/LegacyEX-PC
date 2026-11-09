#ifndef GAME_ENTITY_AI_BREAKDOORGOAL_HPP
#define GAME_ENTITY_AI_BREAKDOORGOAL_HPP

#include <string>

#include "game/entity/ai/DoorInteractGoal.hpp"

class Mob;

class BreakDoorGoal : public DoorInteractGoal
{
public:
    BreakDoorGoal(Mob* mob);
    virtual ~BreakDoorGoal();
    virtual bool canUse();
    virtual void start();
    virtual void stop();
    virtual bool canContinueToUse();
    virtual void tick();
    virtual std::wstring getName() { return L"BreakDoor"; }

protected:
    int mBreakingTime;
    int mLastBreakProgress;
};

#endif

#ifndef GAME_ENTITY_AI_OPENDOORGOAL_HPP
#define GAME_ENTITY_AI_OPENDOORGOAL_HPP

#include <string>

#include "game/entity/ai/DoorInteractGoal.hpp"

class Mob;

class OpenDoorGoal : public DoorInteractGoal
{
public:
    OpenDoorGoal(Mob* mob, bool closeDoor);
    virtual ~OpenDoorGoal();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName() { return L"OpenDoor"; }

private:
    bool mCloseDoor;
    int mCloseDoorTimer;
};

#endif

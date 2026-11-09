#ifndef GAME_ENTITY_AI_RESTRICTOPENDOORGOAL_HPP
#define GAME_ENTITY_AI_RESTRICTOPENDOORGOAL_HPP

#include <boost/weak_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"

class DoorInfo;
class PathfinderMob;

class RestrictOpenDoorGoal : public Goal
{
public:
    RestrictOpenDoorGoal(PathfinderMob* mob);

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName() { return L"RestrictOpenDoor"; }

private:
    PathfinderMob* mMob;
    boost::weak_ptr<DoorInfo> mDoorInfo;
};

#endif

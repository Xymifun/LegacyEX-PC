#ifndef GAME_ENTITY_AI_MOVEINDOORSGOAL_HPP
#define GAME_ENTITY_AI_MOVEINDOORSGOAL_HPP

#include <boost/weak_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"
#include "game/core/DoorInfo.hpp"

class PathfinderMob;

class MoveIndoorsGoal : public Goal
{
public:
    MoveIndoorsGoal(PathfinderMob* mob);

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual std::wstring getName() { return L"MoveIndoors"; }

private:
    PathfinderMob* mMob;
    boost::weak_ptr<DoorInfo> mDoorInfo;
    int mDoorX;
    int mDoorZ;
};

#endif

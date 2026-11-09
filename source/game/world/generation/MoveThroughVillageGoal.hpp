#ifndef GAME_WORLD_GENERATION_MOVETHROUGHVILLAGEGOAL_HPP
#define GAME_WORLD_GENERATION_MOVETHROUGHVILLAGEGOAL_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/entity/ai/Goal.hpp"

class DoorInfo;
class PathfinderMob;
class Path;
class Village;

class MoveThroughVillageGoal : public Goal
{
public:
    MoveThroughVillageGoal(PathfinderMob* mob, double speedModifier, bool onlyAtNight);
    virtual ~MoveThroughVillageGoal();

    void updateVisited();
    bool hasVisited(boost::shared_ptr<DoorInfo> door);
    boost::shared_ptr<DoorInfo> getNextDoorInfo(boost::shared_ptr<Village> village);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual std::wstring getName() { return L"MoveThroughVillage"; }

private:
    PathfinderMob* mMob;
    double mSpeedModifier;
    Path* mPath;
    boost::weak_ptr<DoorInfo> mDoorInfo;
    bool mOnlyAtNight;
    std::vector<boost::weak_ptr<DoorInfo> > mVisited;
};

#endif

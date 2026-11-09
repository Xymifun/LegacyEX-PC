#ifndef GAME_ENTITY_AI_FOLLOWFLOCKLEADERGOAL_HPP
#define GAME_ENTITY_AI_FOLLOWFLOCKLEADERGOAL_HPP

#include <boost/shared_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"

class BlockPos;
class Mob;

class FollowFlockLeaderGoal : public Goal
{
public:
    FollowFlockLeaderGoal(Mob* mob, bool inWater);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    BlockPos GetTargetPosition(boost::shared_ptr<Mob> leader);
    virtual void tick();
    virtual std::wstring getName() { return L"FollowFlock"; }

private:
    int mInterval;
    Mob* mMob;
    boost::shared_ptr<Mob> mLeader;
    int mTimeToRecalcPath;
    bool mInWater;
};

#endif

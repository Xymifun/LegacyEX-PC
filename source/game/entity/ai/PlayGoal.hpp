#ifndef GAME_ENTITY_AI_PLAYGOAL_HPP
#define GAME_ENTITY_AI_PLAYGOAL_HPP

#include <boost/weak_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"
#include "game/util/Vec3.hpp"

class Villager;

class PlayGoal : public Goal
{
public:
    PlayGoal(Villager* villager, double speedModifier);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName() { return L"Play"; }

private:
    Villager* mVillager;
    boost::weak_ptr<Villager> mTarget;
    double mSpeedModifier;
    Vec3 mPlayPosition;
    int mPlayTime;
};

#endif

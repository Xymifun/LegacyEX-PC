#ifndef GAME_ENTITY_AI_LANDONOWNERSSHOULDERGOAL_HPP
#define GAME_ENTITY_AI_LANDONOWNERSSHOULDERGOAL_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"

class Player;
class ShoulderRidingEntity;

class LandOnOwnersShoulderGoal : public Goal
{
public:
    LandOnOwnersShoulderGoal(ShoulderRidingEntity* owner);

    virtual bool canUse();
    virtual bool canInterrupt();
    virtual void start();
    virtual void tick();
    virtual std::wstring getName() { return L"LandOnOwnersShoulderGoal"; }

private:
    ShoulderRidingEntity* mOwner;
    boost::weak_ptr<Player> mPlayer;
    bool mShoulderRiding;
};

#endif

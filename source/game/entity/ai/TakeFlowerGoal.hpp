#ifndef GAME_ENTITY_AI_TAKEFLOWERGOAL_HPP
#define GAME_ENTITY_AI_TAKEFLOWERGOAL_HPP

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/entity/ai/Goal.hpp"

class Villager;
class VillagerGolem;

class TakeFlowerGoal : public Goal
{
public:
    explicit TakeFlowerGoal(Villager* villager);

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName() { return L"TakeFlower"; }

private:
    Villager* mVillager;
    boost::weak_ptr<VillagerGolem> mGolem;
    int mTicks;
    bool mWantsToTakeFlower;
};

#endif

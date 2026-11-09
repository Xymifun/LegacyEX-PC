#ifndef GAME_ENTITY_AI_OFFERFLOWERGOAL_HPP
#define GAME_ENTITY_AI_OFFERFLOWERGOAL_HPP

#include <boost/weak_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"

class Villager;
class VillagerGolem;

class OfferFlowerGoal : public Goal
{
public:
    explicit OfferFlowerGoal(VillagerGolem* golem);

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName() { return L"OfferFlower"; }

private:
    VillagerGolem* mGolem;
    boost::weak_ptr<Villager> mTarget;
    int mTicks;
};

#endif

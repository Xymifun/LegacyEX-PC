#ifndef GAME_WORLD_GENERATION_TRADEWITHPLAYERGOAL_HPP
#define GAME_WORLD_GENERATION_TRADEWITHPLAYERGOAL_HPP

#include "game/entity/ai/Goal.hpp"

class Villager;

class TradeWithPlayerGoal : public Goal
{
public:
    TradeWithPlayerGoal(Villager* villager);
    virtual ~TradeWithPlayerGoal();
    virtual bool canUse();
    virtual void start();
    virtual void stop();
    virtual std::wstring getName() { return L"TradeWithPlayer"; }

private:
    Villager* mVillager;
};

#endif

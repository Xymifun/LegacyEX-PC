#ifndef GAME_WORLD_GENERATION_LOOKATTRADINGPLAYERGOAL_HPP
#define GAME_WORLD_GENERATION_LOOKATTRADINGPLAYERGOAL_HPP

#include "game/world/generation/LookAtPlayerGoal.hpp"

class Villager;

class LookAtTradingPlayerGoal : public LookAtPlayerGoal
{
public:
    LookAtTradingPlayerGoal(Villager* villager);
    virtual ~LookAtTradingPlayerGoal() {}
    virtual bool canUse();
    virtual std::wstring getName() { return L"LookAtTradingPlayer"; }

private:
    Villager* mVillager;
};

#endif

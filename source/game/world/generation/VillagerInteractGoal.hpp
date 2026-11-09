#ifndef GAME_WORLD_GENERATION_VILLAGERINTERACTGOAL_HPP
#define GAME_WORLD_GENERATION_VILLAGERINTERACTGOAL_HPP

#include "game/entity/ai/InteractGoal.hpp"

class Villager;

class VillagerInteractGoal : public InteractGoal
{
public:
    explicit VillagerInteractGoal(Villager* villager);
    virtual void start();
    virtual void tick();
    virtual std::wstring getName() { return L"VillagerInteract"; }

private:
    int mInteractionDelay;
    Villager* mVillager;
};

#endif

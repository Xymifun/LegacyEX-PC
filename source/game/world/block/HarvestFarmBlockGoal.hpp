#ifndef GAME_WORLD_BLOCK_HARVESTFARMBLOCKGOAL_HPP
#define GAME_WORLD_BLOCK_HARVESTFARMBLOCKGOAL_HPP

#include <string>

#include "game/world/block/MoveToBlockGoal.hpp"

class BlockPos;
class Level;
class Villager;

class HarvestFarmBlockGoal : public MoveToBlockGoal
{
public:
    HarvestFarmBlockGoal(Villager* villager, double speedModifier);
    virtual ~HarvestFarmBlockGoal();

    bool fjGetNextFarmTarget();
    virtual bool canUse();
    virtual bool canContinueToUse();
    int findInventorySlotForFarmSeeds();
    virtual void tick();
    virtual bool isValidTarget(Level* level, const BlockPos& pos);
    virtual std::wstring getName() { return L"HarvestFarmBlock"; }

private:
    Villager* mVillager;
    bool mHasFarmSeeds;
    bool mWantsMoreFood;
    int mTargetType;
    int mUnknown;
};

#endif

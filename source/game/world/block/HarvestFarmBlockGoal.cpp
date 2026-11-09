#include "game/world/block/HarvestFarmBlockGoal.hpp"
#include "game/world/block/Material.hpp"

#include "game/entity/ai/LookControl.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/SimpleContainer.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/CropBlock.hpp"
#include "game/world/generation/Villager.hpp"

HarvestFarmBlockGoal::HarvestFarmBlockGoal(Villager* villager, double speedModifier)
    : MoveToBlockGoal(villager, speedModifier, 16, 1, 1.0),
      mVillager(villager),
      mHasFarmSeeds(false),
      mWantsMoreFood(false),
      mTargetType(0)
{
}

HarvestFarmBlockGoal::~HarvestFarmBlockGoal()
{
}

bool HarvestFarmBlockGoal::fjGetNextFarmTarget()
{
    mTargetType = -1;
    mHasFarmSeeds = mVillager->hasFarmSeeds();
    mWantsMoreFood = mVillager->wantsMoreFood();
    if (!findNearestBlock())
    {
        return false;
    }
    return isValidTarget(mVillager->mLevel, mBlockPos);
}

bool HarvestFarmBlockGoal::canUse()
{
    if (!mVillager->mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
    {
        return false;
    }
    if (mNextStartTick > 0)
    {
        --mNextStartTick;
        return false;
    }
    mNextStartTick = mVillager->getRandom()->nextInt(200);
    return fjGetNextFarmTarget();
}

bool HarvestFarmBlockGoal::canContinueToUse()
{
    return mTargetType >= 0 && MoveToBlockGoal::canContinueToUse();
}

int HarvestFarmBlockGoal::findInventorySlotForFarmSeeds()
{
    SimpleContainer* inventory = mVillager->getInventory();
    int size = inventory->getContainerSize();
    for (int slot = 0; slot < size; ++slot)
    {
        not_null_ptr<ItemInstance> item = inventory->getItem(slot);
        if (!item->isEmpty())
        {
            Item* itemType = item->getItem();
            if (itemType != NULL)
            {
                if (itemType == Items::wheatSeeds)
                {
                    return slot;
                }
                if (itemType == Items::potato)
                {
                    return slot;
                }
                if (itemType == Items::carrot)
                {
                    return slot;
                }
                if (itemType == Items::beetrootSeeds)
                {
                    return slot;
                }
            }
        }
    }
    return -1;
}

void HarvestFarmBlockGoal::tick()
{
    MoveToBlockGoal::tick();
    mVillager->getLookControl()->setLookAt((double)mBlockPos.x + 0.5, (double)mBlockPos.y + 1.0,
                                           (double)mBlockPos.z + 0.5, 10.0f, mVillager->getMaxHeadXRot());

    if (!isReachedTarget())
    {
        return;
    }

    Level* level = mVillager->mLevel;
    BlockPos above = mBlockPos.above();
    const BlockState* state = level->getBlockState(above);
    CropBlock* crop = dynamic_cast<CropBlock*>(state->getBlock());

    if (mTargetType == 0 && crop != NULL && crop->isMaxAge(state))
    {
        level->destroyBlock(above, true);
    }
    else if (mTargetType == 1 && state->getMaterial() == Material::AIR)
    {
        int slot = findInventorySlotForFarmSeeds();
        if (slot >= 0)
        {
            SimpleContainer* inventory = mVillager->getInventory();
            not_null_ptr<ItemInstance> item = inventory->getItem(slot);
            if (!item->isEmpty())
            {
                if (item->getItem() == Items::wheatSeeds)
                {
                    level->setBlock(above, Blocks::wheat->defaultBlockState(), Block::UPDATE_ALL, false);
                }
                else if (item->getItem() == Items::potato)
                {
                    level->setBlock(above, Blocks::potatoes->defaultBlockState(), Block::UPDATE_ALL, false);
                }
                else if (item->getItem() == Items::carrot)
                {
                    level->setBlock(above, Blocks::carrots->defaultBlockState(), Block::UPDATE_ALL, false);
                }
                else if (item->getItem() == Items::beetrootSeeds)
                {
                    level->setBlock(above, Blocks::beetroots->defaultBlockState(), Block::UPDATE_ALL, false);
                }
            }
            item->shrink(1);
            if (item->isEmpty())
            {
                inventory->setItem(slot, ItemInstance::EMPTY);
            }
        }
        else
        {
            mTargetType = -1;
            mNextStartTick = 0;
            return;
        }
        if (fjGetNextFarmTarget())
        {
            start();
            return;
        }
    }

    mTargetType = -1;
    mNextStartTick = 10;
}

bool HarvestFarmBlockGoal::isValidTarget(Level* level, const BlockPos& pos)
{
    BlockPos target = pos;
    if (level->getBlockState(pos)->getBlock() == Blocks::farmland)
    {
        target = target.above();
        const BlockState* state = level->getBlockState(target);
        CropBlock* crop = dynamic_cast<CropBlock*>(state->getBlock());
        if (mHasFarmSeeds && state->getMaterial() == Material::AIR)
        {
            mTargetType = 1;
            return true;
        }
        if (mWantsMoreFood && crop != NULL && (mTargetType == 0 || mTargetType == -1) && crop->isMaxAge(state))
        {
            mTargetType = 0;
            return true;
        }
    }
    return false;
}

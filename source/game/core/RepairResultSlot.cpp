#include "game/core/RepairResultSlot.hpp"
#include "game/world/LevelEvent.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/item/RepairContainer.hpp"
#include "game/ui/AnvilMenu.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/AnvilBlock.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

RepairResultSlot::RepairResultSlot(AnvilMenu* menu, const BlockPos& position, boost::shared_ptr<Container> container,
                                   int index, int x, int y)
    : Slot(container, index, x, y)
{
    mMenu = menu;
    mPosition = position;
}

bool RepairResultSlot::mayPlace(not_null_ptr<ItemInstance>)
{
    return false;
}

bool RepairResultSlot::mayPickup(boost::shared_ptr<Player> player)
{
    return (player->mAbilities.mInstabuild || player->mExperienceLevel >= mMenu->mRepairCost) && mMenu->mRepairCost > 0
           && hasItem();
}

not_null_ptr<ItemInstance> RepairResultSlot::onTake(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> item)
{
    if (!player->mAbilities.mInstabuild)
    {
        player->giveExperienceLevels(-mMenu->mRepairCost);
        if (!item->isEmpty())
        {
            player->OnTakeFromAnvil(item);
        }
    }

    mMenu->mInputSlots->setItem(0, ItemInstance::EMPTY);
    if (mMenu->mMaterialCost < 1)
    {
        mMenu->mInputSlots->setItem(1, ItemInstance::EMPTY);
    }
    else
    {
        not_null_ptr<ItemInstance> material = mMenu->mInputSlots->getItem(1);
        if (!material->isEmpty() && material->getCount() > mMenu->mMaterialCost)
        {
            material->shrink(mMenu->mMaterialCost);
            mMenu->mInputSlots->setItem(1, material);
        }
        else
        {
            mMenu->mInputSlots->setItem(1, ItemInstance::EMPTY);
        }
    }

    mMenu->mRepairCost = 0;

    const BlockState* state = mMenu->mLevel->getBlockState(mPosition);
    if (!player->mAbilities.mInstabuild && !mMenu->mLevel->mIsClientSide && state->getBlock() == Blocks::anvil
        && player->getRandom()->nextFloat() < 0.12f)
    {
        int damage = state->getValue(AnvilBlock::DAMAGE) + 1;
        if (damage > 2)
        {
            mMenu->mLevel->removeBlock(mPosition);
            mMenu->mLevel->levelEvent(LevelEvent::SOUND_ANVIL_BROKEN, mPosition, 0);
        }
        else
        {
            TypedBoxed<int> damageValue(damage);
            const BlockState* damagedState = state->setBoxedValue(AnvilBlock::DAMAGE, &damageValue);
            mMenu->mLevel->setBlock(mPosition, damagedState, Block::UPDATE_CLIENTS, false);
            mMenu->mLevel->levelEvent(LevelEvent::SOUND_ANVIL_USED, mPosition, 0);
        }
    }
    else if (!mMenu->mLevel->mIsClientSide)
    {
        mMenu->mLevel->levelEvent(LevelEvent::SOUND_ANVIL_USED, mPosition, 0);
    }

    return item;
}

bool RepairResultSlot::mayCombine(not_null_ptr<ItemInstance>)
{
    return false;
}

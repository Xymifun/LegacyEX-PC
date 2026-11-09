#include "game/world/block/RedstoneLampBlock.hpp"

#include "game/core/IconRegister.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

RedstoneLampBlock::RedstoneLampBlock(bool lit) : Block(Material::BUILDABLE_GLASS)
{
    DerivedInit();
    mLit = lit;
    if (lit)
        setLightEmission(1.0f);
}

void RedstoneLampBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!level->mIsClientSide)
    {
        if (mLit)
        {
            if (!level->hasNeighborSignal(pos))
            {
                level->setBlock(pos, Blocks::redstone_lamp->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                return;
            }
            if (mLit)
                return;
        }
        if (level->hasNeighborSignal(pos))
            level->setBlock(pos, Blocks::lit_redstone_lamp->defaultBlockState(), Block::UPDATE_CLIENTS, false);
    }
}

void RedstoneLampBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                        const BlockPos& fromPos)
{
    if (!level->mIsClientSide)
    {
        if (mLit)
        {
            if (!level->hasNeighborSignal(pos))
            {
                level->addToTickNextTick(pos, this, 4);
                return;
            }
            if (mLit)
                return;
        }
        if (level->hasNeighborSignal(pos))
            level->setBlock(pos, Blocks::lit_redstone_lamp->defaultBlockState(), Block::UPDATE_ALL, false);
    }
}

void RedstoneLampBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (!level->mIsClientSide && mLit && !level->hasNeighborSignal(pos))
        level->setBlock(pos, Blocks::redstone_lamp->defaultBlockState(), Block::UPDATE_ALL, false);
}

Item* RedstoneLampBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return Item::byBlock(Blocks::redstone_lamp);
}

not_null_ptr<ItemInstance> RedstoneLampBlock::getCloneItemInstance(Level* level, const BlockPos& pos,
                                                                   const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(Blocks::redstone_lamp)));
}

not_null_ptr<ItemInstance> RedstoneLampBlock::getSilkTouchItemInstance(const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::redstone_lamp));
}

void RedstoneLampBlock::registerIcons(IconRegister* iconRegister)
{
    if (mLit)
    {
        mIcon = iconRegister->registerIcon(L"redstoneLight_lit");
    }
    else
    {
        mIcon = iconRegister->registerIcon(L"redstoneLight");
    }
}

#include "game/world/block/StaticLiquidBlock.hpp"

#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/util/Random.hpp"

StaticLiquidBlock::StaticLiquidBlock(Material* material) : LiquidBlock(material)
{
    LiquidBlock::DerivedInit();
    Block::setTicking(false);
    if (material == Material::LAVA)
    {
        Block::setTicking(true);
    }
}

void StaticLiquidBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                        const BlockPos& fromPos)
{
    LiquidBlock::neighborChanged(state, level, pos, block, fromPos);
    if (Block::byId(LiquidBlock::getLiquidElement(level, pos) >> 4) == this)
    {
        setDynamic(level, pos, state);
    }
}

void StaticLiquidBlock::setDynamic(Level* level, const BlockPos& pos, const BlockState* state)
{
    const bool remove = level->getBlockState(pos)->getMaterial() != getCraftingMaterial();
    const unsigned short elementId = LiquidBlock::getLiquidElement(level, pos);
    LiquidBlock::setLiquidElement(
        level, pos, static_cast<unsigned short>(LiquidBlock::getDynamicLiquidElementId(elementId)), remove, 18);
    LiquidBlock* dynamicLiquid = LiquidBlock::getDynamicLiquid(getCraftingMaterial());
    level->addToTickNextTick(pos, dynamicLiquid, getTickDelay(level));
}

void StaticLiquidBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (getCraftingMaterial() == Material::LAVA)
    {
        LiquidBlock::trySpreadFire(level, pos, random);
    }
}

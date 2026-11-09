#include "game/world/block/SoulSandBlock.hpp"

#include "game/core/BubbleColumnSegment.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

static const AABB* SOUL_SAND_CLIP_AABB = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.875, 1.0);

SoulSandBlock::SoulSandBlock() : Block(Material::SAND, MaterialColor::COLOR_BROWN)
{
    DerivedInit();
    setTicking(true);
}

const AABB* SoulSandBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return SOUL_SAND_CLIP_AABB;
}

void SoulSandBlock::entityInside(Level*, const BlockPos&, const BlockState*, const boost::shared_ptr<Entity>& entity)
{
    entity->mXd *= 0.4;
    entity->mZd *= 0.4;
}

void SoulSandBlock::tick(Level* level, const BlockPos& pos, const BlockState*, Random*)
{
    Block* blockAbove = level->getBlock(pos.above().above());
    if (level->isEmptyWaterBlock(pos.above()) && blockAbove == Blocks::air)
    {
        int liquidLevel = level->getLiquidBlockState(pos.above())->getValue(LiquidBlock::LEVEL);
        if (!level->isEmptyWaterBlock(pos.above()))
            return;
        if (liquidLevel != 0)
            return;
    }

    static_cast<BubbleColumnSegment*>(Blocks::bubble_column_segment)->addBubbleColumnSegment(level, pos);
}

void SoulSandBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*,
                                    const BlockPos& fromPos)
{
    if (fromPos == pos.above() && level->isEmptyWaterBlock(pos.above()))
        level->addToTickNextTick(pos, this, level->mRandom->nextInt(2, 3));
}

void SoulSandBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    Block* blockAbove = level->getBlock(pos.above().above());
    if (level->isEmptyWaterBlock(pos.above()) && blockAbove == Blocks::air)
    {
        int liquidLevel = level->getLiquidBlockState(pos.above())->getValue(LiquidBlock::LEVEL);
        if (!level->isEmptyWaterBlock(pos.above()))
            return;
        if (liquidLevel != 0)
            return;
    }

    static_cast<BubbleColumnSegment*>(Blocks::bubble_column_segment)->addBubbleColumnSegment(level, pos);
}

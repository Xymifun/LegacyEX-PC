#include "game/world/block/MagmaBlock.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/core/BubbleColumnSegment.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/IceBlock.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

MagmaBlock::MagmaBlock() : Block(Material::STONE)
{
    DerivedInit();
    setLightEmission(0.2f);
    setTicking(true);
}

const MaterialColor* MagmaBlock::getMapColor(const BlockState*, LevelSource*, const BlockPos&)
{
    return MaterialColor::NETHER;
}

void MagmaBlock::stepOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity)
{
    if (!entity->isFireImmune() && eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType())
        && !EnchantmentHelper::hasFrostWalker(boost::static_pointer_cast<LivingEntity>(entity)))
    {
        entity->hurt(DamageSource::HOT_FLOOR, 1.0f);
    }

    Block::stepOn(level, pos, entity);
}

int MagmaBlock::getLightColor(const BlockState*, LevelSource*, const BlockPos&, int)
{
    return 0xf000f0;
}

void MagmaBlock::addToTickQueue(Level* level, const BlockPos& pos, Random* random)
{
    level->addToTickNextTick(pos, this, getTickDelay(level) + random->nextInt(getTickDelay(level)));
}

void MagmaBlock::tick(Level* level, const BlockPos& pos, const BlockState*, Random* random)
{
    Block* blockAbove = level->getBlock(pos.above().above());
    bool emptyWater = level->isEmptyWaterBlock(pos.above());
    if (emptyWater && blockAbove == Blocks::air)
    {
        int liquidLevel = level->getLiquidBlockState(pos.above())->getValue(LiquidBlock::LEVEL);
        if (!level->isEmptyWaterBlock(pos.above()) || liquidLevel != 0)
            return;
    }

    BubbleColumnSegment* bubbleColumnSegment = static_cast<BubbleColumnSegment*>(Blocks::bubble_column_segment);
    if (!bubbleColumnSegment->addBubbleColumnSegment(level, pos))
    {
        BlockPos above = pos.above();
        Block* block = level->getBlock(above);
        bool ice = block == Blocks::ice;
        if (block == Blocks::snow_layer || ice)
        {
            if (random->nextInt(8) == 0 && ice)
                IceBlock::melt(level, above);
            addToTickQueue(level, pos, random);
        }
    }
}

void MagmaBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block*, const BlockPos& fromPos)
{
    if (fromPos == pos.above())
    {
        if (level->isEmptyWaterBlock(pos.above()))
            level->addToTickNextTick(pos, this, level->mRandom->nextInt(2, 3));
    }

    Block* block = level->getBlock(fromPos);
    bool ice = block == Blocks::ice;
    if (block == Blocks::water || block == Blocks::flowing_water || block == Blocks::snow_layer || ice)
        addToTickQueue(level, pos, level->mRandom);
}

int MagmaBlock::getTickDelay(Level*)
{
    return 160;
}

void MagmaBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
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

bool MagmaBlock::isValidSpawn(const BlockState*, boost::shared_ptr<Entity> entity)
{
    return entity->isFireImmune();
}

void MagmaBlock::animateTick(const BlockState*, Level* level, const BlockPos& pos, Random* random)
{
    if (!level->isRainingAt(pos.above()))
        return;

    for (int i = 0; i < 4; ++i)
    {
        double x = pos.x + random->nextDouble();
        double y = pos.above().y;
        double z = pos.z + random->nextDouble();
        level->addParticle(PARTICLE_SMOKE, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
    }
}

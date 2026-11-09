#include "game/world/block/WaterlilyBlock.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"

static const AABB* LILY_PAD_AABB = AABB::newPermanent(0.0625, 0.0, 0.0625, 0.9375, 0.09375, 0.9375);

WaterlilyBlock::WaterlilyBlock() : Bush()
{
    DerivedInit();
}

void WaterlilyBlock::addCollisionAABBs(const BlockState*, Level*, const BlockPos& pos, const AABB* entityBox,
                                       std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity, bool)
{
    if (entity && entity->GetType() != EntityType::BOAT)
    {
        Block::addCollisionAABB(pos, entityBox, collidingBoxes, LILY_PAD_AABB);
    }
}

void WaterlilyBlock::entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                                  const boost::shared_ptr<Entity>& entity)
{
    Block::entityInside(level, pos, state, entity);
    if (eTYPE_DERIVED_FROM(EntityType::BOAT, entity->GetType()))
    {
        level->destroyBlock(pos, true);
    }
}

int WaterlilyBlock::getColor() const
{
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Tile_WaterLily);
}

int WaterlilyBlock::getColor(const BlockState*)
{
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Tile_WaterLily);
}

int WaterlilyBlock::getColor(LevelSource*, const BlockPos&)
{
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Tile_WaterLily);
}

int WaterlilyBlock::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState*)
{
    return getColor(levelSource, pos);
}

const AABB* WaterlilyBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return LILY_PAD_AABB;
}

bool WaterlilyBlock::mayPlaceOn(Level*, const BlockPos&, const BlockState* state)
{
    return state->getBlock() == Blocks::water || state->getMaterial() == Material::ICE;
}

bool WaterlilyBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    if (pos.y < 0 || pos.y > 255)
    {
        return false;
    }

    const BlockState* belowState = level->getBlockState(pos.below());
    Material* material = belowState->getMaterial();
    return (material == Material::WATER && belowState->getValue(LiquidBlock::LEVEL) == 0) || material == Material::ICE;
}

int WaterlilyBlock::convertBlockStateToLegacyData(const BlockState*)
{
    return 0;
}

bool WaterlilyBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

EBlockRenderShape WaterlilyBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_LILYPAD;
}

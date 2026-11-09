#include "game/world/block/GrassPathBlock.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemId.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

static const AABB* gGrassPathShape = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.9375, 1.0);

GrassPathBlock::GrassPathBlock() : Block(Material::DIRT)
{
    DerivedInit();
    setLightBlock(0xff);
}

bool GrassPathBlock::shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                      const Direction* direction)
{
    switch (direction->get3DDataValue())
    {
    case Direction::eDirection_Up:
        return true;
    case Direction::eDirection_North:
    case Direction::eDirection_South:
    case Direction::eDirection_West:
    case Direction::eDirection_East:
    {
        const BlockState* neighbourState = levelSource->getBlockState(pos.relative(direction));
        Block* neighbour = neighbourState->getBlock();
        return !(neighbourState->isSolidRender() || neighbour == Blocks::farmland || neighbour == Blocks::grass_path);
    }
    default:
        return Block::shouldRenderFace(state, levelSource, pos, direction);
    }
}

void GrassPathBlock::updateBlockState(Level* level, const BlockPos& pos)
{
    const BlockState* aboveState = level->getBlockState(pos.above());
    if (aboveState->getMaterial()->isSolid())
    {
        level->setBlockAndUpdate(pos, Blocks::dirt->defaultBlockState());
    }
}

void GrassPathBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onPlace(level, pos, state);
    updateBlockState(level, pos);
}

bool GrassPathBlock::isSilkTouchable()
{
    return true;
}

const AABB* GrassPathBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return gGrassPathShape;
}

bool GrassPathBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool GrassPathBlock::isCubeShaped(const BlockState*)
{
    return false;
}

Item* GrassPathBlock::getResource(const BlockState*, Random* random, int lootBonus)
{
    return Blocks::dirt->getResource(
        Blocks::dirt->defaultBlockState()->setValue(DirtBlock::VARIANT, DirtBlock::Variant::DIRT), random, lootBonus);
}

not_null_ptr<ItemInstance> GrassPathBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(this));
}

void GrassPathBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                     const BlockPos& fromPos)
{
    Block::neighborChanged(state, level, pos, block, fromPos);
    updateBlockState(level, pos);
}

EBlockFaceShape GrassPathBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&,
                                                  const Direction* direction)
{
    return direction == Direction::DOWN ? BLOCK_FACE_SOLID : BLOCK_FACE_UNDEFINED;
}

unsigned int GrassPathBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    switch (Item::getId(tooltipData.m_pItemInstance->getItem()))
    {
    case ItemId::WOODEN_HOE:
    case ItemId::STONE_HOE:
    case ItemId::IRON_HOE:
    case ItemId::DIAMOND_HOE:
    case ItemId::GOLDEN_HOE:
        if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success
            && tooltipData.m_bCanPlace)
        {
            return IDS_TOOLTIPS_TILL;
        }
        break;
    }
    return Block::GetInteractTooltip(tooltipData);
}

bool GrassPathBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

TextureAtlasSprite* GrassPathBlock::getTexture(const Direction* direction, const BlockState*)
{
    if (direction == Direction::UP)
    {
        return mTopIcon;
    }
    if (direction == Direction::DOWN)
    {
        return Blocks::dirt->getTexture(direction);
    }
    return mIcon;
}

TextureAtlasSprite* GrassPathBlock::getTexture(LevelSource*, const BlockPos&, const Direction* direction,
                                               const BlockState*)
{
    if (direction == Direction::UP)
    {
        return mTopIcon;
    }
    if (direction == Direction::DOWN)
    {
        return Blocks::dirt->getTexture(direction);
    }
    return mIcon;
}

void GrassPathBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"grass_path_side");
    mTopIcon = iconRegister->registerIcon(L"grass_path_top");
}

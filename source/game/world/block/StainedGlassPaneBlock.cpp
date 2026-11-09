#include "game/world/block/Predicates.hpp"
#include "game/world/block/StainedGlassPaneBlock.hpp"

#include <typeinfo>
#include <vector>
#include <boost/unordered_set.hpp>

#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BeaconBlock.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<const DyeColor*>* StainedGlassPaneBlock::COLOR = NULL;
TextureAtlasSprite* gStainedGlassPaneIcons[16];
TextureAtlasSprite* gStainedGlassPaneEdges[16];

void StainedGlassPaneBlock::blockStaticCtor()
{
    COLOR = EnumProperty<const DyeColor*>::create(L"color", typeid(DyeColor), DyeColor::COLORS,
                                                  Predicates<const DyeColor*>::alwaysTrue);
}

StainedGlassPaneBlock::StainedGlassPaneBlock() : ThinFenceBlock(L"glass", L"glass_pane_top", Material::GLASS, false)
{
    ClearDerivedInit();
    DerivedInit();
    registerDefaultState(mStateDefinition->any()
                             ->setValue(THIN_FENCE_NORTH, false)
                             ->setValue(THIN_FENCE_EAST, false)
                             ->setValue(THIN_FENCE_SOUTH, false)
                             ->setValue(THIN_FENCE_WEST, false)
                             ->setValue(COLOR, DyeColor::WHITE));
}

int StainedGlassPaneBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(COLOR)->getBlockData();
}

const MaterialColor* StainedGlassPaneBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    return MaterialColor::getDyeMaterial(state->getValue(COLOR));
}

int StainedGlassPaneBlock::getRenderLayer()
{
    return 1;
}

const BlockState* StainedGlassPaneBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState();
    return state->setValue(COLOR, DyeColor::byBlockData(data));
}

int StainedGlassPaneBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(COLOR)->getBlockData();
}

const BlockState* StainedGlassPaneBlock::rotate(const BlockState* state, Rotation* rotation)
{
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise180:
        return state->setValue(THIN_FENCE_NORTH, state->getValue(THIN_FENCE_SOUTH))
            ->setValue(THIN_FENCE_EAST, state->getValue(THIN_FENCE_WEST))
            ->setValue(THIN_FENCE_SOUTH, state->getValue(THIN_FENCE_NORTH))
            ->setValue(THIN_FENCE_WEST, state->getValue(THIN_FENCE_EAST));
    case Rotation::eTYPE_CounterClockwise90:
        return state->setValue(THIN_FENCE_NORTH, state->getValue(THIN_FENCE_EAST))
            ->setValue(THIN_FENCE_EAST, state->getValue(THIN_FENCE_SOUTH))
            ->setValue(THIN_FENCE_SOUTH, state->getValue(THIN_FENCE_WEST))
            ->setValue(THIN_FENCE_WEST, state->getValue(THIN_FENCE_NORTH));
    case Rotation::eTYPE_Clockwise90:
        return state->setValue(THIN_FENCE_NORTH, state->getValue(THIN_FENCE_WEST))
            ->setValue(THIN_FENCE_EAST, state->getValue(THIN_FENCE_NORTH))
            ->setValue(THIN_FENCE_SOUTH, state->getValue(THIN_FENCE_EAST))
            ->setValue(THIN_FENCE_WEST, state->getValue(THIN_FENCE_SOUTH));
    default:
        return state;
    }
}

const BlockState* StainedGlassPaneBlock::mirror(const BlockState* state, Mirror* mirror)
{
    switch (mirror->getType())
    {
    case Mirror::eTYPE_LeftRight:
        return state->setValue(THIN_FENCE_NORTH, state->getValue(THIN_FENCE_SOUTH))
            ->setValue(THIN_FENCE_SOUTH, state->getValue(THIN_FENCE_NORTH));
    case Mirror::eTYPE_FrontBack:
        return state->setValue(THIN_FENCE_EAST, state->getValue(THIN_FENCE_WEST))
            ->setValue(THIN_FENCE_WEST, state->getValue(THIN_FENCE_EAST));
    default:
        return ThinFenceBlock::mirror(state, mirror);
    }
}

BlockStateDefinition* StainedGlassPaneBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {THIN_FENCE_NORTH, THIN_FENCE_EAST, THIN_FENCE_WEST, THIN_FENCE_SOUTH, COLOR};
    return new BlockStateDefinition(this, properties, 5);
}

void StainedGlassPaneBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    if (level->mIsClientSide)
        return;
    BeaconBlock::updateValidityWithinBeam(level, pos);
}

void StainedGlassPaneBlock::onRemove(Level* level, const BlockPos& pos, const BlockState*)
{
    if (level->mIsClientSide)
        return;
    BeaconBlock::updateValidityWithinBeam(level, pos);
}

TextureAtlasSprite* StainedGlassPaneBlock::getIconTexture(const Direction*, const BlockState* state)
{
    return gStainedGlassPaneIcons[state->getValue(COLOR)->getBlockData()];
}

TextureAtlasSprite* StainedGlassPaneBlock::getEdgeTexture(const BlockState* state)
{
    return gStainedGlassPaneEdges[state->getValue(COLOR)->getBlockData()];
}

TextureAtlasSprite* StainedGlassPaneBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return getIconTexture(direction, state);
}

void StainedGlassPaneBlock::registerIcons(IconRegister* iconRegister)
{
    ThinFenceBlock::registerIcons(iconRegister);
    for (int data = 0; data < 16; ++data)
    {
        gStainedGlassPaneIcons[data] = iconRegister->registerIcon(
            getIconName() + L"_" + DyeColor::TEXTURE_NAMES[DyeColor::byBlockData(data)->getItemData()]);
        gStainedGlassPaneEdges[data] = iconRegister->registerIcon(
            getIconName() + L"_pane_top_" + DyeColor::TEXTURE_NAMES[DyeColor::byBlockData(data)->getItemData()]);
    }
}

TextureAtlasSprite* StainedGlassPaneBlock::getEdgeTexture()
{
    return getEdgeTexture(defaultBlockState());
}

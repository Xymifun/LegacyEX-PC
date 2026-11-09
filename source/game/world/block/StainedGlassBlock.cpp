#include "game/world/block/StainedGlassBlock.hpp"

#include <typeinfo>
#include <vector>

#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BeaconBlock.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<const DyeColor*>* StainedGlassBlock::COLOR = NULL;
TextureAtlasSprite* gStainedGlassBlockIcons[16] = {NULL};

void StainedGlassBlock::blockStaticCtor()
{
    COLOR = EnumProperty<const DyeColor*>::create(L"color", typeid(DyeColor), DyeColor::COLORS,
                                                  Predicates<const DyeColor*>::alwaysTrue);
}

StainedGlassBlock::StainedGlassBlock(Material* material) : HalfTransparentBlock(L"glass", material, false)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(COLOR, DyeColor::WHITE));
}

int StainedGlassBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(COLOR)->getBlockData();
}

const MaterialColor* StainedGlassBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    return MaterialColor::getDyeMaterial(state->getValue(COLOR));
}

int StainedGlassBlock::getRenderLayer()
{
    return 1;
}

int StainedGlassBlock::getResourceCount(Random*)
{
    return 0;
}

bool StainedGlassBlock::isSilkTouchable()
{
    return true;
}

bool StainedGlassBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

bool StainedGlassBlock::isCubeShaped(const BlockState*)
{
    return true;
}

bool StainedGlassBlock::isTransparent(const BlockState*)
{
    return true;
}

const BlockState* StainedGlassBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(COLOR, DyeColor::byBlockData(data));
}

void StainedGlassBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    if (level->mIsClientSide)
    {
        return;
    }

    BeaconBlock::updateValidityWithinBeam(level, pos);
}

void StainedGlassBlock::onRemove(Level* level, const BlockPos& pos, const BlockState*)
{
    if (level->mIsClientSide)
    {
        return;
    }

    BeaconBlock::updateValidityWithinBeam(level, pos);
}

int StainedGlassBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(COLOR)->getBlockData();
}

BlockStateDefinition* StainedGlassBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {COLOR};
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* StainedGlassBlock::getTexture(const Direction*, const BlockState* state)
{
    return gStainedGlassBlockIcons[state->getValue(COLOR)->blockData];
}

void StainedGlassBlock::registerIcons(IconRegister* iconRegister)
{
    for (int data = 0; data < 16; ++data)
    {
        gStainedGlassBlockIcons[data] = iconRegister->registerIcon(
            getIconName() + L"_" + DyeColor::TEXTURE_NAMES[DyeColor::byBlockData(data)->getItemData()]);
    }
}

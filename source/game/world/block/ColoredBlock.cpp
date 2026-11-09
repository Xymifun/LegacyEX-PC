#include "game/world/block/Predicates.hpp"
#include "game/world/block/ColoredBlock.hpp"

#include <typeinfo>
#include <vector>

#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<const DyeColor*>* ColoredBlock::COLOR = NULL;

void ColoredBlock::blockStaticCtor()
{
    COLOR = EnumProperty<const DyeColor*>::create(L"color", typeid(DyeColor), DyeColor::COLORS,
                                                  Predicates<const DyeColor*>::alwaysTrue);
}

ColoredBlock::ColoredBlock(Material* material) : Block(material)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(COLOR, DyeColor::WHITE));
}

int ColoredBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(COLOR)->getBlockData();
}

const MaterialColor* ColoredBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return MaterialColor::getDyeMaterial(state->getValue(COLOR));
}

const BlockState* ColoredBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(COLOR, DyeColor::byBlockData(data));
}

int ColoredBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(COLOR)->getBlockData();
}

TextureAtlasSprite* ColoredBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return mIcons[state->getValue(COLOR)->getBlockData()];
}

BlockStateDefinition* ColoredBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {COLOR};
    return new BlockStateDefinition(this, properties, 1);
}

void ColoredBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 16; ++i)
    {
        mIcons[i] = iconRegister->registerIcon(getIconName() + L"_"
                                               + DyeColor::TEXTURE_NAMES[DyeColor::byBlockData(i)->getItemData()]);
    }
}

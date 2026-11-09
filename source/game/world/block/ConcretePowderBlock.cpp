#include "game/world/block/Predicates.hpp"
#include "game/world/block/ConcretePowderBlock.hpp"

#include <typeinfo>
#include <vector>

#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Direction.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ColoredBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<const DyeColor*>* ConcretePowderBlock::COLOR = NULL;

void ConcretePowderBlock::blockStaticCtor()
{
    COLOR = EnumProperty<const DyeColor*>::create(L"color", typeid(DyeColor), DyeColor::COLORS,
                                                  Predicates<const DyeColor*>::alwaysTrue);
}

ConcretePowderBlock::ConcretePowderBlock() : FallingBlock(Material::SAND)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(COLOR, DyeColor::WHITE));
}

void ConcretePowderBlock::onLand(Level* level, const BlockPos& pos, const BlockState* state,
                                 const BlockState* replacedState)
{
    if (replacedState->getMaterial()->isLiquid())
    {
        level->setBlock(pos,
                        Blocks::concrete->defaultBlockState()->setValue(ColoredBlock::COLOR, state->getValue(COLOR)),
                        Block::UPDATE_ALL, false);
    }
}

unsigned int ConcretePowderBlock::tryTouchWater(Level* level, const BlockPos& pos, const BlockState* state)
{
    unsigned int touched = 0;
    for (std::vector<const Direction*>::iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end(); ++it)
    {
        const Direction* direction = *it;
        if (direction != Direction::DOWN)
        {
            BlockPos neighbor = pos.relative(direction);
            if (level->getBlockState(neighbor)->getMaterial() == Material::WATER)
            {
                touched = 1;
                break;
            }
        }
    }
    if (touched)
    {
        level->setBlock(pos,
                        Blocks::concrete->defaultBlockState()->setValue(ColoredBlock::COLOR, state->getValue(COLOR)),
                        Block::UPDATE_ALL, false);
    }
    return touched;
}

void ConcretePowderBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                          const BlockPos& fromPos)
{
    if (tryTouchWater(level, pos, state) == 0)
    {
        FallingBlock::neighborChanged(state, level, pos, block, fromPos);
    }
}

void ConcretePowderBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (tryTouchWater(level, pos, state) == 0)
    {
        FallingBlock::onPlace(level, pos, state);
    }
}

int ConcretePowderBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(COLOR)->getBlockData();
}

const MaterialColor* ConcretePowderBlock::getMapColor(const BlockState* state, LevelSource* levelSource,
                                                      const BlockPos& pos)
{
    return MaterialColor::getDyeMaterial(state->getValue(COLOR));
}

const BlockState* ConcretePowderBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(COLOR, DyeColor::byBlockData(data));
}

int ConcretePowderBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(COLOR)->getBlockData();
}

BlockStateDefinition* ConcretePowderBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {COLOR};
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* ConcretePowderBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return mIcons[state->getValue(COLOR)->getBlockData()];
}

void ConcretePowderBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 16; ++i)
    {
        mIcons[i] = iconRegister->registerIcon(getIconName() + L"_"
                                               + DyeColor::TEXTURE_NAMES[DyeColor::byBlockData(i)->getItemData()]);
    }
}

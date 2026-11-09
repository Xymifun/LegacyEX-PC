#include "game/core/Coral.hpp"
#include "game/core/StringIds.hpp"

#include <cstring>
#include <string>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/IntegerProperty.hpp"

AbstractProperty<int>* Coral::VARIANT = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"variant", 0, 4));

namespace
{
std::wstring CORAL_ICON_NAMES[5] = {L"coral_plant_blue", L"coral_plant_pink", L"coral_plant_purple", L"coral_plant_red",
                                    L"coral_plant_yellow"};

const AABB* CORAL_SHAPE = AABB::newPermanent(0.1f, 0.0, 0.1f, 0.9f, 0.8f, 0.9f);
}

Coral::Coral() : Bush(Material::REPLACEABLE_PLANT)
{
    DerivedInit();

    memset(mIcons, 0, sizeof(mIcons));
}

int Coral::getColor(const BlockState* state)
{
    return Blocks::grass->getColor(state);
}

int Coral::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    return Blocks::grass->getColor(levelSource, pos, state);
}

const MaterialColor* Coral::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return Blocks::grass->getMapColor(state, levelSource, pos);
}

Item* Coral::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return NULL;
}

not_null_ptr<ItemInstance> Coral::getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral, 1, convertBlockStateToLegacyData(state)));
}

bool Coral::canSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    return mayPlaceOn(level, pos.below(), level->getBlockState(pos.below()));
}

void Coral::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                            const BlockPos& fromPos)
{
    if (level->mIsClientSide)
    {
        return;
    }

    if (!level->isSolidBlockingBlock(pos.below()) && !level->getBlock(pos.below())->isSlabBlock())
    {
        level->setBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL, false);
    }

    level->addToTickNextTick(pos, this, 10);
}

TextureAtlasSprite* Coral::getTexture(const Direction* direction, const BlockState* state)
{
    return mIcons[state->getValue(Coral::VARIANT)];
}

void Coral::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 5; ++i)
    {
        mIcons[i] = iconRegister->registerIcon(CORAL_ICON_NAMES[i]);
    }
}

int Coral::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(Coral::VARIANT);
}

const BlockState* Coral::getBlockState(int data)
{
    return defaultBlockState()->setValue(Coral::VARIANT, data);
}

int Coral::getDescriptionId(int auxValue)
{
    switch (auxValue)
    {
    case 1:
        return IDS_TILE_CORAL_PINK;
    case 2:
        return IDS_TILE_CORAL_PURPLE;
    case 3:
        return IDS_TILE_CORAL_RED;
    case 4:
        return IDS_TILE_CORAL_YELLOW;
    default:
        return IDS_TILE_CORAL_BLUE;
    }
}

int Coral::getUseDescriptionId(int auxValue)
{
    return IDS_DESC_TILE_CORAL;
}

const AABB* Coral::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return CORAL_SHAPE;
}

BlockStateDefinition* Coral::createBlockStateDefinition()
{
    static const Property* PROPERTIES[1] = {Coral::VARIANT};

    return new BlockStateDefinition(this, PROPERTIES, 1);
}

bool Coral::isSilkTouchable()
{
    return true;
}

bool Coral::mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (level->isUnderWater(pos.above(), 263, 0))
    {
        if (level->getBlock(pos.above(2))->mMaterial != Material::AIR)
        {
            if (level->getLiquidBlock(pos)->mMaterial == Material::WATER)
            {
                int liquidLevel = level->getLiquidBlockState(pos)->getValue(LiquidBlock::LEVEL);
                if (liquidLevel > 0 && liquidLevel < 8)
                {
                    return false;
                }
            }

            Block* block = state->getBlock();
            bool onSlabTop = block->isSlabBlock()
                             && !HalfSlabBlock::isBottomSlab(block->convertBlockStateToLegacyData(state));

            if (state->canProvideSupport(Direction::UP, BLOCK_SUPPORT_CENTER) || onSlabTop)
            {
                return true;
            }
        }
    }

    return false;
}

void Coral::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (level->getLiquidBlock(pos)->mMaterial == mMaterial)
    {
        level->setExtraBlock(pos, Blocks::flowing_water->defaultBlockState(), Block::UPDATE_ALL, false);
    }
}

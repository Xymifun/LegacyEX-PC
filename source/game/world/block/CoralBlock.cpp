#include "game/world/block/CoralBlock.hpp"
#include "game/core/StringIds.hpp"

#include <cstring>
#include <cwchar>
#include <string>

#include "game/core/IconRegister.hpp"
#include "game/util/Math.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<int>* CoralBlock::VARIANT = static_cast<AbstractProperty<int>*>(
    IntegerProperty::create(L"variant", 0, 5));
AbstractProperty<bool>* CoralBlock::DEAD = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"dead_coral"));

static std::wstring ICON_NAMES[10] = {L"coral_blue",     L"coral_pink",       L"coral_purple",    L"coral_red",
                                      L"coral_yellow",   L"coral_blue_dead",  L"coral_pink_dead", L"coral_purple_dead",
                                      L"coral_red_dead", L"coral_yellow_dead"};

static const unsigned int LIVE_DESCRIPTION_IDS[4] = {0x3513E921U, 0xF7EF6A83U, 0x259DE196U, 0x919D90B9U};

static const int DEAD_DESCRIPTION_IDS[5] = {0x323B09A8U, 0xE8C04B12U, 0xCDCA46E0U, 0x30F2646DU, 0xEE954CBAU};

CoralBlock::CoralBlock() : Block(Material::STONE)
{
    DerivedInit();
    memset(mIcons, 0, sizeof(mIcons));

    registerDefaultState(mStateDefinition->any()->setValue(VARIANT, 0)->setValue(DEAD, false));
    setTicking(true);
}

Item* CoralBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(Blocks::coral_block);
}

not_null_ptr<ItemInstance> CoralBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_block, 1, convertBlockStateToLegacyData(state)));
}

void CoralBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (!level->mIsClientSide)
    {
        level->addToTickNextTick(pos, this, getTickDelay(level));
    }
}

void CoralBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (!level->mIsClientSide && !level->isTouchingMaterial(pos, Material::WATER)
        && level->isTouchingMaterial(pos, Material::AIR))
    {
        const BlockState* deadState = state->setValue(DEAD, true);
        level->setBlock(pos, deadState, Block::UPDATE_ALL, false);
    }
}

TextureAtlasSprite* CoralBlock::getTexture(const Direction*, const BlockState* state)
{
    int variant = state->getValue(VARIANT);
    if (variant > 4)
    {
        variant = 4;
    }
    if (state->getValue(DEAD))
    {
        variant += 5;
    }
    return mIcons[variant];
}

void CoralBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 10; i++)
    {
        mIcons[i] = iconRegister->registerIcon(ICON_NAMES[i]);
    }
}

int CoralBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(VARIANT);
    if (state->getValue(DEAD))
    {
        data |= 8;
    }
    return data;
}

const BlockState* CoralBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState()->setValue(VARIANT, data & 7);
    return state->setValue(DEAD, (data >> 3 & 1) != 0);
}

const BlockState* CoralBlock::setPropertyValueFromString(const BlockState* state, const Property* property,
                                                         const std::wstring& value)
{
    if (property == VARIANT && value.compare(L"dead_coral") == 0)
    {
        return state->setValue(DEAD, true)->setValue(VARIANT, Math::randomInt(5));
    }
    return Block::setPropertyValueFromString(state, property, value);
}

int CoralBlock::getDescriptionId(int auxValue)
{
    switch (auxValue)
    {
    case 1:
    case 2:
    case 3:
    case 4:
        return LIVE_DESCRIPTION_IDS[auxValue - 1];
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
        return DEAD_DESCRIPTION_IDS[auxValue - 8];
    default:
        return IDS_TILE_CORAL_BLOCK_BLUE;
    }
}

int CoralBlock::getUseDescriptionId(int auxValue)
{
    unsigned int aux = auxValue;
    switch (aux)
    {
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
        return IDS_DESC_TILE_CORAL_BLOCK_DEAD;
    default:
        return IDS_DESC_TILE_CORAL_BLOCK;
    }
}

int CoralBlock::getTickDelay(Level* level)
{
    return level->mRandom->nextInt(40, 200);
}

void CoralBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    if (!level->isTouchingMaterial(pos, Material::WATER) && level->isTouchingMaterial(pos, Material::AIR))
    {
        level->addToTickNextTick(pos, this, 45);
    }
}

BlockStateDefinition* CoralBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {DEAD, VARIANT};
    return new BlockStateDefinition(this, properties, 2);
}

int CoralBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT) | 8;
}

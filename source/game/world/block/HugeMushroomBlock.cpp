#include "game/world/block/HugeMushroomBlock.hpp"

#include <cstring>
#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::NORTH_WEST = new HugeMushroomBlock::Variant(1, L"north_west");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::NORTH = new HugeMushroomBlock::Variant(2, L"north");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::NORTH_EAST = new HugeMushroomBlock::Variant(3, L"north_east");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::WEST = new HugeMushroomBlock::Variant(4, L"west");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::CENTER = new HugeMushroomBlock::Variant(5, L"center");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::EAST = new HugeMushroomBlock::Variant(6, L"east");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::SOUTH_WEST = new HugeMushroomBlock::Variant(7, L"south_west");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::SOUTH = new HugeMushroomBlock::Variant(8, L"south");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::SOUTH_EAST = new HugeMushroomBlock::Variant(9, L"south_east");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::STEM = new HugeMushroomBlock::Variant(10, L"stem");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::ALL_INSIDE = new HugeMushroomBlock::Variant(0, L"all_inside");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::ALL_OUTSIDE = new HugeMushroomBlock::Variant(11,
                                                                                                     L"all_outside");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::ALL_STEM = new HugeMushroomBlock::Variant(12, L"all_stem");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::VALUES[13] = {
    HugeMushroomBlock::Variant::ALL_INSIDE, HugeMushroomBlock::Variant::NORTH_WEST,
    HugeMushroomBlock::Variant::NORTH,      HugeMushroomBlock::Variant::NORTH_EAST,
    HugeMushroomBlock::Variant::WEST,       HugeMushroomBlock::Variant::CENTER,
    HugeMushroomBlock::Variant::EAST,       HugeMushroomBlock::Variant::SOUTH_WEST,
    HugeMushroomBlock::Variant::SOUTH,      HugeMushroomBlock::Variant::SOUTH_EAST,
    HugeMushroomBlock::Variant::STEM,       HugeMushroomBlock::Variant::ALL_OUTSIDE,
    HugeMushroomBlock::Variant::ALL_STEM};
EnumProperty<HugeMushroomBlock::Variant*>* HugeMushroomBlock::VARIANT = NULL;
const std::wstring HugeMushroomBlock::TEXTURE_STEM = L"skin_stem";
const std::wstring HugeMushroomBlock::TEXTURE_INSIDE = L"inside";
const std::wstring HugeMushroomBlock::TEXTURE_TYPE[2] = {L"skin_brown", L"skin_red"};

int getMushroomDataValue(HugeMushroomBlock::Variant* variant)
{
    Block* block = Blocks::red_mushroom_block;
    const BlockState* state = block->defaultBlockState()->setValue(HugeMushroomBlock::VARIANT, variant);
    return block->convertBlockStateToLegacyData(state);
}

HugeMushroomBlock::Variant::Variant(int data, const std::wstring& name)
{
    mData = data;
    mName = name;
}

int HugeMushroomBlock::Variant::getData()
{
    return mData;
}

HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::byData(int data)
{
    if (data < 0 || data > 12)
    {
        data = 0;
    }
    Variant* result = NULL;
    for (int i = 0; i < 13; ++i)
    {
        if (VALUES[i]->getData() == data)
        {
            result = VALUES[i];
            break;
        }
    }
    return result;
}

std::wstring HugeMushroomBlock::Variant::toString() const
{
    return mName;
}

std::wstring HugeMushroomBlock::Variant::getSerializedName() const
{
    return mName;
}

template std::wstring _toString<HugeMushroomBlock::Variant>(HugeMushroomBlock::Variant*);

void HugeMushroomBlock::blockStaticCtor()
{
    std::wstring name;
    name = L"variant";
    EnumProperty<Variant*>* variant;
    Predicate<Variant*>* filter = Predicates<Variant*>::alwaysTrue;
    {
        boost::unordered_set<Variant*> values;
        for (int i = 0; i < 13; ++i)
        {
            values.insert(Variant::VALUES[i]);
        }

        variant = EnumProperty<Variant*>::create(name, typeid(Variant), values, filter);
    }
    VARIANT = variant;
}

HugeMushroomBlock::HugeMushroomBlock(Material* material, const MaterialColor* materialColor, Block* otherBlock)
    : Block(material, materialColor),
      mIsOtherBlock(0)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(VARIANT, Variant::ALL_OUTSIDE));
    mIsOtherBlock = -1;
    mOtherBlock = otherBlock;
    memset(mTextures, 0, sizeof(mTextures));
}

int HugeMushroomBlock::getResourceCount(Random* random)
{
    int count = random->nextInt(10) - 7;
    return count < 0 ? 0 : count;
}

const MaterialColor* HugeMushroomBlock::getMapColor(const BlockState* state, LevelSource* levelSource,
                                                    const BlockPos& pos)
{
    switch (state->getValue(VARIANT)->getData())
    {
    case 12:
        return MaterialColor::CLOTH;
    case 0:
    case 10:
        return MaterialColor::SAND;
    default:
        return Block::getMapColor(state, levelSource, pos);
    }
}

Item* HugeMushroomBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(mOtherBlock);
}

not_null_ptr<ItemInstance> HugeMushroomBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1, state->getValue(VARIANT)->getData()));
}

const BlockState* HugeMushroomBlock::getStateForPlacement(Level*, const BlockPos&, const Direction*, float, float,
                                                          float, int auxValue, boost::shared_ptr<LivingEntity>)
{
    return getBlockState(auxValue);
}

const BlockState* HugeMushroomBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(VARIANT, Variant::byData(data));
}

int HugeMushroomBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

const BlockState* HugeMushroomBlock::rotate(const BlockState* state, Rotation* rotation)
{
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise180:
        switch (state->getValue(VARIANT)->getData())
        {
        case 10:
            break;
        case 1:
            return state->setValue(VARIANT, Variant::SOUTH_EAST);
        case 2:
            return state->setValue(VARIANT, Variant::SOUTH);
        case 3:
            return state->setValue(VARIANT, Variant::SOUTH_WEST);
        case 4:
            return state->setValue(VARIANT, Variant::EAST);
        case 6:
            return state->setValue(VARIANT, Variant::WEST);
        case 7:
            return state->setValue(VARIANT, Variant::NORTH_EAST);
        case 8:
            return state->setValue(VARIANT, Variant::NORTH);
        case 9:
            return state->setValue(VARIANT, Variant::NORTH_WEST);
        default:
            return state;
        }
    case Rotation::eTYPE_CounterClockwise90:
        switch (state->getValue(VARIANT)->getData())
        {
        case 1:
            return state->setValue(VARIANT, Variant::SOUTH_WEST);
        case 2:
            return state->setValue(VARIANT, Variant::WEST);
        case 3:
            return state->setValue(VARIANT, Variant::NORTH_WEST);
        case 4:
            return state->setValue(VARIANT, Variant::SOUTH);
        case 6:
            return state->setValue(VARIANT, Variant::NORTH);
        case 7:
            return state->setValue(VARIANT, Variant::SOUTH_EAST);
        case 8:
            return state->setValue(VARIANT, Variant::EAST);
        case 9:
            return state->setValue(VARIANT, Variant::NORTH_EAST);
        case 10:
            break;
        default:
            return state;
        }
    case Rotation::eTYPE_Clockwise90:
        switch (state->getValue(VARIANT)->getData())
        {
        case 1:
            return state->setValue(VARIANT, Variant::NORTH_EAST);
        case 2:
            return state->setValue(VARIANT, Variant::EAST);
        case 3:
            return state->setValue(VARIANT, Variant::SOUTH_EAST);
        case 4:
            return state->setValue(VARIANT, Variant::NORTH);
        case 6:
            return state->setValue(VARIANT, Variant::SOUTH);
        case 7:
            return state->setValue(VARIANT, Variant::NORTH_WEST);
        case 8:
            return state->setValue(VARIANT, Variant::WEST);
        case 9:
            return state->setValue(VARIANT, Variant::SOUTH_WEST);
        default:
            return state;
        }
    default:
        return state;
    }
}

const BlockState* HugeMushroomBlock::mirror(const BlockState* state, Mirror* mirror)
{
    Variant* variant = state->getValue(VARIANT);
    switch (mirror->getType())
    {
    case Mirror::eTYPE_LeftRight:
        switch (variant->getData())
        {
        case 1:
            return state->setValue(VARIANT, Variant::SOUTH_WEST);
        case 2:
            return state->setValue(VARIANT, Variant::SOUTH);
        case 3:
            return state->setValue(VARIANT, Variant::SOUTH_EAST);
        case 7:
            return state->setValue(VARIANT, Variant::NORTH_WEST);
        case 8:
            return state->setValue(VARIANT, Variant::NORTH);
        case 9:
            return state->setValue(VARIANT, Variant::NORTH_EAST);
        default:
            return Block::mirror(state, mirror);
        }
    case Mirror::eTYPE_FrontBack:
        switch (variant->getData())
        {
        case 1:
            return state->setValue(VARIANT, Variant::NORTH_EAST);
        case 3:
            return state->setValue(VARIANT, Variant::NORTH_WEST);
        case 4:
            return state->setValue(VARIANT, Variant::EAST);
        case 6:
            return state->setValue(VARIANT, Variant::WEST);
        case 7:
            return state->setValue(VARIANT, Variant::SOUTH_EAST);
        case 9:
            return state->setValue(VARIANT, Variant::SOUTH_WEST);
        default:
            return Block::mirror(state, mirror);
        }
    default:
        return Block::mirror(state, mirror);
    }
}

not_null_ptr<ItemInstance> HugeMushroomBlock::getSilkTouchItemInstance(const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1, Variant::ALL_OUTSIDE->getData()));
}

BlockStateDefinition* HugeMushroomBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {VARIANT};
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* HugeMushroomBlock::getTexture(const Direction* direction, const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();
    if (data == 10 && direction->get3DDataValue() > 1)
        return mStemTexture;

    if (mIsOtherBlock == -1)
        mIsOtherBlock = mOtherBlock == Blocks::red_mushroom;

    if (data >= 1 && data <= 9 && direction == Direction::UP)
        return mTextures[mIsOtherBlock];
    if (data >= 1 && data <= 3 && direction == Direction::NORTH)
        return mTextures[mIsOtherBlock];
    if (data >= 7 && data <= 9 && direction == Direction::SOUTH)
        return mTextures[mIsOtherBlock];
    if ((data == 1 || data == 4 || data == 7) && direction == Direction::WEST)
        return mTextures[mIsOtherBlock];
    if ((data == 3 || data == 6 || data == 9) && direction == Direction::EAST)
        return mTextures[mIsOtherBlock];
    if (data == 11)
        return mTextures[mIsOtherBlock];
    if (data == 12)
        return mStemTexture;
    return mInsideTexture;
}

void HugeMushroomBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 2; ++i)
        mTextures[i] = iconRegister->registerIcon(getIconName() + L"_" + TEXTURE_TYPE[i]);
    mInsideTexture = iconRegister->registerIcon(getIconName() + L"_" + TEXTURE_INSIDE);
    mStemTexture = iconRegister->registerIcon(getIconName() + L"_" + TEXTURE_STEM);
}

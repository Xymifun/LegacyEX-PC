#include "game/world/block/QuartzBlock.hpp"

#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

static const int LINES_X_DATA = 3;
static const int LINES_Z_DATA = 4;

static const std::wstring gQuartzTopTexture = L"top";
static const std::wstring gQuartzChiseledTopTexture = L"chiseled_top";
static const std::wstring gQuartzLinesTopTexture = L"lines_top";
static const std::wstring gQuartzBottomTexture = L"bottom";
const std::wstring gQuartzVariantTextures[5] = {L"side", L"chiseled", L"lines", L"", L""};

QuartzBlock::Variant* QuartzBlock::Variant::DEFAULT = new QuartzBlock::Variant(0, L"default", L"default");
QuartzBlock::Variant* QuartzBlock::Variant::CHISELED = new QuartzBlock::Variant(1, L"chiseled", L"chiseled");
QuartzBlock::Variant* QuartzBlock::Variant::LINES_Y = new QuartzBlock::Variant(2, L"lines_y", L"linesy");
QuartzBlock::Variant* QuartzBlock::Variant::LINES_X = new QuartzBlock::Variant(3, L"lines_x", L"linesx");
QuartzBlock::Variant* QuartzBlock::Variant::LINES_Z = new QuartzBlock::Variant(4, L"lines_z", L"linesz");

QuartzBlock::Variant* gQuartzVariants[5] = {QuartzBlock::Variant::DEFAULT, QuartzBlock::Variant::CHISELED,
                                            QuartzBlock::Variant::LINES_Y, QuartzBlock::Variant::LINES_X,
                                            QuartzBlock::Variant::LINES_Z};

EnumProperty<QuartzBlock::Variant*>* QuartzBlock::VARIANT = NULL;

QuartzBlock::Variant::Variant(int data, const std::wstring& serializedName, const std::wstring& name)
{
    mData = data;
    mSerializedName = serializedName;
    mName = name;
}

int QuartzBlock::Variant::getData()
{
    return mData;
}

QuartzBlock::Variant* QuartzBlock::Variant::byData(int data)
{
    if (data < 0 || data > 4)
    {
        data = 0;
    }

    return gQuartzVariants[data];
}

std::wstring QuartzBlock::Variant::toString() const
{
    return mName;
}

std::wstring QuartzBlock::Variant::getSerializedName() const
{
    return mSerializedName;
}

void QuartzBlock::blockStaticCtor()
{
    std::wstring name = L"variant";
    EnumProperty<Variant*>* property;
    {
        const Predicate<Variant*>* filter = Predicates<Variant*>::alwaysTrue;
        boost::unordered_set<Variant*> values;
        for (int i = 0; i < 5; ++i)
        {
            values.insert(gQuartzVariants[i]);
        }

        property = EnumProperty<Variant*>::create(name, typeid(Variant), values, filter);
    }

    VARIANT = property;
}

QuartzBlock::QuartzBlock() : Block(Material::STONE)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(VARIANT, Variant::DEFAULT));
}

const BlockState* QuartzBlock::getStateForPlacement(Level*, const BlockPos&, const Direction* direction, float, float,
                                                    float, int auxValue, boost::shared_ptr<LivingEntity>)
{
    if (auxValue == Variant::LINES_Y->getData())
    {
        Direction::Axis::EAxis axis = direction->getAxis()->getId();
        const BlockState* state = defaultBlockState();
        switch (axis)
        {
        case Direction::Axis::eAxis_Z:
            return state->setValue(VARIANT, Variant::LINES_Z);
        case Direction::Axis::eAxis_X:
            return state->setValue(VARIANT, Variant::LINES_X);
        default:
            return state->setValue(VARIANT, Variant::LINES_Y);
        }
    }

    int chiseledData = Variant::CHISELED->getData();
    const BlockState* state = defaultBlockState();
    if (auxValue == chiseledData)
    {
        return state->setValue(VARIANT, Variant::CHISELED);
    }

    return state->setValue(VARIANT, Variant::DEFAULT);
}

int QuartzBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    Variant* variant = state->getValue(VARIANT);
    if (variant == Variant::LINES_X || variant == Variant::LINES_Z)
    {
        return Variant::LINES_Y->getData();
    }

    return variant->getData();
}

not_null_ptr<ItemInstance> QuartzBlock::getSilkTouchItemInstance(const BlockState* state)
{
    Variant* variant = state->getValue(VARIANT);
    if (variant == Variant::LINES_X || variant == Variant::LINES_Z)
    {
        return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1, Variant::LINES_Y->getData()));
    }

    return Block::getSilkTouchItemInstance(state);
}

const MaterialColor* QuartzBlock::getMapColor(const BlockState*, LevelSource*, const BlockPos&)
{
    return MaterialColor::QUARTZ;
}

const BlockState* QuartzBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(VARIANT, Variant::byData(data));
}

int QuartzBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

const BlockState* QuartzBlock::rotate(const BlockState* state, Rotation* rotation)
{
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise90:
    case Rotation::eTYPE_CounterClockwise90:
        Variant* variant = state->getValue(VARIANT);
        switch (variant->getData())
        {
        case LINES_X_DATA:
            return state->setValue(VARIANT, Variant::LINES_Z);
        case LINES_Z_DATA:
            return state->setValue(VARIANT, Variant::LINES_X);
        }
    }

    return state;
}

BlockStateDefinition* QuartzBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {VARIANT};
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* QuartzBlock::getTexture(const Direction* direction, const BlockState* state)
{
    Variant* variant = state->getValue(VARIANT);
    if (variant == Variant::LINES_Y || variant == Variant::LINES_X || variant == Variant::LINES_Z)
    {
        if (variant == Variant::LINES_Y && (direction == Direction::UP || direction == Direction::DOWN))
        {
            return mTextures[6];
        }
        if (variant == Variant::LINES_X && (direction == Direction::EAST || direction == Direction::WEST))
        {
            return mTextures[6];
        }
        if (variant == Variant::LINES_Z && (direction == Direction::NORTH || direction == Direction::SOUTH))
        {
            return mTextures[6];
        }
    }
    else
    {
        if (direction == Direction::UP)
        {
            return variant != Variant::CHISELED ? mTextures[7] : mTextures[5];
        }
        if (direction == Direction::DOWN)
        {
            return variant == Variant::CHISELED ? mTextures[5] : mTextures[8];
        }
    }

    return mTextures[variant->getData()];
}

void QuartzBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 5; ++i)
    {
        if (gQuartzVariantTextures[i].empty())
        {
            mTextures[i] = mTextures[i - 1];
        }
        else
        {
            mTextures[i] = iconRegister->registerIcon(getIconName() + L"_" + gQuartzVariantTextures[i]);
        }
    }

    mTextures[7] = iconRegister->registerIcon(getIconName() + L"_" + gQuartzTopTexture);
    mTextures[5] = iconRegister->registerIcon(getIconName() + L"_" + gQuartzChiseledTopTexture);
    mTextures[6] = iconRegister->registerIcon(getIconName() + L"_" + gQuartzLinesTopTexture);
    mTextures[8] = iconRegister->registerIcon(getIconName() + L"_" + gQuartzBottomTexture);
}

EBlockRenderShape QuartzBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_QUARTZ;
}

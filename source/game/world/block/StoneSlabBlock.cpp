#include "game/world/block/StoneSlabBlock.hpp"
#include "game/core/StringIds.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/Direction.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

StoneSlabBlock::Variant* StoneSlabBlock::Variant::STONE = NULL;
StoneSlabBlock::Variant* StoneSlabBlock::Variant::SAND = NULL;
StoneSlabBlock::Variant* StoneSlabBlock::Variant::WOOD = NULL;
StoneSlabBlock::Variant* StoneSlabBlock::Variant::COBBLESTONE = NULL;
StoneSlabBlock::Variant* StoneSlabBlock::Variant::BRICK = NULL;
StoneSlabBlock::Variant* StoneSlabBlock::Variant::SMOOTHBRICK = NULL;
StoneSlabBlock::Variant* StoneSlabBlock::Variant::NETHERBRICK = NULL;
StoneSlabBlock::Variant* StoneSlabBlock::Variant::QUARTZ = NULL;

static StoneSlabBlock::Variant* gStoneSlabVariants[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
EnumProperty<StoneSlabBlock::Variant*>* StoneSlabBlock::VARIANT = NULL;
static AbstractProperty<bool>* gStoneSlabSeamlessProperty = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"seamless"));

StoneSlabBlock::Variant::Variant(int data, const MaterialColor* materialColor, const std::wstring& name,
                                 int translationKey)
{
    mColor = materialColor;
    mData = data;
    mName = name;
    mTranslationKey = translationKey;
}

int StoneSlabBlock::Variant::getData()
{
    return mData;
}
int StoneSlabBlock::Variant::getTranslationKey()
{
    return mTranslationKey;
}

StoneSlabBlock::Variant* StoneSlabBlock::Variant::byData(int data)
{
    if (data < 0 || data > 7)
    {
        data = 0;
    }
    return gStoneSlabVariants[data];
}

const MaterialColor* StoneSlabBlock::Variant::getColor()
{
    return mColor;
}

std::wstring StoneSlabBlock::Variant::toString() const
{
    return mName;
}
std::wstring StoneSlabBlock::Variant::getSerializedName() const
{
    return mName;
}

void StoneSlabBlock::blockStaticCtor()
{
    Variant::STONE = new Variant(0, MaterialColor::STONE, L"stone", IDS_TILE_STONESLAB_STONE);
    Variant::SAND = new Variant(1, MaterialColor::SAND, L"sandstone", IDS_TILE_STONESLAB_SAND);
    Variant::WOOD = new Variant(2, MaterialColor::WOOD, L"wood_old", IDS_TILE_STONESLAB_WOOD);
    Variant::COBBLESTONE = new Variant(3, MaterialColor::STONE, L"cobblestone", 0xA1DC2A82);
    Variant::BRICK = new Variant(4, MaterialColor::COLOR_RED, L"brick", IDS_TILE_STONESLAB_BRICK);
    Variant::SMOOTHBRICK = new Variant(5, MaterialColor::STONE, L"stone_brick", IDS_TILE_STONESLAB_SMOOTHBRICK);
    Variant::NETHERBRICK = new Variant(6, MaterialColor::NETHER, L"nether_brick", IDS_TILE_STONESLAB_NETHERBRICK);
    Variant::QUARTZ = new Variant(7, MaterialColor::QUARTZ, L"quartz", IDS_TILE_STONESLAB_QUARTZ);
    gStoneSlabVariants[0] = Variant::STONE;
    gStoneSlabVariants[1] = Variant::SAND;
    gStoneSlabVariants[2] = Variant::WOOD;
    gStoneSlabVariants[3] = Variant::COBBLESTONE;
    gStoneSlabVariants[4] = Variant::BRICK;
    gStoneSlabVariants[5] = Variant::SMOOTHBRICK;
    gStoneSlabVariants[6] = Variant::NETHERBRICK;
    gStoneSlabVariants[7] = Variant::QUARTZ;

    boost::unordered_set<Variant*> values;
    for (int i = 0; i < 8; ++i)
    {
        values.insert(gStoneSlabVariants[i]);
    }
    VARIANT = EnumProperty<Variant*>::create(L"variant", typeid(Variant), values, Predicates<Variant*>::alwaysTrue);
}

StoneSlabBlock::StoneSlabBlock() : HalfSlabBlock(Material::STONE)
{
}

void StoneSlabBlock::DerivedInit()
{
    HalfSlabBlock::DerivedInit();
    const BlockState* state = mStateDefinition->any();
    if (isFullSize())
    {
        state = state->setValue(gStoneSlabSeamlessProperty, false);
    }
    else
    {
        state = state->setValue(TYPE, BOTTOM);
    }
    state = state->setValue(VARIANT, Variant::STONE);
    registerDefaultState(state);
}

Item* StoneSlabBlock::getResource(const BlockState* state, Random*, int)
{
    return Item::byBlock(Blocks::stone_slab);
}

not_null_ptr<ItemInstance> StoneSlabBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stone_slab, 1, data));
}

int StoneSlabBlock::getAuxName(int auxValue)
{
    return Variant::byData(auxValue)->getTranslationKey();
}

Property* StoneSlabBlock::getTypeProperty()
{
    return VARIANT;
}

Boxed* StoneSlabBlock::getTypeForItem(not_null_ptr<ItemInstance> itemInstance)
{
    return new TypedBoxed<Variant*>(Variant::byData(itemInstance->getAuxValue() & 7));
}

const BlockState* StoneSlabBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState()->setValue(VARIANT, Variant::byData(data & 7));
    if (isFullSize())
    {
        state = state->setValue(gStoneSlabSeamlessProperty, (data & 8) != 0);
    }
    else
    {
        state = state->setValue(TYPE, (data & 8) == 0 ? BOTTOM : TOP);
    }
    return state;
}

int StoneSlabBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();
    if (isFullSize())
    {
        if (state->getValue(gStoneSlabSeamlessProperty))
        {
            data |= 8;
        }
    }
    else if (state->getValue(HalfSlabBlock::TYPE) == HalfSlabBlock::TOP)
    {
        data |= 8;
    }
    return data;
}

BlockStateDefinition* StoneSlabBlock::createBlockStateDefinition()
{
    if (isFullSize())
    {
        static const Property* fullProperties[2] = {gStoneSlabSeamlessProperty, VARIANT};
        return new BlockStateDefinition(this, fullProperties, 2);
    }
    static const Property* halfProperties[2] = {HalfSlabBlock::TYPE, VARIANT};
    return new BlockStateDefinition(this, halfProperties, 2);
}

int StoneSlabBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

const MaterialColor* StoneSlabBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    return state->getValue(VARIANT)->getColor();
}

TextureAtlasSprite* StoneSlabBlock::getTexture(const Direction* direction, const BlockState* state)
{
    StoneSlabBlock::Variant* variant = state->getValue(VARIANT);
    switch (variant->getData())
    {
    case 0:
        if (direction != Direction::UP && direction != Direction::DOWN)
        {
            return mSideTexture;
        }
        break;
    case 1:
        return Blocks::sandstone->getTexture(direction);
    case 2:
        return Blocks::planks->getTexture(direction);
    case 3:
        return Blocks::cobblestone->getTexture(direction);
    case 4:
        return Blocks::brick_block->getTexture(direction);
    case 5:
        return Blocks::stonebrick->getTexture(direction);
    case 6:
        return Blocks::nether_brick->getTexture(Direction::UP);
    case 7:
        return Blocks::quartz_block->getTexture(direction);
    }
    return mIcon;
}

void StoneSlabBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"stoneslab_top");
    mSideTexture = iconRegister->registerIcon(L"stoneslab_side");
}

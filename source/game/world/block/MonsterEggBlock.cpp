#include "game/world/block/MonsterEggBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/Silverfish.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/core/GameRules.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/StoneBlock.hpp"
#include "game/world/block/StoneBrickBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/Material.hpp"

MonsterEggBlock::Variant* MonsterEggBlock::Variant::STONE = new MonsterEggBlock::Variant(0, L"stone",
                                                                                         IDS_TILE_STONE_SILVERFISH);
MonsterEggBlock::Variant* MonsterEggBlock::Variant::COBBLESTONE = new MonsterEggBlock::Variant(
    1, L"cobblestone", IDS_TILE_STONE_SILVERFISH_COBBLESTONE);
MonsterEggBlock::Variant* MonsterEggBlock::Variant::STONEBRICK = new MonsterEggBlock::Variant(
    2, L"stone_brick", IDS_TILE_STONE_SILVERFISH_STONE_BRICK);
MonsterEggBlock::Variant* MonsterEggBlock::Variant::MOSSY_STONEBRICK = new MonsterEggBlock::Variant(
    3, L"mossy_brick", IDS_TILE_STONE_SILVERFISH_MOSSY_STONE_BRICK);
MonsterEggBlock::Variant* MonsterEggBlock::Variant::CRACKED_STONEBRICK = new MonsterEggBlock::Variant(
    4, L"cracked_brick", IDS_TILE_STONE_SILVERFISH_CRACKED_STONE_BRICK);
MonsterEggBlock::Variant* MonsterEggBlock::Variant::CHISELED_STONEBRICK = new MonsterEggBlock::Variant(
    5, L"chiseled_brick", IDS_TILE_STONE_SILVERFISH_CHISELED_STONE_BRICK);
MonsterEggBlock::Variant* MonsterEggBlock::Variant::VALUES[6] = {MonsterEggBlock::Variant::STONE,
                                                                 MonsterEggBlock::Variant::COBBLESTONE,
                                                                 MonsterEggBlock::Variant::STONEBRICK,
                                                                 MonsterEggBlock::Variant::MOSSY_STONEBRICK,
                                                                 MonsterEggBlock::Variant::CRACKED_STONEBRICK,
                                                                 MonsterEggBlock::Variant::CHISELED_STONEBRICK};
EnumProperty<MonsterEggBlock::Variant*>* MonsterEggBlock::VARIANT = NULL;

MonsterEggBlock::Variant::Variant(int data, const std::wstring& name, int translationKey)
{
    mData = data;
    mName = name;
    mTranslationKey = translationKey;
}

std::wstring MonsterEggBlock::Variant::toString() const
{
    return mName;
}

std::wstring MonsterEggBlock::Variant::getSerializedName() const
{
    return mName;
}

int MonsterEggBlock::Variant::getData()
{
    return mData;
}

int MonsterEggBlock::Variant::getTranslationKey()
{
    return mTranslationKey;
}

MonsterEggBlock::Variant* MonsterEggBlock::Variant::byData(int data)
{
    if (data < 0 || data > 5)
    {
        data = 0;
    }
    return VALUES[data];
}

const BlockState* MonsterEggBlock::Variant::getHostBlockState()
{
    switch (mData)
    {
    default:
        return Blocks::stone->defaultBlockState()->setValue(StoneBlock::VARIANT, StoneBlock::Variant::STONE);
    case 1:
        return Blocks::cobblestone->defaultBlockState();
    case 2:
        return Blocks::stonebrick->defaultBlockState()->setValue(StoneBrickBlock::VARIANT,
                                                                 StoneBrickBlock::Variant::DEFAULT);
    case 3:
        return Blocks::stonebrick->defaultBlockState()->setValue(StoneBrickBlock::VARIANT,
                                                                 StoneBrickBlock::Variant::MOSSY);
    case 4:
        return Blocks::stonebrick->defaultBlockState()->setValue(StoneBrickBlock::VARIANT,
                                                                 StoneBrickBlock::Variant::CRACKED);
    case 5:
        return Blocks::stonebrick->defaultBlockState()->setValue(StoneBrickBlock::VARIANT,
                                                                 StoneBrickBlock::Variant::CHISELED);
    }
}

MonsterEggBlock::Variant* MonsterEggBlock::Variant::byHostBlock(const BlockState* state)
{
    for (int i = 0; i < 6; ++i)
    {
        Variant* variant = VALUES[i];
        if (state == variant->getHostBlockState())
        {
            return variant;
        }
    }
    return STONE;
}

void MonsterEggBlock::blockStaticCtor()
{
    std::wstring name(L"variant");
    EnumProperty<Variant*>* property;
    {
        const Predicate<Variant*>* filter = Predicates<Variant*>::alwaysTrue;
        boost::unordered_set<Variant*> values;
        int i;
        for (i = 0; i < 6; ++i)
        {
            values.insert(Variant::VALUES[i]);
        }
        property = EnumProperty<Variant*>::create(name, typeid(Variant), values, filter);
    }
    VARIANT = property;
}

MonsterEggBlock::MonsterEggBlock() : Block(Material::CLAY)
{
    DerivedInit();
    setDestroyTime(0.0f);
}

int MonsterEggBlock::getResourceCount(Random*)
{
    return 0;
}

bool MonsterEggBlock::isCompatibleHostBlock(const BlockState* state)
{
    const Block* block = state->getBlock();
    bool compatible = true;
    return state == Blocks::stone->defaultBlockState()->setValue(StoneBlock::VARIANT, StoneBlock::Variant::STONE)
           || block == Blocks::cobblestone || block == Blocks::stonebrick;
}

not_null_ptr<ItemInstance> MonsterEggBlock::getSilkTouchItemInstance(const BlockState* state)
{
    Variant* variant = state->getValue(VARIANT);
    switch (variant->getData())
    {
    case 1:
        return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::cobblestone));
    case 2:
        return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stonebrick));
    case 3:
        return not_null_ptr<ItemInstance>(
            new ItemInstance(Blocks::stonebrick, 1, StoneBrickBlock::Variant::MOSSY->getData()));
    case 4:
        return not_null_ptr<ItemInstance>(
            new ItemInstance(Blocks::stonebrick, 1, StoneBrickBlock::Variant::CRACKED->getData()));
    case 5:
        return not_null_ptr<ItemInstance>(
            new ItemInstance(Blocks::stonebrick, 1, StoneBrickBlock::Variant::CHISELED->getData()));
    default:
        return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stone));
    }
}

void MonsterEggBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState*, float, int)
{
    if (!level->mIsClientSide && level->getGameRules()->getBoolean(GameRules::RULE_DOBLOCKDROPS))
    {
        if (static_cast<unsigned int>(level->countInstanceOf(EntityType::MONSTER, false, NULL, NULL)) < 70
            && static_cast<unsigned int>(level->countInstanceOf(EntityType::SILVERFISH, true, NULL, NULL)) < 15)
        {
            boost::shared_ptr<Silverfish> silverfish(new Silverfish(level));
            silverfish->moveTo(pos.x + 0.5, pos.y, pos.z + 0.5, 0.0f, 0.0f);
            level->addEntity(silverfish);
            silverfish->spawnAnim();
        }
    }
}

not_null_ptr<ItemInstance> MonsterEggBlock::getCloneItemInstance(Level* level, const BlockPos& pos,
                                                                 const BlockState* state)
{
    return not_null_ptr<ItemInstance>(
        new ItemInstance(this, 1, state->getBlock()->convertBlockStateToLegacyData(state)));
}

const BlockState* MonsterEggBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(VARIANT, Variant::byData(data));
}

int MonsterEggBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

BlockStateDefinition* MonsterEggBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {VARIANT};
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* MonsterEggBlock::getTexture(const Direction* direction, const BlockState* state)
{
    Variant* variant = state->getValue(VARIANT);
    TextureAtlasSprite* result = NULL;
    if (variant == Variant::STONE)
    {
        result = Blocks::stone->getTexture(direction);
    }
    else if (variant == Variant::COBBLESTONE)
    {
        result = Blocks::cobblestone->getTexture(direction);
    }
    else if (variant == Variant::STONEBRICK)
    {
        result = Blocks::stonebrick->getTexture(direction);
    }
    else if (variant == Variant::MOSSY_STONEBRICK)
    {
        const BlockState* brickState = Blocks::stonebrick->defaultBlockState()->setValue(
            StoneBrickBlock::VARIANT, StoneBrickBlock::Variant::MOSSY);
        result = Blocks::stonebrick->getTexture(direction, brickState);
    }
    else if (variant == Variant::CRACKED_STONEBRICK)
    {
        const BlockState* brickState = Blocks::stonebrick->defaultBlockState()->setValue(
            StoneBrickBlock::VARIANT, StoneBrickBlock::Variant::CRACKED);
        result = Blocks::stonebrick->getTexture(direction, brickState);
    }
    else if (variant == Variant::CHISELED_STONEBRICK)
    {
        const BlockState* brickState = Blocks::stonebrick->defaultBlockState()->setValue(
            StoneBrickBlock::VARIANT, StoneBrickBlock::Variant::CHISELED);
        result = Blocks::stonebrick->getTexture(direction, brickState);
    }
    return result;
}

void MonsterEggBlock::registerIcons(IconRegister*)
{
}

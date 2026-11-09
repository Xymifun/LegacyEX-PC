#include "game/world/block/DoublePlantBlock.hpp"
#include "game/core/StringIds.hpp"

#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/DyeColor.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/biome/BiomeColors.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TallGrassBlock.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Stats.hpp"
#include "game/item/Items.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/item/ItemId.hpp"

namespace
{
std::wstring sVariantTextures[6][4] = {
    {L"double_plant_sunflower_top", L"double_plant_sunflower_bottom", L"double_plant_sunflower_front",
     L"double_plant_sunflower_back"},
    {L"double_plant_syringa_top", L"double_plant_syringa_bottom", L"", L""},
    {L"double_plant_grass_top", L"double_plant_grass_bottom", L"", L""},
    {L"double_plant_fern_top", L"double_plant_fern_bottom", L"", L""},
    {L"double_plant_rose_top", L"double_plant_rose_bottom", L"", L""},
    {L"double_plant_paeonia_top", L"double_plant_paeonia_bottom", L"", L""},
};
}

AbstractProperty<DoublePlantBlock::Variant*>* DoublePlantBlock::VARIANT = NULL;
AbstractProperty<DoublePlantBlock::Half*>* DoublePlantBlock::HALF = NULL;
AbstractProperty<const Direction*>* DoublePlantBlock::FACING = NULL;
DoublePlantBlock::Variant* DoublePlantBlock::VARIANTS[6] = {
    new DoublePlantBlock::Variant(0, L"sunflower", static_cast<int>(IDS_TILE_SUNFLOWER),
                                  static_cast<int>(IDS_DESC_SUNFLOWER), sVariantTextures[0]),
    new DoublePlantBlock::Variant(1, L"syringa", static_cast<int>(IDS_TILE_LILAC), static_cast<int>(IDS_DESC_LILAC),
                                  sVariantTextures[1]),
    new DoublePlantBlock::Variant(2, L"double_grass", static_cast<int>(IDS_TILE_TALL_GRASS_DOUBLE),
                                  static_cast<int>(IDS_DESC_TALL_GRASS_DOUBLE), sVariantTextures[2]),
    new DoublePlantBlock::Variant(3, L"double_fern", static_cast<int>(IDS_TILE_FERN_LARGE),
                                  static_cast<int>(IDS_DESC_FERN_LARGE), sVariantTextures[3]),
    new DoublePlantBlock::Variant(4, L"double_rose", static_cast<int>(IDS_TILE_ROSE_BUSH),
                                  static_cast<int>(IDS_DESC_ROSE_BUSH), sVariantTextures[4]),
    new DoublePlantBlock::Variant(5, L"paeonia", static_cast<int>(IDS_TILE_PEONY), static_cast<int>(IDS_DESC_PEONY),
                                  sVariantTextures[5])};
DoublePlantBlock::Half* DoublePlantBlock::UPPER_HALF = new DoublePlantBlock::Half(DoublePlantBlock::Half::UPPER);
DoublePlantBlock::Half* DoublePlantBlock::LOWER_HALF = new DoublePlantBlock::Half(DoublePlantBlock::Half::LOWER);
DoublePlantBlock::Half* DoublePlantBlock::HALVES[2] = {DoublePlantBlock::UPPER_HALF, DoublePlantBlock::LOWER_HALF};
DoublePlantBlock::Variant* gDoublePlantGrass = DoublePlantBlock::VARIANTS[2];
DoublePlantBlock::Variant* gDoublePlantFern = DoublePlantBlock::VARIANTS[3];

DoublePlantBlock::Half::Half(int id) : mId(id)
{
}
std::wstring DoublePlantBlock::Half::toString() const
{
    return getSerializedName();
}
std::wstring DoublePlantBlock::Half::getSerializedName() const
{
    return this == UPPER_HALF ? L"upper" : L"lower";
}

DoublePlantBlock::Variant::Variant(int data, const std::wstring& name, int nameId, int descriptionId,
                                   std::wstring* textures)
    : mData(data),
      mName(name),
      mNameId(nameId),
      mDescriptionId(descriptionId),
      mTextures(textures)
{
    mIcons[0] = NULL;
    mIcons[1] = NULL;
    mIcons[2] = NULL;
    mIcons[3] = NULL;
}
int DoublePlantBlock::Variant::getData()
{
    return mData;
}
DoublePlantBlock::Variant* DoublePlantBlock::Variant::byData(int data)
{
    if (data < 0 || data >= 6)
        data = 0;
    return VARIANTS[data];
}
TextureAtlasSprite* DoublePlantBlock::Variant::getIcon(ETexture texture)
{
    return mIcons[texture];
}
std::wstring* DoublePlantBlock::Variant::getTexture(int texture)
{
    return mTextures + texture;
}
void DoublePlantBlock::Variant::setIcon(ETexture texture, TextureAtlasSprite* icon)
{
    mIcons[texture] = icon;
}
std::wstring DoublePlantBlock::Variant::toString() const
{
    return mName;
}
std::wstring DoublePlantBlock::Variant::getSerializedName() const
{
    return mName;
}
int DoublePlantBlock::Variant::getNameId()
{
    return mNameId;
}
int DoublePlantBlock::Variant::getDescriptionId()
{
    return mDescriptionId;
}

DoublePlantBlock::DoublePlantBlock() : Bush(Material::REPLACEABLE_PLANT)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()
                             ->setValue(VARIANT, Variant::SUNFLOWER)
                             ->setValue(HALF, LOWER_HALF)
                             ->setValue(FACING, Direction::NORTH));
    setDestroyTime(0.0f);
    setSoundType(SoundType::GRASS);
}

DoublePlantBlock::Variant* DoublePlantBlock::getType(LevelSource* levelSource, const BlockPos& pos,
                                                     const BlockState* state)
{
    if (state->getBlock() == this)
        return state->fillVirtualBlockStateProperties(levelSource, pos)->getValue(VARIANT);
    return gDoublePlantFern;
}
bool DoublePlantBlock::isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state, bool)
{
    Variant* type = getType(level, pos, state);
    return type != gDoublePlantGrass && type != gDoublePlantFern;
}
bool DoublePlantBlock::isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*)
{
    return true;
}
void DoublePlantBlock::performBonemeal(Level* level, Random*, const BlockPos& pos, const BlockState* state)
{
    popResource(level, pos,
                not_null_ptr<ItemInstance>(new ItemInstance(this, 1, getType(level, pos, state)->getData())));
}
DoublePlantBlock::Variant* DoublePlantBlock::Variant::SUNFLOWER = DoublePlantBlock::VARIANTS[0];
DoublePlantBlock::Variant* DoublePlantBlock::Variant::SYRINGA = DoublePlantBlock::VARIANTS[1];
DoublePlantBlock::Variant* DoublePlantBlock::Variant::GRASS = DoublePlantBlock::VARIANTS[2];
DoublePlantBlock::Variant* DoublePlantBlock::Variant::FERN = DoublePlantBlock::VARIANTS[3];
DoublePlantBlock::Variant* DoublePlantBlock::Variant::ROSE = DoublePlantBlock::VARIANTS[4];
DoublePlantBlock::Variant* DoublePlantBlock::Variant::PAEONIA = DoublePlantBlock::VARIANTS[5];

void DoublePlantBlock::blockStaticCtor()
{
    {
        std::wstring name(L"variant");
        EnumProperty<Variant*>* variantProperty;
        {
            boost::unordered_set<Variant*> variants;
            for (int i = 0; i < 6; ++i)
            {
                variants.insert(VARIANTS[i]);
            }
            variantProperty = EnumProperty<Variant*>::create(name, typeid(Variant), variants,
                                                             Predicates<Variant*>::alwaysTrue);
        }
        VARIANT = variantProperty;
    }
    {
        std::wstring name(L"half");
        boost::unordered_set<Half*> halves;
        halves.insert(HALVES[0]);
        halves.insert(HALVES[1]);
        HALF = EnumProperty<Half*>::create(name, typeid(Half), halves, Predicates<Half*>::alwaysTrue);
    }
    FACING = HorizontalDirectionalBlock::FACING_PROPERTY;
}

static AABB* sLowerShape = AABB::newPermanent(0.3f, 0.0f, 0.3f, 0.7f, 1.0f, 0.7f);
static AABB* sUpperShape = AABB::newPermanent(0.3f, 0.0f, 0.3f, 0.7f, 0.6f, 0.7f);

const AABB* DoublePlantBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    return state->getValue(HALF) == UPPER_HALF ? sUpperShape : sLowerShape;
}
bool DoublePlantBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return Bush::mayPlace(level, pos) && level->isEmptyBlock(pos.above());
}
bool DoublePlantBlock::mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos)
{
    const BlockState* state = levelSource->getBlockState(pos);
    if (state->getBlock() == this)
    {
        Variant* type = state->fillVirtualBlockStateProperties(levelSource, pos)->getValue(VARIANT);
        return type == gDoublePlantFern || type == gDoublePlantGrass;
    }
    return true;
}
bool DoublePlantBlock::canContainLiquid(const BlockState*) const
{
    return false;
}
void DoublePlantBlock::checkAlive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (canSurvive(level, pos, state))
        return;
    bool isUpper = state->getValue(HALF) == UPPER_HALF;
    BlockPos upperPos = isUpper ? pos : pos.above();
    BlockPos lowerPos = isUpper ? pos.below() : pos;
    Block* upperBlock = isUpper ? this : level->getBlockState(upperPos)->getBlock();
    Block* lowerBlock = isUpper ? level->getBlockState(lowerPos)->getBlock() : this;
    if (upperBlock == this)
    {
        level->setBlock(upperPos, Blocks::air->defaultBlockState(), Block::UPDATE_CLIENTS, false);
    }
    if (lowerBlock == this)
    {
        level->setBlock(lowerPos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL, false);
        if (!isUpper)
        {
            spawnResources(level, lowerPos, state, 0);
        }
    }
}
bool DoublePlantBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (state->getValue(HALF) == UPPER_HALF)
    {
        return level->getBlockState(pos.below())->getBlock() == this;
    }
    const BlockState* aboveState = level->getBlockState(pos.above());
    return aboveState->getBlock() == this && Bush::canSurvive(level, pos, aboveState);
}
Item* DoublePlantBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    if (state->getValue(HALF) == UPPER_HALF)
        return Items::air;

    Variant* variant = state->getValue(VARIANT);
    if (variant == gDoublePlantFern)
        return Items::air;
    if (variant == gDoublePlantGrass)
        return random->nextInt(8) == 0 ? Items::wheatSeeds : Items::air;
    return Block::getResource(state, random, lootBonus);
}
int DoublePlantBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    if (state->getValue(HALF) == UPPER_HALF)
    {
        return 0;
    }
    if (state->getValue(VARIANT) == gDoublePlantGrass)
    {
        return 0;
    }
    return state->getValue(VARIANT)->getData();
}
int DoublePlantBlock::getColor(LevelSource* levelSource, const BlockPos& pos, int)
{
    const BlockState* state = levelSource->getBlockState(pos);
    return getColor(levelSource, pos, state);
}
int DoublePlantBlock::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    Variant* type = getType(levelSource, pos, state);
    return type == gDoublePlantGrass || type == gDoublePlantFern ? BiomeColors::getAverageGrassColor(levelSource, pos)
                                                                 : 0xffffff;
}
void DoublePlantBlock::placeAt(Level* level, const BlockPos& pos, Variant* variant, int flags)
{
    level->setBlock(pos, defaultBlockState()->setValue(HALF, LOWER_HALF)->setValue(VARIANT, variant), flags, false);
    level->setBlock(pos.above(), defaultBlockState()->setValue(HALF, UPPER_HALF), flags, false);
}
void DoublePlantBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                                   boost::shared_ptr<LivingEntity>, not_null_ptr<ItemInstance>)
{
    level->setBlock(pos.above(), defaultBlockState()->setValue(HALF, UPPER_HALF), Block::UPDATE_CLIENTS, false);
}
int DoublePlantBlock::popGrassResources(Level* level, const BlockPos& pos, const BlockState* state,
                                        boost::shared_ptr<Player> player)
{
    Variant* variant = state->getValue(VARIANT);
    if (variant != gDoublePlantFern && variant != gDoublePlantGrass)
        return 0;

    Player* awardee = player.operator->();
    Stat* stat = gStats->get_blocksMined(Block::getId(this));
    awardee->awardStat(stat, GenericStats::param_blocksMined(state, 1));
    int data = variant == gDoublePlantGrass ? TallGrassBlock::Variant::GRASS->getData()
                                            : TallGrassBlock::Variant::FERN->getData();
    Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Blocks::tallgrass, 2, data)));
    return 1;
}
void DoublePlantBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                                     const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                                     not_null_ptr<ItemInstance> tool)
{
    if (!level->mIsClientSide && tool->getItem() == Items::shears && state->getValue(HALF) == LOWER_HALF)
    {
        if (popGrassResources(level, pos, state, player) != 0)
            return;
    }
    Block::playerDestroy(level, player, pos, state, blockEntity, tool);
}
bool DoublePlantBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                         boost::shared_ptr<Player> player)
{
    if (state->getValue(HALF) == UPPER_HALF)
    {
        if (level->getBlockState(pos.below())->getBlock() == this)
        {
            if (!player->mAbilities.mInstabuild)
            {
                const BlockState* below = level->getBlockState(pos.below());
                Variant* variant = below->getValue(VARIANT);
                if (variant == gDoublePlantFern || variant == gDoublePlantGrass)
                {
                    if (!level->mIsClientSide)
                    {
                        bool shears = !player->getMainHandItem()->isEmpty()
                                      && player->getMainHandItem()->getItem() == Items::shears;
                        if (shears)
                        {
                            popGrassResources(level, pos, below, player);
                            level->removeBlock(pos.below());
                        }
                        else
                        {
                            level->destroyBlock(pos.below(), true);
                        }
                    }
                    else
                    {
                        level->removeBlock(pos.below());
                    }
                }
                else
                {
                    level->destroyBlock(pos.below(), true);
                }
            }
            else
            {
                level->removeBlock(pos.below());
            }
        }
    }
    else if (level->getBlockState(pos.above())->getBlock() == this)
    {
        level->setBlock(pos.above(), Blocks::air->defaultBlockState(), Block::UPDATE_CLIENTS, false);
    }
    return Block::playerWillDestroy(level, pos, state, player);
}
not_null_ptr<ItemInstance> DoublePlantBlock::getCloneItemInstance(Level* level, const BlockPos& pos,
                                                                  const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(this, 1, getType(level, pos, state)->getData()));
}
const BlockState* DoublePlantBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState();
    if ((data & 8) != 0)
        return state->setValue(HALF, UPPER_HALF);
    return state->setValue(HALF, LOWER_HALF)->setValue(VARIANT, Variant::byData(data & 7));
}
const BlockState* DoublePlantBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                                    const BlockPos& pos)
{
    if (state->getValue(HALF) == UPPER_HALF)
    {
        const BlockState* below = levelSource->getBlockState(pos.below());
        if (below->getBlock() == this)
        {
            state = state->setValue(VARIANT, below->getValue(VARIANT));
        }
    }
    return state;
}
int DoublePlantBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    if (state->getValue(HALF) == UPPER_HALF)
    {
        return state->getValue(FACING)->get2DDataValue() | 8;
    }
    return state->getValue(VARIANT)->getData();
}
unsigned int DoublePlantBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == ItemId::DYE
        && tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail
        && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0
        && isValidBonemealTarget((Level*)tooltipData.m_pLevel, tooltipData.m_blockPos, tooltipData.m_pBlockState, true))
    {
        return IDS_TOOLTIPS_GROW;
    }
    return Block::GetInteractTooltip(tooltipData);
}
BlockStateDefinition* DoublePlantBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {HALF, VARIANT, FACING};
    return new BlockStateDefinition(this, properties, 3);
}
EBlockOffsetType DoublePlantBlock::getOffsetType()
{
    return BLOCK_OFFSET_XZ;
}
EBlockRenderShape DoublePlantBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_DOUBLE_PLANT;
}
TextureAtlasSprite* DoublePlantBlock::getTexture(const BlockState* state, Variant::ETexture texture)
{
    return state->getValue(VARIANT)->getIcon(texture);
}
TextureAtlasSprite* DoublePlantBlock::getIconTexture(const Direction*, const BlockState* state)
{
    if (state->getValue(VARIANT) == Variant::SUNFLOWER)
        return getTexture(state, Variant::SIDE);
    return getTexture(state, Variant::BOTTOM);
}
TextureAtlasSprite* DoublePlantBlock::getTexture(const Direction*, const BlockState* state)
{
    return getTexture(state, state->getValue(HALF) != UPPER_HALF ? Variant::TOP : Variant::BOTTOM);
}
void DoublePlantBlock::registerIcons(IconRegister* iconRegister)
{
    for (int variantIndex = 0; variantIndex < 6; ++variantIndex)
    {
        Variant* variant = VARIANTS[variantIndex];
        for (int texture = 0; texture < 4; ++texture)
        {
            std::wstring* name = variant->getTexture(texture);
            if (!name->empty())
            {
                variant->setIcon(static_cast<Variant::ETexture>(texture), iconRegister->registerIcon(*name));
            }
        }
    }
}

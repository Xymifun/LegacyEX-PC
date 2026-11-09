#include "game/world/block/FlowerPotBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/L10N.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockItem.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/FlowerBlock.hpp"
#include "game/world/block/FlowerPotBlockEntity.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TallGrassBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/Predicates.hpp"

FlowerPotBlock::Contents* FlowerPotBlock::Contents::EMPTY = new FlowerPotBlock::Contents(Contents::eContentsEmpty,
                                                                                         L"empty");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::POPPY = new FlowerPotBlock::Contents(Contents::eContentsPoppy,
                                                                                         L"rose");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::BLUE_ORCHID = new FlowerPotBlock::Contents(
    Contents::eContentsBlueOrchid, L"blue_orchid");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::ALLIUM = new FlowerPotBlock::Contents(Contents::eContentsAllium,
                                                                                          L"allium");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::HOUSTONIA = new FlowerPotBlock::Contents(
    Contents::eContentsHoustonia, L"houstonia");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::RED_TULIP = new FlowerPotBlock::Contents(
    Contents::eContentsRedTulip, L"red_tulip");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::ORANGE_TULIP = new FlowerPotBlock::Contents(
    Contents::eContentsOrangeTulip, L"orange_tulip");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::WHITE_TULIP = new FlowerPotBlock::Contents(
    Contents::eContentsWhiteTulip, L"white_tulip");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::PINK_TULIP = new FlowerPotBlock::Contents(
    Contents::eContentsPinkTulip, L"pink_tulip");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::OXEYE_DAISY = new FlowerPotBlock::Contents(
    Contents::eContentsOxeyeDaisy, L"oxeye_daisy");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::DANDELION = new FlowerPotBlock::Contents(
    Contents::eContentsDandelion, L"dandelion");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::OAK_SAPLING = new FlowerPotBlock::Contents(
    Contents::eContentsOakSapling, L"oak_sapling");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::SPRUCE_SAPLING = new FlowerPotBlock::Contents(
    Contents::eContentsSpruceSapling, L"spruce_sapling");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::BIRCH_SAPLING = new FlowerPotBlock::Contents(
    Contents::eContentsBirchSapling, L"birch_sapling");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::JUNGLE_SAPLING = new FlowerPotBlock::Contents(
    Contents::eContentsJungleSapling, L"jungle_sapling");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::ACACIA_SAPLING = new FlowerPotBlock::Contents(
    Contents::eContentsAcaciaSapling, L"acacia_sapling");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::DARK_OAK_SAPLING = new FlowerPotBlock::Contents(
    Contents::eContentsDarkOakSapling, L"dark_oak_sapling");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::MUSHROOM_RED = new FlowerPotBlock::Contents(
    Contents::eContentsMushroomRed, L"mushroom_red");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::MUSHROOM_BROWN = new FlowerPotBlock::Contents(
    Contents::eContentsMushroomBrown, L"mushroom_brown");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::DEAD_BUSH = new FlowerPotBlock::Contents(
    Contents::eContentsDeadBush, L"dead_bush");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::FERN = new FlowerPotBlock::Contents(Contents::eContentsFern,
                                                                                        L"fern");
FlowerPotBlock::Contents* FlowerPotBlock::Contents::CACTUS = new FlowerPotBlock::Contents(Contents::eContentsCactus,
                                                                                          L"cactus");

const FlowerPotBlock::Contents* FlowerPotBlock::Contents::VALUES[FlowerPotBlock::Contents::eContents_Count] = {
    FlowerPotBlock::Contents::EMPTY,
    FlowerPotBlock::Contents::POPPY,
    FlowerPotBlock::Contents::BLUE_ORCHID,
    FlowerPotBlock::Contents::ALLIUM,
    FlowerPotBlock::Contents::HOUSTONIA,
    FlowerPotBlock::Contents::RED_TULIP,
    FlowerPotBlock::Contents::ORANGE_TULIP,
    FlowerPotBlock::Contents::WHITE_TULIP,
    FlowerPotBlock::Contents::PINK_TULIP,
    FlowerPotBlock::Contents::OXEYE_DAISY,
    FlowerPotBlock::Contents::DANDELION,
    FlowerPotBlock::Contents::OAK_SAPLING,
    FlowerPotBlock::Contents::SPRUCE_SAPLING,
    FlowerPotBlock::Contents::BIRCH_SAPLING,
    FlowerPotBlock::Contents::JUNGLE_SAPLING,
    FlowerPotBlock::Contents::ACACIA_SAPLING,
    FlowerPotBlock::Contents::DARK_OAK_SAPLING,
    FlowerPotBlock::Contents::MUSHROOM_RED,
    FlowerPotBlock::Contents::MUSHROOM_BROWN,
    FlowerPotBlock::Contents::DEAD_BUSH,
    FlowerPotBlock::Contents::FERN,
    FlowerPotBlock::Contents::CACTUS};

EnumProperty<const FlowerPotBlock::Contents*>* FlowerPotBlock::CONTENTS = NULL;
AbstractProperty<int>* FlowerPotBlock::LEGACY_DATA = static_cast<AbstractProperty<int>*>(
    IntegerProperty::create(L"legacy_data", 0, 15));
AABB* FlowerPotBlock::SHAPE = AABB::newPermanent(0.3125, 0.0, 0.3125, 0.6875, 0.375, 0.6875);
const double FlowerPotBlock::POT_WIDTH = 0.375;

FlowerPotBlock::Contents::Contents(EContents contents, const std::wstring& name) : mName(name), mContents(contents)
{
}

std::wstring FlowerPotBlock::Contents::toString() const
{
    return mName;
}

std::wstring FlowerPotBlock::Contents::getSerializedName() const
{
    return mName;
}

FlowerPotBlock::FlowerPotBlock() : BaseEntityBlock(Material::DECORATION)
{
    DerivedInit();
    registerDefaultState(
        mStateDefinition->any()->setValue<const Contents*>(CONTENTS, Contents::EMPTY)->setValue(LEGACY_DATA, 0));
    sendBlockData(15);
}

void FlowerPotBlock::blockStaticCtor()
{
    std::wstring name(L"contents");
    const Predicate<const Contents*>* filter = Predicates<const Contents*>::alwaysTrue;
    EnumProperty<const Contents*>* property;
    {
        boost::unordered_set<const Contents*> values;
        for (int i = 0; i < Contents::eContents_Count; ++i)
        {
            values.insert(Contents::VALUES[i]);
        }
        property = EnumProperty<const Contents*>::create(name, typeid(Contents), values, filter);
    }

    CONTENTS = property;
}

boost::shared_ptr<BlockEntity> FlowerPotBlock::newBlockEntity(Level* level, int data)
{
    Block* block = NULL;
    int itemData = 0;
    switch (data)
    {
    case 1:
        block = Blocks::red_flower;
        itemData = FlowerBlock::Flower::POPPY->getData();
        break;
    case 2:
        block = Blocks::yellow_flower;
        break;
    case 3:
        block = Blocks::sapling;
        itemData = PlanksBlock::Variant::OAK->getData();
        break;
    case 4:
        block = Blocks::sapling;
        itemData = PlanksBlock::Variant::SPRUCE->getData();
        break;
    case 5:
        block = Blocks::sapling;
        itemData = PlanksBlock::Variant::BIRCH->getData();
        break;
    case 6:
        block = Blocks::sapling;
        itemData = PlanksBlock::Variant::JUNGLE->getData();
        break;
    case 12:
        block = Blocks::sapling;
        itemData = PlanksBlock::Variant::ACACIA->getData();
        break;
    case 13:
        block = Blocks::sapling;
        itemData = PlanksBlock::Variant::DARK_OAK->getData();
        break;
    case 7:
        block = Blocks::red_mushroom;
        break;
    case 8:
        block = Blocks::brown_mushroom;
        break;
    case 9:
        block = Blocks::cactus;
        break;
    case 10:
        block = Blocks::deadbush;
        break;
    case 11:
        block = Blocks::tallgrass;
        itemData = TallGrassBlock::Variant::FERN->getData();
        break;
    default:
        break;
    }
    boost::shared_ptr<BlockEntity> blockEntity = boost::shared_ptr<FlowerPotBlockEntity>(
        new FlowerPotBlockEntity(Item::byBlock(block), itemData));
    blockEntity->setLevel(level);
    return blockEntity;
}

std::wstring FlowerPotBlock::getName()
{
    return std::wstring(L10N::GetString(IDS_TILE_FLOWERPOT));
}

const AABB* FlowerPotBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return SHAPE;
}

bool FlowerPotBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool FlowerPotBlock::isCubeShaped(const BlockState*)
{
    return false;
}

int FlowerPotBlock::getColor(LevelSource* levelSource, const BlockPos& pos, int tintIndex)
{
    boost::shared_ptr<BlockEntity> blockEntity = levelSource->getBlockEntity(pos);
    if (blockEntity != NULL)
    {
        bool isFlowerPot = eTYPE_DERIVED_FROM(BlockEntityType::FLOWER_POT, blockEntity->GetType());
        if (isFlowerPot)
        {
            Item* item = boost::static_pointer_cast<FlowerPotBlockEntity>(blockEntity)->getPlantItem();
            if (item != NULL && dynamic_cast<BlockItem*>(item) != NULL)
            {
                return Block::byItem(item)->getColor(levelSource, pos, tintIndex);
            }
        }
    }
    return 0xFFFFFF;
}

boost::shared_ptr<FlowerPotBlockEntity> FlowerPotBlock::getFlowerPotEntity(Level* level, const BlockPos& pos)
{
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (blockEntity != NULL)
    {
        bool isFlowerPot = eTYPE_DERIVED_FROM(BlockEntityType::FLOWER_POT, blockEntity->GetType());
        if (isFlowerPot)
        {
            return boost::dynamic_pointer_cast<FlowerPotBlockEntity>(blockEntity);
        }
    }
    return boost::shared_ptr<FlowerPotBlockEntity>();
}

bool FlowerPotBlock::canBePotted(const not_null_ptr<ItemInstance>& item)
{
    Block* block = Block::byItem(item->getItem());
    if (block == Blocks::yellow_flower || block == Blocks::red_flower || block == Blocks::cactus
        || block == Blocks::brown_mushroom || block == Blocks::red_mushroom || block == Blocks::sapling
        || block == Blocks::deadbush)
    {
        return true;
    }
    int auxValue = item->getAuxValue();
    if (block == Blocks::tallgrass && auxValue == TallGrassBlock::Variant::FERN->getData())
    {
        return true;
    }
    return false;
}

bool FlowerPotBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                         InteractionHand::EInteractionHand hand, const Direction*, float, float, float, bool)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    boost::shared_ptr<FlowerPotBlockEntity> entity = getFlowerPotEntity(level, pos);
    if (entity == NULL)
    {
        return false;
    }
    not_null_ptr<ItemInstance> plant = entity->getPlantItemInstance();
    if (plant->isEmpty())
    {
        if (!canBePotted(item))
        {
            return false;
        }
        entity->setItemInstance(item);
        if (!player->mAbilities.mInstabuild)
        {
            item->shrink(1);
        }
    }
    else
    {
        if (item->isEmpty())
        {
            player->setItemInHand(hand, plant);
        }
        else
        {
            if (!player->addItem(plant))
            {
                player->drop(plant, false);
            }
        }
        entity->setItemInstance(ItemInstance::EMPTY);
    }
    entity->setChanged();
    level->sendBlockUpdated(pos, state, state, 3, true);
    level->updateObservingBlocksAt(pos, this);
    return true;
}

not_null_ptr<ItemInstance> FlowerPotBlock::getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState*)
{
    boost::shared_ptr<FlowerPotBlockEntity> entity = getFlowerPotEntity(level, pos);
    if (entity != NULL)
    {
        not_null_ptr<ItemInstance> plant = entity->getPlantItemInstance();
        if (!plant->isEmpty())
        {
            return plant;
        }
    }
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::flowerPot));
}

bool FlowerPotBlock::CanSurvive(const BlockState* state)
{
    return state->canProvideSupport(Direction::UP, BLOCK_SUPPORT_CENTER);
}

void FlowerPotBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*,
                                     const BlockPos&)
{
    if (!CanSurvive(level->getBlockState(pos.below())))
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
    }
}

void FlowerPotBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    boost::shared_ptr<FlowerPotBlockEntity> entity = getFlowerPotEntity(level, pos);
    FlowerPotBlockEntity* pot = entity.get();
    if (pot != NULL && pot->getPlantItem() != NULL)
    {
        Block::popResource(level, pos,
                           not_null_ptr<ItemInstance>(new ItemInstance(pot->getPlantItem(), 1, pot->getItemData())));
    }
    BaseEntityBlock::onRemove(level, pos, state);
}

bool FlowerPotBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                       boost::shared_ptr<Player> player)
{
    bool result = Block::playerWillDestroy(level, pos, state, player);
    if (player->mAbilities.mInstabuild)
    {
        boost::shared_ptr<FlowerPotBlockEntity> entity = getFlowerPotEntity(level, pos);
        if (entity != NULL)
        {
            entity->setItemInstance(ItemInstance::EMPTY);
        }
    }
    return result;
}

Item* FlowerPotBlock::getResource(const BlockState*, Random*, int)
{
    return Items::flowerPot;
}
bool FlowerPotBlock::canContainLiquid(const BlockState*) const
{
    return true;
}

BlockStateDefinition* FlowerPotBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {CONTENTS, LEGACY_DATA};
    return new BlockStateDefinition(this, properties, 2);
}

int FlowerPotBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(LEGACY_DATA);
}

const BlockState* FlowerPotBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                                  const BlockPos& pos)
{
    const Contents* contents = Contents::EMPTY;
    boost::shared_ptr<BlockEntity> blockEntity = levelSource->getBlockEntity(pos);
    if (blockEntity != NULL)
    {
        bool isFlowerPot = eTYPE_DERIVED_FROM(BlockEntityType::FLOWER_POT, blockEntity->GetType());
        if (isFlowerPot)
        {
            boost::shared_ptr<FlowerPotBlockEntity> pot = boost::static_pointer_cast<FlowerPotBlockEntity>(blockEntity);
            Item* item = pot->getPlantItem();
            if (item != NULL && dynamic_cast<BlockItem*>(item) != NULL)
            {
                int itemData = pot->getItemData();
                Block* block = Block::byItem(item);
                if (block == Blocks::sapling)
                {
                    switch (PlanksBlock::Variant::byData(itemData)->getData())
                    {
                    case PlanksBlock::Variant::eDataOak:
                        contents = Contents::OAK_SAPLING;
                        break;
                    case PlanksBlock::Variant::eDataSpruce:
                        contents = Contents::SPRUCE_SAPLING;
                        break;
                    case PlanksBlock::Variant::eDataBirch:
                        contents = Contents::BIRCH_SAPLING;
                        break;
                    case PlanksBlock::Variant::eDataJungle:
                        contents = Contents::JUNGLE_SAPLING;
                        break;
                    case PlanksBlock::Variant::eDataAcacia:
                        contents = Contents::ACACIA_SAPLING;
                        break;
                    case PlanksBlock::Variant::eDataDarkOak:
                        contents = Contents::DARK_OAK_SAPLING;
                        break;
                    default:
                        contents = Contents::EMPTY;
                        break;
                    }
                }
                else if (block == Blocks::tallgrass)
                {
                    switch (itemData)
                    {
                    case 0:
                        contents = Contents::DEAD_BUSH;
                        break;
                    case 2:
                        contents = Contents::FERN;
                        break;
                    default:
                        contents = Contents::EMPTY;
                        break;
                    }
                }
                else if (block == Blocks::yellow_flower)
                {
                    contents = Contents::DANDELION;
                }
                else if (block == Blocks::red_flower)
                {
                    switch (FlowerBlock::Flower::byData(FlowerBlock::FlowerCategory::RED, itemData)->getId())
                    {
                    case FlowerBlock::Flower::FLOWER_POPPY:
                        contents = Contents::POPPY;
                        break;
                    case FlowerBlock::Flower::FLOWER_BLUE_ORCHID:
                        contents = Contents::BLUE_ORCHID;
                        break;
                    case FlowerBlock::Flower::FLOWER_ALLIUM:
                        contents = Contents::ALLIUM;
                        break;
                    case FlowerBlock::Flower::FLOWER_HOUSTONIA:
                        contents = Contents::HOUSTONIA;
                        break;
                    case FlowerBlock::Flower::FLOWER_RED_TULIP:
                        contents = Contents::RED_TULIP;
                        break;
                    case FlowerBlock::Flower::FLOWER_ORANGE_TULIP:
                        contents = Contents::ORANGE_TULIP;
                        break;
                    case FlowerBlock::Flower::FLOWER_WHITE_TULIP:
                        contents = Contents::WHITE_TULIP;
                        break;
                    case FlowerBlock::Flower::FLOWER_PINK_TULIP:
                        contents = Contents::PINK_TULIP;
                        break;
                    case FlowerBlock::Flower::FLOWER_OXEYE_DAISY:
                        contents = Contents::OXEYE_DAISY;
                        break;
                    default:
                        contents = Contents::EMPTY;
                        break;
                    }
                }
                else if (block == Blocks::red_mushroom)
                {
                    contents = Contents::MUSHROOM_RED;
                }
                else if (block == Blocks::brown_mushroom)
                {
                    contents = Contents::MUSHROOM_BROWN;
                }
                else if (block == Blocks::deadbush)
                {
                    contents = Contents::DEAD_BUSH;
                }
                else if (block == Blocks::cactus)
                {
                    contents = Contents::CACTUS;
                }
            }
        }
    }
    return state->setValue<const Contents*>(CONTENTS, contents);
}

int FlowerPotBlock::getRenderLayer()
{
    return 0;
}
EBlockFaceShape FlowerPotBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}
EBlockRenderShape FlowerPotBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_FLOWER_POT;
}

unsigned int FlowerPotBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    boost::shared_ptr<BlockEntity> blockEntity = tooltipData.m_pLevel->getBlockEntity(tooltipData.m_blockPos);
    bool plantEmpty = false;
    if (blockEntity != NULL)
    {
        bool isFlowerPot = eTYPE_DERIVED_FROM(BlockEntityType::FLOWER_POT, blockEntity->GetType());
        if (isFlowerPot)
        {
            not_null_ptr<ItemInstance> plant =
                boost::dynamic_pointer_cast<FlowerPotBlockEntity>(blockEntity)->getPlantItemInstance();
            plantEmpty = plant->isEmpty();
        }
    }

    if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Fail
        && !tooltipData.m_pItemInstance->isEmpty() && convertBlockStateToLegacyData(tooltipData.m_pBlockState) == 0)
    {
        int id = Item::getId(tooltipData.m_pItemInstance->getItem());
        if (tooltipData.m_bUseBlock && id < 256)
        {
            switch (id)
            {
            case 6:
            case 32:
            case 37:
            case 38:
            case 39:
            case 40:
            case 81:
                if (plantEmpty)
                    return IDS_TOOLTIPS_PLANT;
                return IDS_TOOLTIPS_PICKUP_GENERIC;
            case 31:
                if (tooltipData.m_pItemInstance->getAuxValue() != TallGrassBlock::Variant::GRASS->getData())
                {
                    if (plantEmpty)
                        return IDS_TOOLTIPS_PLANT;
                    return IDS_TOOLTIPS_PICKUP_GENERIC;
                }
                break;
            }
        }
    }
    else if (tooltipData.m_pItemInstance->isEmpty() && !plantEmpty)
    {
        return IDS_TOOLTIPS_PICKUP_GENERIC;
    }
    return Block::GetInteractTooltip(tooltipData);
}

bool FlowerPotBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return Block::mayPlace(level, pos) && CanSurvive(level->getBlockState(pos.below()));
}

bool FlowerPotBlock::waterSpreadCausesSpawn()
{
    return true;
}

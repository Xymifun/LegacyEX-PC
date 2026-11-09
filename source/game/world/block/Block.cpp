#include "game/world/block/Block.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcModContent.hpp"
#endif
#include "game/core/StringIds.hpp"
#include "game/core/Integer.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/item/ItemId.hpp"
#include "game/core/EntityType.hpp"

#include <limits>

#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/LeadItem.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/core/ExperienceOrb.hpp"
#include "game/world/block/BlockImpl.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/AirBlock.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/ThinFenceBlock.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/SnowBlock.hpp"
#include "game/world/block/StaticLiquidBlock.hpp"
#include "game/world/block/OreBlock.hpp"
#include "game/world/block/BedBlock.hpp"
#include "game/world/block/ButtonBlock.hpp"
#include "game/world/block/PistonBaseBlock.hpp"
#include "game/world/block/PistonHeadBlock.hpp"
#include "game/world/block/NewLeafBlock.hpp"
#include "game/world/block/NewLogBlock.hpp"
#include "game/world/block/OldLeafBlock.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/PoweredRailBlock.hpp"
#include "game/world/block/PrismarineBlock.hpp"
#include "game/world/block/PrismarineSlabBlock.hpp"
#include "game/world/block/QuartzBlock.hpp"
#include "game/world/block/RailBlock.hpp"
#include "game/world/block/RedSandStoneBlock.hpp"
#include "game/world/block/RedStoneWireBlock.hpp"
#include "game/world/block/RotatedPillarBlock.hpp"
#include "game/world/block/SandStoneBlock.hpp"
#include "game/world/block/StainedGlassBlock.hpp"
#include "game/world/block/StainedGlassPaneBlock.hpp"
#include "game/world/block/StoneBlock.hpp"
#include "game/world/block/StoneBrickBlock.hpp"
#include "game/world/block/StoneSlabBlock.hpp"
#include "game/world/block/TorchBlock.hpp"
#include "game/world/block/TrapDoorBlock.hpp"
#include "game/world/block/TripWireHookBlock.hpp"
#include "game/world/block/WallBlock.hpp"
#include "game/world/block/WoodSlabBlock.hpp"
#include "game/world/block/WoolCarpetBlock.hpp"
#include "game/world/block/PurpurSlabBlockFull.hpp"
#include "game/world/block/PurpurSlabBlockHalf.hpp"
#include "game/world/block/StainedHardenedClayBlock.hpp"
#include "game/world/block/StrippedLogBlock.hpp"
#include "game/world/block/RotatedPillarBlockImpl.hpp"
#include "game/world/block/PrismarineSlabBlockFull.hpp"
#include "game/world/block/PrismarineSlabBlockHalf.hpp"
#include "game/world/block/TurtleEggBlock.hpp"
#include "game/world/block/GlazedTerracottaBlock.hpp"
#include "game/world/block/ColoredBlock.hpp"
#include "game/world/block/DriedKelpBlock.hpp"
#include "game/world/block/KelpBlock.hpp"
#include "game/world/block/CoralBlock.hpp"
#include "game/world/block/BarrierBlock.hpp"
#include "game/world/block/BlueIceBlock.hpp"
#include "game/world/block/BoneBlock.hpp"
#include "game/world/block/BookshelfBlock.hpp"
#include "game/world/block/CactusBlock.hpp"
#include "game/world/block/CakeBlock.hpp"
#include "game/world/block/ChorusFlowerBlock.hpp"
#include "game/world/block/ChorusPlantBlock.hpp"
#include "game/world/block/ClayBlock.hpp"
#include "game/world/block/CocoaBlock.hpp"
#include "game/world/block/CraftingTableBlock.hpp"
#include "game/world/block/DetectorRailBlock.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/DragonEggBlock.hpp"
#include "game/world/block/DynamicLiquidBlock.hpp"
#include "game/world/block/EndPortalFrameBlock.hpp"
#include "game/world/block/EndRodBlock.hpp"
#include "game/world/block/FarmBlock.hpp"
#include "game/world/block/FenceBlock.hpp"
#include "game/world/block/FenceGateBlock.hpp"
#include "game/world/block/FireBlock.hpp"
#include "game/world/block/FullNewStoneSlabBlock.hpp"
#include "game/world/block/FullStoneSlabBlock.hpp"
#include "game/world/block/FullWoodSlabBlock.hpp"
#include "game/world/block/GlassBlock.hpp"
#include "game/world/block/Glowstoneblock.hpp"
#include "game/world/block/GrassBlock.hpp"
#include "game/world/block/GrassPathBlock.hpp"
#include "game/world/block/HalfNewStoneSlabBlock.hpp"
#include "game/world/block/HalfStoneSlabBlock.hpp"
#include "game/world/block/HalfWoodSlabBlock.hpp"
#include "game/world/block/HardenedClayBlock.hpp"
#include "game/world/block/HayBlock.hpp"
#include "game/world/block/HugeMushroomBlock.hpp"
#include "game/world/block/IceBlock.hpp"
#include "game/world/block/LadderBlock.hpp"
#include "game/world/block/LeverBlock.hpp"
#include "game/world/block/LogBlock.hpp"
#include "game/world/block/MagmaBlock.hpp"
#include "game/world/block/MelonBlock.hpp"
#include "game/world/block/MonsterEggBlock.hpp"
#include "game/world/block/MyceliumBlock.hpp"
#include "game/world/block/NetherBrickBlock.hpp"
#include "game/world/block/NetherrackBlock.hpp"
#include "game/world/block/NewStoneSlabBlock.hpp"
#include "game/world/block/ObsidianBlock.hpp"
#include "game/world/block/PackedIceBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/PortalBlock.hpp"
#include "game/world/block/PoweredBlock.hpp"
#include "game/world/block/PumpkinBlock.hpp"
#include "game/world/block/PurpurSlabBlock.hpp"
#include "game/world/block/RedFlowerBlock.hpp"
#include "game/world/block/RedstoneLampBlock.hpp"
#include "game/world/block/RedStoneOreBlock.hpp"
#include "game/world/block/RedstoneTorchBlock.hpp"
#include "game/world/block/ReedsBlock.hpp"
#include "game/world/block/SeaLanternBlock.hpp"
#include "game/world/block/SlimeBlock.hpp"
#include "game/world/block/SoulSandBlock.hpp"
#include "game/world/block/SpongeBlock.hpp"
#include "game/world/block/StructureBlock.hpp"
#include "game/world/block/StructureVoidBlock.hpp"
#include "game/world/block/TntBlock.hpp"
#include "game/world/block/TripWireBlock.hpp"
#include "game/world/block/VineBlock.hpp"
#include "game/world/block/WebBlock.hpp"
#include "game/world/block/YellowFlowerBlock.hpp"
#include "game/world/block/AnvilBlock.hpp"
#include "game/world/block/BannerBlock.hpp"
#include "game/world/block/BeaconBlock.hpp"
#include "game/world/block/BeetrootBlock.hpp"
#include "game/world/block/BrewingStandBlock.hpp"
#include "game/world/block/CarrotBlock.hpp"
#include "game/world/block/CauldronBlock.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/world/block/CommandBlock.hpp"
#include "game/world/block/ComparatorBlock.hpp"
#include "game/world/block/ConcretePowderBlock.hpp"
#include "game/world/block/ConduitBlock.hpp"
#include "game/world/block/CropBlock.hpp"
#include "game/world/block/DaylightDetectorBlock.hpp"
#include "game/world/block/DeadBushBlock.hpp"
#include "game/world/block/DispenserBlock.hpp"
#include "game/world/block/DropperBlock.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/EnchantmentTableBlock.hpp"
#include "game/world/block/EnderChestBlock.hpp"
#include "game/world/block/EndGatewayBlock.hpp"
#include "game/world/block/EndPortalBlock.hpp"
#include "game/world/block/FlowerPotBlock.hpp"
#include "game/world/block/FrostedIceBlock.hpp"
#include "game/world/block/FurnaceBlock.hpp"
#include "game/world/block/GravelBlock.hpp"
#include "game/world/block/HopperBlock.hpp"
#include "game/world/block/JukeboxBlock.hpp"
#include "game/world/block/MobSpawnerBlock.hpp"
#include "game/world/block/MushroomBlock.hpp"
#include "game/world/block/NetherWartBlock.hpp"
#include "game/world/block/NoteBlock.hpp"
#include "game/world/block/PistonMagicBlock.hpp"
#include "game/world/block/PotatoBlock.hpp"
#include "game/world/block/PressurePlateBlock.hpp"
#include "game/world/block/RepeaterBlock.hpp"
#include "game/world/block/SandBlock.hpp"
#include "game/world/block/SaplingBlock.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"
#include "game/world/block/SkullBlock.hpp"
#include "game/world/block/SnowLayerBlock.hpp"
#include "game/world/block/StoneButtonBlock.hpp"
#include "game/world/block/StandingBannerBlock.hpp"
#include "game/world/block/StandingSignBlock.hpp"
#include "game/world/block/StemBlock.hpp"
#include "game/world/block/TallGrassBlock.hpp"
#include "game/world/block/WallBannerBlock.hpp"
#include "game/world/block/WallSignBlock.hpp"
#include "game/world/block/WaterlilyBlock.hpp"
#include "game/world/block/WeightedPressurePlateBlock.hpp"
#include "game/world/block/WoodButtonBlock.hpp"
#include "game/core/Coral.hpp"
#include "game/core/CoralFan.hpp"
#include "game/core/CoralFanHang.hpp"
#include "game/core/SeaGrass.hpp"
#include "game/core/SeaPickle.hpp"
#include "game/core/BubbleColumnSegment.hpp"
#include "game/server/ObserverBlock.hpp"

#include "game/client/audio/SoundType.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Explosion.hpp"
#include "game/core/FoodConstants.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/FireworksRocketEntity.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/Enchantments.hpp"
#include "game/item/Item.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/core/Stats.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/block/BlockItem.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/StairBlock.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/LevelData.hpp"
#include "game/core/HitResult.hpp"
#include "platform/wiiu/TLSStorageCafe.hpp"

template <class T>
std::wstring _toString(T value);

static const ResourceLocation* AIR_LOCATION = new ResourceLocation(L"air");
DefaultedMappedRegistry<ResourceLocation, Block*>* const Block::blockRegistry =
    new DefaultedMappedRegistry<ResourceLocation, Block*>(*AIR_LOCATION);
IdMapper<const BlockState*>* Block::blockStateRegistry = new IdMapper<const BlockState*>();
const AABB* const Block::NULL_AABB = NULL;
const int Block::SIGNAL_MAX = 15;
const int Block::SIGNAL_NONE = 0;
const AABB* Block::FULL_BLOCK_SHAPE = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
float Block::INDESTRUCTIBLE = std::numeric_limits<float>::max();
unsigned int THREAD_STORAGE_INDEX = TlsAlloc();

boost::shared_ptr<ItemInstance> createItem(Block* block)
{
    return createItem(block->defaultBlockState());
}

Block::ThreadStorage::ThreadStorage()
{
    mCacheIndex = 0;
    mFastGraphics = false;

    const AABB** shapes = new const AABB*[3];
    shapes[0] = NULL;
    shapes[1] = NULL;
    shapes[2] = NULL;

    mShapes.length = 3;
    mShapes.data = shapes;
}

Block::ThreadStorage::~ThreadStorage()
{
    delete[] mShapes.data;
}

void Block::CreateNewThreadStorage()
{
    TlsSetValue(THREAD_STORAGE_INDEX, new ThreadStorage());
}

void Block::ReleaseThreadStorage()
{
    delete (ThreadStorage*)TlsGetValue(THREAD_STORAGE_INDEX);
}

void Block::_init(Material* material, const MaterialColor* materialColor)
{
    mMaterialColor = materialColor;
    mParticleGravity = 1.0f;
    mSoundType = SoundType::STONE;
    mFriction = 0.6f;
    mBlockType = EBLOCK_TYPE_NONE;
    mLightEmission = 0;
    mMaterial = material;
    mCollectStatistics = true;
    mBlockEntity = false;
    mExplosionResistance = 0.0f;
    mDestroyTime = 0.0f;
    mTicking = false;
    mTranslucent = !material->blocksLight();
    mIcon = NULL;
    mBaseItemMaterial = 0;
    mBaseItemType = 0;
    mIsDoor = false;
    mIsFence = false;
    mSemiTransparent = false;
    mIsRail = false;
    mMipmap = true;
    mIsWall = false;
    mDerivedInitDone = false;
    mUseNeighbourBrightness = false;
    mIsFenceGate = false;

    setThickness(0.0f);
}

Block::Block(Material* material)
{
    _init(material, material->getColor());
}

Block::Block(Material* material, const MaterialColor* materialColor)
{
    _init(material, materialColor);
}

Block::~Block()
{
}

void Block::registerBlock(int id, const ResourceLocation& location, Block* block)
{
    blockRegistry->registerMapping(id, ResourceLocation(location), block);
}

void Block::registerBlock(int id, const std::wstring& name, Block* block)
{
    ResourceLocation location(name);
    blockRegistry->registerMapping(id, ResourceLocation(location), block);
}

int Block::getId(Block* block)
{
    return blockRegistry->getId(block);
}

int Block::getElementId(Block* block)
{
    return blockRegistry->getId(block) << 4;
}

int Block::getElementId(const BlockState* state)
{
    Block* block = state->getBlock();
    return (blockRegistry->getId(block) << 4) | block->convertBlockStateToLegacyData(state);
}

int Block::getIdWithData(const BlockState* state)
{
    Block* block = state->getBlock();
    return getId(block) + block->convertBlockStateToLegacyData(state) * 0x1000;
}

Block* Block::byId(int id)
{
    if (id > 0x400)
    {
        id -= 0x300;
    }

#ifdef MINECRAFT_PC
    if (id > kPcModLastBlockId || id < 0)
#else
    if (id > 0x13E)
#endif
    {
        return NULL;
    }

    return blockRegistry->byId(id);
}

bool Block::existsWithId(int id)
{
    return blockRegistry->containsId(id);
}

const BlockState* Block::getStateByElementId(unsigned short elementId)
{
    return blockStateRegistry->byId(elementId);
}

const BlockState* Block::stateByIdWithData(int idWithData)
{
    return byId(idWithData & 0xFFF)->getBlockState((idWithData >> 12) & 0xF);
}

Block* Block::byItem(Item* item)
{
    BlockItem* blockItem = dynamic_cast<BlockItem*>(item);

    if (blockItem == NULL)
    {
        return Blocks::air;
    }

    return blockItem->getBlock();
}

Block* Block::byString(const std::wstring& name)
{
    ResourceLocation location(name);

    if (blockRegistry->containsKey(ResourceLocation(location)))
    {
        return blockRegistry->get(ResourceLocation(location));
    }

    return blockRegistry->byId(Integer::parseInt(name, 10));
}

Block* Block::setBaseItemTypeAndMaterial(int baseItemType, int baseItemMaterial)
{
    mBaseItemType = baseItemType;
    mBaseItemMaterial = baseItemMaterial;
    return this;
}

int Block::getBaseItemType()
{
    return mBaseItemType;
}

Block* Block::setDestroyTime(float destroyTime)
{
    mDestroyTime = destroyTime;

    if (destroyTime * 5.0f > mExplosionResistance)
    {
        mExplosionResistance = destroyTime * 5.0f;
    }

    return this;
}

bool Block::calculateIsWaterBlocking() const
{
    if (mThickness <= 0.0f)
    {
        if (mMaterial->blocksMotion() || mMaterial->isSolid())
        {
            return true;
        }
    }

    return false;
}

void Block::setThickness(float thickness)
{
    mThickness = 0.0f;
    mWaterBlocking = calculateIsWaterBlocking();
}

void Block::registerDefaultState(const BlockState* state)
{
    mDefaultBlockState = state;
}

const BlockState* Block::defaultBlockState()
{
    return mDefaultBlockState;
}

void Block::DerivedInit()
{
    mDerivedInitDone = true;
    mStateDefinition = createBlockStateDefinition();
    registerDefaultState(mStateDefinition->any());

    mFullBlock = defaultBlockState()->isSolidRender();
    mLightBlock = mFullBlock ? 255 : 0;
}

void Block::ClearDerivedInit()
{
    mDerivedInitDone = false;
    delete mStateDefinition;
}

Block* Block::setIconName(const std::wstring& iconName)
{
    mIconName = iconName;
    return this;
}

Block* Block::setSoundType(const SoundType* soundType)
{
    mSoundType = soundType;
    return this;
}

Block* Block::setNameAndDescriptionId(int descriptionId, int useDescriptionId)
{
    mDescriptionId = descriptionId;
    mUseDescriptionId = useDescriptionId;
    return this;
}

Block* Block::setIndestructible()
{
    setDestroyTime(INDESTRUCTIBLE);
    return this;
}

Block* Block::sendBlockData(unsigned char blockData)
{
    mBlockData = blockData;
    return this;
}

Block* Block::setLightBlock(int lightBlock)
{
    mLightBlock = lightBlock;
    return this;
}

Block* Block::setTicking(bool ticking)
{
    mTicking = ticking;
    return this;
}

Block* Block::setLightEmission(float lightEmission)
{
    mLightEmission = (int)(lightEmission * 15.0f);
    return this;
}

Block* Block::setExplodeable(float explosionResistance)
{
    mExplosionResistance = explosionResistance * 3.0f;
    return this;
}

bool Block::isTopSolidBlocking(const BlockState* state)
{
    return state->getMaterial()->isSolidBlocking() && state->isCubeShaped();
}

bool Block::canProvideSupport(const BlockState* state, const Direction* direction, EBlockSupportType)
{
    if (direction != Direction::UP)
        return isCubeShaped(state) || isSolidBlockingCube(state);
    if (isTopSolidBlocking(state) || isCubeShaped(state))
        return true;
    if (HalfSlabBlock::isHalfSlab(state) && state->getValue(HalfSlabBlock::TYPE) == HalfSlabBlock::TOP)
        return true;
    if (StairBlock::isStairs(state) && state->getValue(StairBlock::HALF) == StairBlock::Half::TOP)
        return true;
    if (state->getBlock() != Blocks::snow_layer)
        return false;
    return state->getValue(SnowLayerBlock::LAYERS) == 7;
}

bool Block::isValidSpawn(const BlockState* state, boost::shared_ptr<Entity> entity)
{
    return true;
}

int Block::getLightEmission(const BlockState* state)
{
    return mLightEmission;
}

bool Block::ParticlesSurviveWithin()
{
    return false;
}

bool Block::shouldBlockTick(Level* level, const BlockPos& pos, const BlockState* state)
{
    return true;
}

bool Block::isLiquidBlock()
{
    return false;
}

unsigned int Block::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (!Item::isItemABlock(Item::getId(tooltipData.m_pItemInstance->getItem())))
    {
        switch (Item::getId(tooltipData.m_pItemInstance->getItem()))
        {
        case ItemId::FLINT_AND_STEEL:
        case ItemId::FIRE_CHARGE:
            if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Fail
                || !tooltipData.m_bCanPlace)
                return IDS_NONE;
            return IDS_TOOLTIPS_IGNITE;

        case ItemId::WHEAT_SEEDS:
        case ItemId::REEDS:
        case ItemId::PUMPKIN_SEEDS:
        case ItemId::MELON_SEEDS:
        case ItemId::NETHER_WART:
        case ItemId::CARROT:
        case ItemId::POTATO:
        case ItemId::BEETROOT_SEEDS:
        case ItemId::KELP:
            break;

        case ItemId::PAINTING:
        case ItemId::ITEM_FRAME:
            if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Success)
                return IDS_NONE;
            return IDS_TOOLTIPS_HANG;

        case ItemId::WATER_BUCKET:
        case ItemId::LAVA_BUCKET:
        case ItemId::FISH_BUCKET:
        case ItemId::SALMON_BUCKET:
        case ItemId::PUFFER_BUCKET:
        case ItemId::TROPICAL_BUCKET:
            if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Success
                || !tooltipData.m_bCanPlace)
                return IDS_NONE;
            return IDS_TOOLTIPS_EMPTY;

        case ItemId::DYE:
            if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Fail)
                return IDS_NONE;
            if (DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 12)
                return IDS_TOOLTIPS_PLANT;
            return IDS_NONE;

        case ItemId::FIREWORKS:
            if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Fail
                || !tooltipData.m_bCanPlace)
                return IDS_NONE;
            return 0x568A07BEu;

        case ItemId::LEAD:
            if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success
                && tooltipData.m_bCanPlace
                && LeadItem::bindPlayerMobsTest(tooltipData.m_pPlayer, tooltipData.m_pLevel,
                                                BlockPos(tooltipData.m_pPlayer)))
                return IDS_TOOLTIPS_ATTACH;
            return IDS_NONE;

        default:
            return IDS_NONE;
        }

        if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail && tooltipData.m_bCanPlace)
            return IDS_TOOLTIPS_PLANT;
        return IDS_NONE;
    }

    switch (Item::getBlockId(tooltipData.m_pItemInstance->getItem()))
    {
    case BlockId::SAPLING:
    case BlockId::TALLGRASS:
    case BlockId::DEADBUSH:
    case BlockId::YELLOW_FLOWER:
    case BlockId::RED_FLOWER:
    case BlockId::BROWN_MUSHROOM:
    case BlockId::RED_MUSHROOM:
    case BlockId::CACTUS:
    case BlockId::REEDS:
    case BlockId::CARROTS:
    case BlockId::POTATOES:
    case BlockId::DOUBLE_PLANT:
    case BlockId::SEA_GRASS:
    case BlockId::SEA_PICKLE:
        if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail && tooltipData.m_bCanPlace)
            return IDS_TOOLTIPS_PLANT;
        return IDS_NONE;

    default:
        break;
    }

    if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Success || !tooltipData.m_bCanPlace)
        return IDS_NONE;
    return IDS_TOOLTIPS_PLACE;
}

unsigned int Block::GetHitTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (!tooltipData.m_bCanDestroy)
    {
        return IDS_NONE;
    }

    if (getId(tooltipData.m_pBlockState->getBlock()) == 7)
    {
        if (!tooltipData.m_pPlayer->GetGameMode()->isCreative())
        {
            return IDS_NONE;
        }

        if (!tooltipData.m_pLevel->mDimension->inPlayerConstraints(tooltipData.m_blockPos))
        {
            return IDS_NONE;
        }
    }

    return IDS_TOOLTIPS_MINE;
}

const MaterialColor* Block::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return mMaterialColor;
}

const BlockState* Block::getBlockState(int data)
{
    return defaultBlockState();
}

int Block::convertBlockStateToLegacyData(const BlockState* state)
{
    state->getProperties();
    return 0;
}

const BlockState* Block::setPropertyValueFromString(const BlockState* state, const Property* property,
                                                    const std::wstring& value)
{
    Boxed* boxed = property->getValue(value);
    const BlockState* result = state->setBoxedValue(property, boxed);

    delete boxed;

    return result;
}

const BlockState* Block::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                         const BlockPos& pos)
{
    return state;
}

const BlockState* Block::rotate(const BlockState* state, Rotation* rotation)
{
    return state;
}

const BlockState* Block::mirror(const BlockState* state, Mirror* mirror)
{
    return state;
}

bool Block::isExceptionBlockForAttaching(Block* block)
{
    return (block != NULL
            && (dynamic_cast<ShulkerBoxBlock*>(block) != NULL || dynamic_cast<LeafBlock*>(block) != NULL
                || dynamic_cast<TrapDoorBlock*>(block) != NULL))
           || block == Blocks::cauldron || block == Blocks::turtle_egg;
}

bool Block::isExceptionBlockForAttachingPlusPiston(Block* block)
{
    return isExceptionBlockForAttaching(block);
}

bool Block::isExceptionForPistonAttaching(Block* block)
{
    return block == Blocks::piston || block == Blocks::sticky_piston || block == Blocks::piston_extension;
}

bool Block::isExceptionForTopSolidBlocking(Block* block)
{
    return block == Blocks::glowstone || block == Blocks::sea_lantern || block == Blocks::ice || block == Blocks::glass
           || block == Blocks::stained_glass;
}

bool Block::isSolidBlockingCube(const BlockState* state)
{
    return mMaterial->blocksMotion() && state->isCubeShaped();
}

bool Block::isSolidBlockingCubeAndNotSignalSource(const BlockState* state)
{
    return mMaterial->isSolidBlocking() && state->isCubeShaped() && !state->isSignalSource();
}

bool Block::isViewBlocking(const BlockState* state)
{
    return mMaterial->blocksMotion() && defaultBlockState()->isCubeShaped();
}

bool Block::isCubeShaped(const BlockState* state)
{
    return true;
}

bool Block::hasCustomBreakingProgress(const BlockState* state)
{
    return false;
}

bool Block::isPathfindable(LevelSource* levelSource, const BlockPos& pos)
{
    return !mMaterial->blocksMotion();
}

EBlockRenderShape Block::getRenderShape(const BlockState* state)
{
    return BLOCK_RENDER_MODEL;
}

bool Block::hasInHandRenderOffset()
{
    return false;
}

bool Block::mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos)
{
    return false;
}

bool Block::isTransparent(const BlockState* state)
{
    return false;
}

bool Block::isIndestructible()
{
    return mDestroyTime == INDESTRUCTIBLE;
}

float Block::getDestroySpeed(const BlockState* state, Level* level, const BlockPos& pos)
{
    return mDestroyTime;
}

Block* Block::disableMipmap()
{
    mMipmap = false;
    return this;
}

Block* Block::setSemiTransparent()
{
    mSemiTransparent = true;
    return this;
}

bool Block::isTicking()
{
    return mTicking;
}

bool Block::isSlabBlock()
{
    return false;
}

bool Block::isWaterBlocking() const
{
    return mWaterBlocking;
}

bool Block::canContainLiquid(const BlockState* state) const
{
    return !state->isCubeShaped();
}

bool Block::isStrippable()
{
    return false;
}

const BlockState* Block::getStrippedBlock(const BlockState* state)
{
    return state;
}

bool Block::liquidCanFlowIntoFromDirection(LevelSource* levelSource, const BlockPos& pos,
                                           const Direction* direction) const
{
    return true;
}

const AABB* Block::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return FULL_BLOCK_SHAPE;
}

int Block::getLightColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, int lightValue)
{
    Block* block = levelSource->getBlockState(pos)->getBlock();
    int lightColor = levelSource->getLightColor(pos, block->getLightEmission(state), -1);

    if (lightColor == 0 && dynamic_cast<HalfSlabBlock*>(block) != NULL)
    {
        BlockPos below = pos.below();
        const BlockState* belowState = levelSource->getBlockState(below);

        Block* belowBlock = belowState->getBlock();

        return levelSource->getLightColor(below, belowBlock->getLightEmission(belowState), -1);
    }

    return lightColor;
}

bool Block::neighbourVisibleThroughShape(const AABB* shape, LevelSource* levelSource, const BlockPos& pos,
                                         const Direction* direction)
{
    switch (direction->get3DDataValue())
    {
    case 0:
        if (shape->mMinY > 0.0)
        {
            return true;
        }
        break;

    case 1:
        if (shape->mMaxY < 1.0)
        {
            return true;
        }
        break;

    case 2:
        if (shape->mMinZ > 0.0)
        {
            return true;
        }
        break;

    case 3:
        if (shape->mMaxZ < 1.0)
        {
            return true;
        }
        break;

    case 4:
        if (shape->mMinX > 0.0)
        {
            return true;
        }
        break;

    case 5:
        if (shape->mMaxX < 1.0)
        {
            return true;
        }
        break;

    default:
        break;
    }

    return false;
}

bool Block::neighbourVisibleThroughShapes(const arrayWithLength<const AABB*>* shapes, LevelSource* levelSource,
                                          const BlockPos& pos, const Direction* direction)
{
    for (unsigned int index = 0; index < shapes->length; index++)
    {
        const AABB* shape = shapes->data[index];

        if (shape != NULL && neighbourVisibleThroughShape(shape, levelSource, pos, direction))
        {
            return true;
        }
    }

    return false;
}

bool Block::shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                             const Direction* direction)
{
    if (neighbourVisibleThroughShapes(getShapes(state, levelSource, pos), levelSource, pos, direction))
    {
        return true;
    }

    return !levelSource->isSolidRender(pos.relative(direction));
}

const arrayWithLength<const AABB*>* Block::getShapes(const BlockState* state, LevelSource* levelSource,
                                                     const BlockPos& pos)
{
    ThreadStorage* storage = (ThreadStorage*)TlsGetValue(THREAD_STORAGE_INDEX);
    const AABB* shape = getShape(state, levelSource, pos);
    const AABB** shapes = storage->mShapes.data;

    shapes[0] = shape;
    shapes[2] = NULL;
    shapes[1] = NULL;

    return &storage->mShapes;
}

bool Block::isSolidFace(LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    return levelSource->getBlockState(pos)->getMaterial()->isSolid();
}

EBlockFaceShape Block::getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                         const Direction* direction)
{
    return BLOCK_FACE_SOLID;
}

const AABB* Block::getOutlineAABB(const BlockState* state, Level* level, const BlockPos& pos)
{
    return state->getShape(level, pos)->move(pos);
}

void Block::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                              std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity, bool isActualState)
{
    addCollisionAABB(pos, entityBox, collidingBoxes, state->getClipAABB(level, pos));
}

void Block::addCollisionAABB(const BlockPos& pos, const AABB* entityBox, std::vector<AABB*>* collidingBoxes,
                             const AABB* blockBox)
{
    if (blockBox == NULL)
    {
        return;
    }

    AABB* movedBox = blockBox->move(pos);

    bool intersects = entityBox->intersects(movedBox->mMinX, movedBox->mMinY, movedBox->mMinZ, movedBox->mMaxX,
                                            movedBox->mMaxY, movedBox->mMaxZ);

    if (intersects)
    {
        collidingBoxes->push_back(movedBox);
    }
}

const AABB* Block::getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return state->getShape(levelSource, pos);
}

bool Block::isSolidRender(const BlockState* state) const
{
    return true;
}

bool Block::mayPick(const BlockState* state, bool includeLiquid)
{
    return mayPick();
}

bool Block::mayPick()
{
    return true;
}

void Block::randomTick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    tick(level, pos, state, random);
}

void Block::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
}

void Block::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
}

void Block::destroy(Level* level, const BlockPos& pos, const BlockState* state)
{
}

void Block::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                            const BlockPos& fromPos)
{
}

void Block::addLights(Level* level, const BlockPos& pos)
{
}

int Block::getTickDelay(Level* level)
{
    return 10;
}

void Block::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
}

void Block::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
}

int Block::getResourceCount(Random* random)
{
    return 1;
}

Item* Block::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return Item::byBlock(this);
}

float Block::getDestroyProgress(const BlockState* state, boost::shared_ptr<Player> player, Level* level,
                                const BlockPos& pos)
{
    float destroySpeed = state->getDestroySpeed(level, pos);

    if (destroySpeed <= 0.0f)
    {
        return 1.0f;
    }

    if (player->canDestroy(state))
    {
        return player->getDestroySpeed(state) / destroySpeed / 30.0f;
    }

    return player->getDestroySpeed(state) / destroySpeed / 100.0f;
}

void Block::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, int lootBonus)
{
    spawnResources(level, pos, state, 1.0f, lootBonus);
}

void Block::popResource(Level* level, const BlockPos& pos, not_null_ptr<ItemInstance> itemInstance)
{
    if (level->mIsClientSide)
    {
        return;
    }

    if (itemInstance->isEmpty())
    {
        return;
    }

    if (!level->getGameRules()->getBoolean(GameRules::RULE_DOBLOCKDROPS))
    {
        return;
    }

    float spread = 0.5f;
    double xo = level->mRandom->nextFloat() * spread + (1.0f - spread) * 0.5;
    double yo = level->mRandom->nextFloat() * spread + (1.0f - spread) * 0.5;
    double zo = level->mRandom->nextFloat() * spread + (1.0f - spread) * 0.5;

    boost::shared_ptr<ItemEntity> itemEntity(new ItemEntity(level, pos.x + xo, pos.y + yo, pos.z + zo, itemInstance));
    itemEntity->setDefaultPickUpDelay();

    level->addEntity(itemEntity);
}

void Block::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance, int lootBonus)
{
    if (level->mIsClientSide)
    {
        return;
    }

    int count = getResourceCountForLootBonus(lootBonus, level->mRandom);

    while (count > 0)
    {
        if (level->mRandom->nextFloat() <= chance)
        {
            Item* item = getResource(state, level->mRandom, lootBonus);

            if (item != Items::air)
            {
                popResource(level, pos,
                            not_null_ptr<ItemInstance>(new ItemInstance(item, 1, getSpawnResourcesAuxValue(state))));
            }
        }

        count--;
    }
}

void Block::popExperience(Level* level, const BlockPos& pos, int amount)
{
    if (level->mIsClientSide)
    {
        return;
    }

    if (!level->getGameRules()->getBoolean(GameRules::RULE_DOBLOCKDROPS))
    {
        return;
    }

    while (amount > 0)
    {
        int value = ExperienceOrb::getExperienceValue(amount);
        amount -= value;

        boost::shared_ptr<ExperienceOrb> orb(
            new ExperienceOrb(level, (double)pos.x + 0.5, (double)pos.y + 0.5, (double)pos.z + 0.5, value));

        level->addEntity(orb);
    }
}

int Block::getSpawnResourcesAuxValue(const BlockState* state)
{
    return 0;
}

float Block::getExplosionResistance(boost::shared_ptr<Entity> entity)
{
    return mExplosionResistance / 5.0f;
}

HitResult* Block::clip(const BlockState* state, Level* level, const BlockPos& pos, Vec3* from, Vec3* to)
{
    return clip(pos, from, to, state->getShape(level, pos));
}

HitResult* Block::clip(const BlockPos& pos, Vec3* from, Vec3* to, const AABB* box)
{
    Vec3* localFrom = from->subtract((double)pos.x, (double)pos.y, (double)pos.z);
    Vec3* localTo = to->subtract((double)pos.x, (double)pos.y, (double)pos.z);

    HitResult* hit = box->clip(localFrom, localTo);

    if (hit == NULL)
    {
        return NULL;
    }

    HitResult* result = new HitResult(hit->mLocation->add((double)pos.x, (double)pos.y, (double)pos.z), hit->mDirection,
                                      pos);

    delete hit;

    return result;
}

void Block::wasExploded(Level* level, const BlockPos& pos, Explosion* explosion)
{
    if (level->mIsClientSide)
    {
        return;
    }

    if (!Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        return;
    }

    if (explosion == NULL)
    {
        return;
    }

    boost::shared_ptr<Entity> source = explosion->getDirectSource();

    if (source == NULL)
    {
        return;
    }

    if (eTYPE_DERIVED_FROM(EntityType::FIREWORKS_ROCKET_ENTITY, source->GetType()))
    {
        source = boost::static_pointer_cast<FireworksRocketEntity>(source)->getOwner();
    }

    if (source != NULL && eTYPE_DERIVED_FROM(EntityType::PLAYER, source->GetType()))
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(source);
        player->GetGameMode()->RecordBlockDestroyed(pos, getId(this));
    }
}

int Block::getRenderLayer()
{
    return 0;
}

bool Block::mayPlace(Level* level, const BlockPos& pos, const Direction* direction)
{
    return mayPlace(level, pos);
}

bool Block::mayPlace(Level* level, const BlockPos& pos)
{
    return level->getBlockState(pos)->getBlock()->mMaterial->isReplaceable();
}

bool Block::mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state)
{
    return true;
}

bool Block::TestUse()
{
    return false;
}

bool Block::TestUse(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player)
{
    return false;
}

bool Block::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY, float hitZ,
                bool clientSide)
{
    return false;
}

void Block::stepOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity)
{
}

const BlockState* Block::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction, float hitX,
                                              float hitY, float hitZ, int auxValue,
                                              boost::shared_ptr<LivingEntity> placer)
{
    return getBlockState(auxValue);
}

void Block::prepareRender(Level* level, const BlockPos& pos)
{
}

void Block::attack(Level* level, const BlockPos& pos, boost::shared_ptr<Player> player)
{
}

Vec3* Block::handleEntityInside(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, Vec3* motion)
{
    return motion;
}

int Block::getColor() const
{
    return 0xFFFFFF;
}

int Block::getColor(const BlockState* state)
{
    return 0xFFFFFF;
}

int Block::getColor(LevelSource* levelSource, const BlockPos& pos, int tintIndex)
{
    return 0xFFFFFF;
}

int Block::getColor(LevelSource* levelSource, const BlockPos& pos)
{
    return getColor(levelSource, pos, 0);
}

int Block::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    return 0xFFFFFF;
}

int Block::getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    return 0;
}

bool Block::isSignalSource(const BlockState* state)
{
    return false;
}

void Block::entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                         const boost::shared_ptr<Entity>& entity)
{
}

int Block::getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                           const Direction* direction)
{
    return 0;
}

bool Block::waterSpreadCausesSpawn()
{
    return false;
}

void Block::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos, const BlockState* state,
                          boost::shared_ptr<BlockEntity> blockEntity, not_null_ptr<ItemInstance> tool)
{
    if (state->getBlock() == Blocks::wheat)
    {
        if (Blocks::wheat->getResource(state, NULL, 0) != NULL)
        {
            player->awardStat(GenericStats::blocksMined(this), GenericStats::param_blocksMined(state, 1));
        }
    }
    else if (state->getBlock() == Blocks::potatoes)
    {
        if (Blocks::potatoes->getResource(state, NULL, 0) != NULL)
        {
            player->awardStat(GenericStats::blocksMined(this), GenericStats::param_blocksMined(state, 1));
        }
    }
    else if (state->getBlock() == Blocks::carrots)
    {
        if (Blocks::carrots->getResource(state, NULL, 0) != NULL)
        {
            player->awardStat(GenericStats::blocksMined(this), GenericStats::param_blocksMined(state, 1));
        }
    }
    else
    {
        player->awardStat(GenericStats::blocksMined(this), GenericStats::param_blocksMined(state, 1));
    }

    player->awardStat(GenericStats::mineBlock(), GenericStats::param_noArgs());
    player->causeFoodExhaustion(FoodConstants::ExhaustionMine());

    if (state->getBlock() == Blocks::log || state->getBlock() == Blocks::log2)
    {
        player->awardStat(GenericStats::mineWood(), GenericStats::param_noArgs());
    }

    if (isSilkTouchable() && EnchantmentHelper::getItemEnchantmentLevel(Enchantments::SILK_TOUCH, tool) > 0)
    {
        not_null_ptr<ItemInstance> item = getSilkTouchItemInstance(state);
        popResource(level, pos, item);
        return;
    }

    spawnResources(level, pos, state, EnchantmentHelper::getItemEnchantmentLevel(Enchantments::BLOCK_FORTUNE, tool));
}

bool Block::isSilkTouchable()
{
    return defaultBlockState()->isCubeShaped() && !mBlockEntity;
}

not_null_ptr<ItemInstance> Block::getSilkTouchItemInstance(const BlockState* state)
{
    Item* item = Item::byBlock(this);
    int auxValue = 0;

    if (item->isStackedByData())
    {
        auxValue = convertBlockStateToLegacyData(state);
    }

    return not_null_ptr<ItemInstance>(new ItemInstance(item, 1, auxValue));
}

bool Block::canSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    return true;
}

int Block::getResourceCountForLootBonus(int lootBonus, Random* random)
{
    return getResourceCount(random);
}

void Block::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                        boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance)
{
}

bool Block::isPossibleToRespawnInThis()
{
    return !mMaterial->isSolid() && !mMaterial->isLiquid();
}

std::wstring Block::getName()
{
    return std::wstring(L"");
}

int Block::getDescriptionId(int auxValue)
{
    return mDescriptionId;
}

int Block::getUseDescriptionId(int auxValue)
{
    return mUseDescriptionId;
}

bool Block::triggerEvent(const BlockState* state, Level* level, const BlockPos& pos, int eventId, int eventParam)
{
    return false;
}

bool Block::isCollectStatistics()
{
    return mCollectStatistics;
}

Block* Block::setNotCollectStatistics()
{
    mCollectStatistics = false;
    return this;
}

EPistonPushReaction Block::getPistonPushReaction(const BlockState* state)
{
    return (EPistonPushReaction)mMaterial->getPushReaction();
}

float Block::getShadeBrightness(const BlockState* state)
{
    if (state->isSolidBlockingCube())
    {
        return 0.2f;
    }

    return 1.0f;
}

void Block::fallOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, float fallDistance)
{
    entity->causeFallDamage(fallDistance, 1.0f);
}

void Block::updateEntityAfterFallOn(Level* level, boost::shared_ptr<Entity> entity)
{
    entity->mYd = 0.0;
}

not_null_ptr<ItemInstance> Block::getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1, getSpawnResourcesAuxValue(state)));
}

bool Block::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                              boost::shared_ptr<Player> player)
{
    return false;
}

void Block::handleRain(Level* level, const BlockPos& pos)
{
}

bool Block::canInstantlyTick()
{
    return true;
}

bool Block::dropFromExplosion(Explosion* explosion)
{
    return true;
}

bool Block::isMatching(Block* block)
{
    return this == block;
}

bool Block::isMatching(Block* first, Block* second)
{
    if (first == NULL || second == NULL)
    {
        return false;
    }

    if (first == second)
    {
        return true;
    }

    return first->isMatching(second);
}

bool Block::hasAnalogOutputSignal(const BlockState* state)
{
    return false;
}

int Block::getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos)
{
    return 0;
}

std::wstring Block::getIconName()
{
    if (mIconName.empty())
    {
        return std::wstring(L"MISSING_ICON_TILE_") + _toString(getId(this)) + L"_" + _toString(mDescriptionId);
    }

    return mIconName;
}

void Block::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(getIconName());
}

std::wstring Block::getTileItemIconName()
{
    return std::wstring(L"");
}

TextureAtlasSprite* Block::getIconTexture(const Direction* direction, const BlockState* state)
{
    return getTexture(direction, state);
}

TextureAtlasSprite* Block::getTexture(LevelSource* levelSource, const BlockPos& pos, const Direction* direction,
                                      const BlockState* state)
{
    if (!LeafBlock::isLeaves(state->getBlock()))
    {
        return getTexture(direction, state);
    }

    int OFFSETS_X[6] = {1, -1, 0, 0, 0, 0};
    int OFFSETS_Y[6] = {0, 0, 1, -1, 0, 0};
    int OFFSETS_Z[6] = {0, 0, 0, 0, 1, -1};

    bool surrounded = true;

    for (int index = 0; index < 6 && surrounded; index++)
    {
        const BlockState* neighbour = levelSource->getBlockState(
            pos.offset(OFFSETS_X[index], OFFSETS_Y[index], OFFSETS_Z[index]));

        if (!LeafBlock::isLeaves(neighbour->getBlock()) && !neighbour->isSolidRender())
        {
            surrounded = false;
        }
    }

    if (!surrounded)
    {
        return getTexture(direction, state);
    }

    static_cast<LeafBlock*>(Blocks::leaves)->setFancy(false);
    static_cast<LeafBlock*>(Blocks::leaves2)->setFancy(false);

    TextureAtlasSprite* texture = getTexture(direction, state);

    static_cast<LeafBlock*>(Blocks::leaves)->setFancy(true);
    static_cast<LeafBlock*>(Blocks::leaves2)->setFancy(true);

    return texture;
}

TextureAtlasSprite* Block::getTexture(const Direction* direction, const BlockState* state)
{
    return mIcon;
}

TextureAtlasSprite* Block::getTexture(const Direction* direction)
{
    return getTexture(direction, defaultBlockState());
}

int Block::getFaceFlagsFast(LevelSource* levelSource, const BlockPos& pos)
{
    const AABB* shape = levelSource->getBlockState(pos)->getShape(levelSource, pos);
    int flags = 0;

    if (shape->mMinY > 0.0 || !levelSource->getBlockState(pos.below())->isSolidRender())
    {
        flags = 1;
    }

    if (shape->mMaxY < 1.0 || !levelSource->getBlockState(pos.above())->isSolidRender())
    {
        flags |= 2;
    }

    if (shape->mMinZ > 0.0 || !levelSource->getBlockState(pos.north())->isSolidRender())
    {
        flags |= 4;
    }

    if (shape->mMaxZ < 1.0 || !levelSource->getBlockState(pos.south())->isSolidRender())
    {
        flags |= 8;
    }

    if (shape->mMinX > 0.0 || !levelSource->getBlockState(pos.west())->isSolidRender())
    {
        flags |= 16;
    }

    if (shape->mMaxX < 1.0 || !levelSource->getBlockState(pos.east())->isSolidRender())
    {
        flags |= 32;
    }

    return flags;
}

int Block::getFaceFlags(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, bool forceAll)
{
    Block* block = state->getBlock();
    int flags;

    if (!forceAll)
    {
        int id = getId(block);

        if (id > 7 && (unsigned int)(id - 12) >= 6)
        {
            int down = block->shouldRenderFace(state, levelSource, pos, Direction::DOWN) ? 1 : 0;
            int up = 0;

            if (block->shouldRenderFace(state, levelSource, pos, Direction::UP))
            {
                up = 2;
            }

            int north = 0;

            if (block->shouldRenderFace(state, levelSource, pos, Direction::NORTH))
            {
                north = 4;
            }

            int south = 0;

            if (block->shouldRenderFace(state, levelSource, pos, Direction::SOUTH))
            {
                south = 8;
            }

            int west = 0;

            if (block->shouldRenderFace(state, levelSource, pos, Direction::WEST))
            {
                west = 16;
            }

            int east = 0;

            if (block->shouldRenderFace(state, levelSource, pos, Direction::EAST))
            {
                east = 32;
            }

            flags = down | up | north | south | west | east;
        }
        else
        {
            flags = block->getFaceFlagsFast(levelSource, pos);
        }
    }
    else
    {
        flags = 0x3F;
    }

    return flags;
}

const BlockState* Block::getInventoryRenderState(const BlockState* state)
{
    return state;
}

BlockStateDefinition* Block::createBlockStateDefinition()
{
    return new BlockStateDefinition(this);
}

BlockStateDefinition* Block::getBlockStateDefinition()
{
    return mStateDefinition;
}

EBlockOffsetType Block::getOffsetType()
{
    return BLOCK_OFFSET_NONE;
}

Vec3* Block::getOffset(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    EBlockOffsetType offsetType = getOffsetType();

    if (offsetType == BLOCK_OFFSET_NONE)
    {
        return Vec3::newTemp(Vec3::ZERO);
    }

    long long seed = Mth::getSeed(pos.x, 0, pos.z);

    Vec3* offset;

    if (offsetType == BLOCK_OFFSET_XYZ)
    {
        offset = Vec3::newTemp(((float)((seed >> 16) & 0xF) / 15.0f - 0.5) * 0.5,
                               ((float)((seed >> 20) & 0xF) / 15.0f - 1.0) * 0.2,
                               ((float)((seed >> 24) & 0xF) / 15.0f - 0.5) * 0.5);
    }
    else
    {
        offset = Vec3::newTemp(((float)((seed >> 16) & 0xF) / 15.0f - 0.5) * 0.5, 0.0,
                               ((float)((seed >> 24) & 0xF) / 15.0f - 0.5) * 0.5);
    }

    return offset;
}

const SoundType* Block::getSoundType()
{
    return mSoundType;
}

std::wstring Block::toString()
{
    return L"Block{" + blockRegistry->getKey(this).toString() + L"}";
}

void Block::appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                            std::vector<HtmlString>* lines, bool advanced)
{
}

bool Block::instanceof(EBlockType blockType)
{
    return mBlockType == blockType;
}

void Block::staticCtor()
{
    DirectionalBlock::FACING = DirectionProperty::create(L"facing");
    HorizontalDirectionalBlock::FACING = DirectionProperty::create(L"facing", Direction::Plane::HORIZONTAL);

    DirectionalBlock::blockStaticCtor();
    HorizontalDirectionalBlock::blockStaticCtor();
    AnvilBlock::blockStaticCtor();
    BannerBlock::blockStaticCtor();
    BedBlock::blockStaticCtor();
    ButtonBlock::blockStaticCtor();
    ChestBlock::blockStaticCtor();
    ColoredBlock::blockStaticCtor();
    ComparatorBlock::blockStaticCtor();
    DetectorRailBlock::blockStaticCtor();
    DirtBlock::blockStaticCtor();
    DispenserBlock::blockStaticCtor();
    DoorBlock::blockStaticCtor();
    DoublePlantBlock::blockStaticCtor();
    EnderChestBlock::blockStaticCtor();
    EndPortalFrameBlock::blockStaticCtor();
    FlowerPotBlock::blockStaticCtor();
    FurnaceBlock::blockStaticCtor();
    HalfSlabBlock::blockStaticCtor();
    HopperBlock::blockStaticCtor();
    HugeMushroomBlock::blockStaticCtor();
    LadderBlock::blockStaticCtor();
    LeverBlock::blockStaticCtor();
    LogBlock::blockStaticCtor();
    MonsterEggBlock::blockStaticCtor();
    PistonBaseBlock::blockStaticCtor();
    PistonHeadBlock::blockStaticCtor();
    PistonMagicBlock::blockStaticCtor();
    PlanksBlock::blockStaticCtor();
    NewLeafBlock::blockStaticCtor();
    NewLogBlock::blockStaticCtor();
    SandBlock::blockStaticCtor();
    NewStoneSlabBlock::blockStaticCtor();
    OldLeafBlock::blockStaticCtor();
    OldLogBlock::blockStaticCtor();
    PortalBlock::blockStaticCtor();
    PoweredRailBlock::blockStaticCtor();
    PrismarineBlock::blockStaticCtor();
    PrismarineSlabBlock::blockStaticCtor();
    QuartzBlock::blockStaticCtor();
    RailBlock::blockStaticCtor();
    RedSandStoneBlock::blockStaticCtor();
    RedStoneWireBlock::blockStaticCtor();
    RotatedPillarBlock::blockStaticCtor();
    SandStoneBlock::blockStaticCtor();
    SaplingBlock::blockStaticCtor();
    SkullBlock::blockStaticCtor();
    StainedGlassBlock::blockStaticCtor();
    StainedGlassPaneBlock::blockStaticCtor();
    StairBlock::blockStaticCtor();
    StoneBlock::blockStaticCtor();
    StoneBrickBlock::blockStaticCtor();
    StoneSlabBlock::blockStaticCtor();
    TallGrassBlock::blockStaticCtor();
    TorchBlock::blockStaticCtor();
    StemBlock::blockStaticCtor();
    TrapDoorBlock::blockStaticCtor();
    TripWireHookBlock::blockStaticCtor();
    WallBlock::blockStaticCtor();
    WallSignBlock::blockStaticCtor();
    WoodSlabBlock::blockStaticCtor();
    WoolCarpetBlock::blockStaticCtor();
    PurpurSlabBlock::blockStaticCtor();
    NoteBlock::blockStaticCtor();
    ConcretePowderBlock::blockStaticCtor();
    StainedHardenedClayBlock::staticCtor();
    CommandBlock::blockStaticCtor();

    registerBlock(0, *AIR_LOCATION, (new AirBlock())->setNameAndDescriptionId(-1, -1));

    registerBlock(1, L"stone",
                  (new StoneBlock())
                      ->setDestroyTime(1.5f)
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setIconName(L"stone")
                      ->setNameAndDescriptionId(IDS_TILE_STONE, IDS_DESC_STONE)
                      ->setBaseItemTypeAndMaterial(14, 2));

    registerBlock(2, L"grass",
                  (new GrassBlock())
                      ->setDestroyTime(0.6f)
                      ->setSoundType(SoundType::GRASS)
                      ->setIconName(L"grass")
                      ->setNameAndDescriptionId(IDS_TILE_GRASS, IDS_DESC_GRASS)
                      ->setBaseItemTypeAndMaterial(13, 58));

    registerBlock(3, L"dirt",
                  (new DirtBlock())
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::GRAVEL)
                      ->setIconName(L"dirt")
                      ->setNameAndDescriptionId(IDS_TILE_DIRT, IDS_DESC_DIRT)
                      ->setBaseItemTypeAndMaterial(13, 58));

    Block* cobblestone = (new BlockImpl(Material::STONE))
                             ->setDestroyTime(2.0f)
                             ->setExplodeable(10.0f)
                             ->setSoundType(SoundType::STONE)
                             ->setIconName(L"cobblestone")
                             ->setNameAndDescriptionId(IDS_TILE_STONE_BRICK, IDS_DESC_STONE_BRICK)
                             ->setBaseItemTypeAndMaterial(14, 2);
    registerBlock(4, L"cobblestone", cobblestone);

    Block* planks = (new PlanksBlock())
                        ->setIconName(L"planks")
                        ->setDestroyTime(2.0f)
                        ->setExplodeable(5.0f)
                        ->setSoundType(SoundType::WOOD)
                        ->sendBlockData(15)
                        ->setNameAndDescriptionId(IDS_TILE_OAKWOOD_PLANKS, IDS_DESC_WOODENPLANKS)
                        ->setBaseItemTypeAndMaterial(50, 1);
    registerBlock(5, L"planks", planks);

    registerBlock(6, L"sapling",
                  (new SaplingBlock())
                      ->setDestroyTime(0.0f)
                      ->setSoundType(SoundType::GRASS)
                      ->setIconName(L"sapling")
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_SAPLING, IDS_DESC_SAPLING)
                      ->disableMipmap());

    registerBlock(7, L"bedrock",
                  (new BlockImpl(Material::STONE))
                      ->setIndestructible()
                      ->setExplodeable(6000000.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setIconName(L"bedrock")
                      ->setNotCollectStatistics()
                      ->setNameAndDescriptionId(IDS_TILE_BEDROCK, IDS_DESC_BEDROCK));

    registerBlock(8, L"flowing_water",
                  (new DynamicLiquidBlock(Material::WATER))
                      ->setDestroyTime(100.0f)
                      ->setLightBlock(2)
                      ->setIconName(L"water_flow")
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_WATER, IDS_DESC_WATER));

    registerBlock(9, L"water",
                  (new StaticLiquidBlock(Material::WATER))
                      ->setDestroyTime(100.0f)
                      ->setLightBlock(2)
                      ->setIconName(L"water_still")
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_WATER, IDS_DESC_WATER));

    registerBlock(10, L"flowing_lava",
                  (new DynamicLiquidBlock(Material::LAVA))
                      ->setDestroyTime(100.0f)
                      ->setLightEmission(1.0f)
                      ->setIconName(L"lava_flow")
                      ->setLightBlock(14)
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_LAVA, IDS_DESC_LAVA));

    registerBlock(11, L"lava",
                  (new StaticLiquidBlock(Material::LAVA))
                      ->setDestroyTime(100.0f)
                      ->setLightEmission(1.0f)
                      ->setIconName(L"lava_still")
                      ->setLightBlock(14)
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_LAVA, IDS_DESC_LAVA));

    registerBlock(12, L"sand",
                  (new SandBlock())
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::SAND)
                      ->setIconName(L"sand")
                      ->setNameAndDescriptionId(IDS_TILE_SAND, IDS_DESC_SAND)
                      ->setBaseItemTypeAndMaterial(13, 13));

    registerBlock(13, L"gravel",
                  (new GravelBlock())
                      ->setDestroyTime(0.6f)
                      ->setSoundType(SoundType::GRAVEL)
                      ->setIconName(L"gravel")
                      ->setNameAndDescriptionId(IDS_TILE_GRAVEL, IDS_DESC_GRAVEL));

    registerBlock(14, L"gold_ore",
                  (new OreBlock())
                      ->setDestroyTime(3.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setIconName(L"gold_ore")
                      ->setNameAndDescriptionId(IDS_TILE_ORE_GOLD, IDS_DESC_ORE_GOLD));

    registerBlock(15, L"iron_ore",
                  (new OreBlock())
                      ->setDestroyTime(3.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setIconName(L"iron_ore")
                      ->setNameAndDescriptionId(IDS_TILE_ORE_IRON, IDS_DESC_ORE_IRON));

    registerBlock(16, L"coal_ore",
                  (new OreBlock())
                      ->setDestroyTime(3.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setIconName(L"coal_ore")
                      ->setNameAndDescriptionId(IDS_TILE_ORE_COAL, IDS_DESC_ORE_COAL));

    registerBlock(17, L"log",
                  (new OldLogBlock())
                      ->setIconName(L"log")
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_LOG, IDS_DESC_LOG));

    registerBlock(18, L"leaves",
                  (new OldLeafBlock())
                      ->setIconName(L"leaves")
                      ->sendBlockData(3)
                      ->setNameAndDescriptionId(IDS_TILE_LEAVES, IDS_DESC_LEAVES));

    registerBlock(19, L"sponge",
                  (new SpongeBlock())
                      ->setDestroyTime(0.6f)
                      ->setSoundType(SoundType::GRASS)
                      ->setIconName(L"sponge")
                      ->setNameAndDescriptionId(IDS_TILE_SPONGE, IDS_DESC_SPONGE));

    registerBlock(20, L"glass",
                  (new GlassBlock(Material::GLASS, 0))
                      ->setDestroyTime(0.3f)
                      ->setSoundType(SoundType::GLASS)
                      ->setIconName(L"glass")
                      ->setNameAndDescriptionId(IDS_TILE_GLASS, IDS_DESC_GLASS)
                      ->setBaseItemTypeAndMaterial(42, 34));

    registerBlock(21, L"lapis_ore",
                  (new OreBlock())
                      ->setDestroyTime(3.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setIconName(L"lapis_ore")
                      ->setNameAndDescriptionId(IDS_TILE_ORE_LAPIS, IDS_DESC_ORE_LAPIS));

    registerBlock(22, L"lapis_block",
                  (new BlockImpl(Material::METAL))
                      ->setDestroyTime(3.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setIconName(L"lapis_block")
                      ->setNameAndDescriptionId(IDS_TILE_BLOCK_LAPIS, IDS_DESC_BLOCK_LAPIS)
                      ->setBaseItemTypeAndMaterial(32, 10));

    registerBlock(23, L"dispenser",
                  (new DispenserBlock())
                      ->setDestroyTime(3.5f)
                      ->setSoundType(SoundType::STONE)
                      ->setIconName(L"dispenser")
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_DISPENSER, IDS_DESC_DISPENSER)
                      ->setBaseItemTypeAndMaterial(51, 2));

    Block* sandstone = (new SandStoneBlock())
                           ->setSoundType(SoundType::STONE)
                           ->setIconName(L"sandstone")
                           ->setDestroyTime(0.8f)
                           ->sendBlockData(15)
                           ->sendBlockData(15)
                           ->setNameAndDescriptionId(IDS_TILE_SANDSTONE, IDS_DESC_SANDSTONE)
                           ->setBaseItemTypeAndMaterial(13, 13);
    registerBlock(24, L"sandstone", sandstone);

    registerBlock(25, L"noteblock",
                  (new NoteBlock())
                      ->setSoundType(SoundType::WOOD)
                      ->setDestroyTime(0.8f)
                      ->setIconName(L"noteblock")
                      ->sendBlockData(15)
                      ->setBaseItemTypeAndMaterial(56, 0)
                      ->setNameAndDescriptionId(IDS_TILE_MUSIC_BLOCK, IDS_DESC_NOTEBLOCK));

    registerBlock(26, L"bed",
                  (new BedBlock())
                      ->setSoundType(SoundType::WOOD)
                      ->setDestroyTime(0.2f)
                      ->setIconName(L"bed")
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_BED, IDS_DESC_BED));

    registerBlock(27, L"golden_rail",
                  (new PoweredRailBlock())
                      ->setDestroyTime(0.7f)
                      ->setSoundType(SoundType::METAL)
                      ->setIconName(L"rail_golden")
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_GOLDEN_RAIL, IDS_DESC_POWEREDRAIL)
                      ->setBaseItemTypeAndMaterial(12, 9)
                      ->disableMipmap());

    registerBlock(28, L"detector_rail",
                  (new DetectorRailBlock())
                      ->setDestroyTime(0.7f)
                      ->setSoundType(SoundType::METAL)
                      ->setIconName(L"rail_detector")
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_DETECTOR_RAIL, IDS_DESC_DETECTORRAIL)
                      ->disableMipmap()
                      ->setBaseItemTypeAndMaterial(12, 8));

    registerBlock(29, L"sticky_piston",
                  (new PistonBaseBlock(1))
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_PISTON_STICK_BASE, IDS_DESC_STICKY_PISTON)
                      ->setBaseItemTypeAndMaterial(27, 28)
                      ->setIconName(L"pistonStickyBase"));

    registerBlock(30, L"web",
                  (new WebBlock())
                      ->setLightBlock(1)
                      ->setDestroyTime(4.0f)
                      ->setIconName(L"web")
                      ->setNameAndDescriptionId(IDS_TILE_WEB, IDS_DESC_WEB)
                      ->disableMipmap());

    registerBlock(31, L"tallgrass",
                  (new TallGrassBlock())
                      ->setDestroyTime(0.0f)
                      ->setSoundType(SoundType::GRASS)
                      ->setIconName(L"tallgrass")
                      ->setNameAndDescriptionId(IDS_TILE_TALL_GRASS, IDS_DESC_TALL_GRASS)
                      ->disableMipmap());

    registerBlock(32, L"deadbush",
                  (new DeadBushBlock())
                      ->setDestroyTime(0.0f)
                      ->setSoundType(SoundType::GRASS)
                      ->setIconName(L"deadbush")
                      ->setNameAndDescriptionId(IDS_TILE_DEAD_BUSH, IDS_DESC_DEAD_BUSH)
                      ->disableMipmap());

    registerBlock(33, L"piston",
                  (new PistonBaseBlock(0))
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_PISTON_BASE, IDS_DESC_PISTON)
                      ->setBaseItemTypeAndMaterial(27, 28)
                      ->setIconName(L"pistonBase"));

    registerBlock(
        34, L"piston_head",
        (new PistonHeadBlock())->setNameAndDescriptionId(IDS_TILE_PISTON_BASE, IDS_DESC_PISTON)->sendBlockData(15));

    registerBlock(35, L"wool",
                  (new ColoredBlock(Material::WOOL))
                      ->setDestroyTime(0.8f)
                      ->setSoundType(SoundType::CLOTH)
                      ->setIconName(L"wool_colored")
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_CLOTH, IDS_DESC_WOOL)
                      ->setBaseItemTypeAndMaterial(49, 55));

    registerBlock(
        36, L"piston_extension",
        (new PistonMagicBlock())->setIconName(L"pistonBase")->setNameAndDescriptionId(IDS_TILE_PISTON_BASE, -1));

    registerBlock(37, L"yellow_flower",
                  (new YellowFlowerBlock())
                      ->setDestroyTime(0.0f)
                      ->setSoundType(SoundType::GRASS)
                      ->setIconName(L"flower_dandelion")
                      ->setNameAndDescriptionId(IDS_TILE_FLOWER, IDS_DESC_FLOWER)
                      ->disableMipmap());

    registerBlock(38, L"red_flower",
                  (new RedFlowerBlock())
                      ->setDestroyTime(0.0f)
                      ->setSoundType(SoundType::GRASS)
                      ->setIconName(L"flower_rose")
                      ->setNameAndDescriptionId(IDS_TILE_POPPY, IDS_DESC_FLOWER)
                      ->disableMipmap());

    Block* brownMushroom = (new MushroomBlock())
                               ->setDestroyTime(0.0f)
                               ->setSoundType(SoundType::GRASS)
                               ->setLightEmission(0.125f)
                               ->setIconName(L"mushroom_brown")
                               ->setNameAndDescriptionId(IDS_TILE_MUSHROOM, IDS_DESC_MUSHROOM)
                               ->disableMipmap();
    registerBlock(39, L"brown_mushroom", brownMushroom);

    Block* redMushroom = (new MushroomBlock())
                             ->setDestroyTime(0.0f)
                             ->setSoundType(SoundType::GRASS)
                             ->setIconName(L"mushroom_red")
                             ->setNameAndDescriptionId(IDS_TILE_MUSHROOM, IDS_DESC_MUSHROOM)
                             ->disableMipmap();
    registerBlock(40, L"red_mushroom", redMushroom);

    registerBlock(41, L"gold_block",
                  (new BlockImpl(Material::METAL, MaterialColor::GOLD))
                      ->setDestroyTime(3.0f)
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::METAL)
                      ->setIconName(L"gold_block")
                      ->setNameAndDescriptionId(IDS_TILE_BLOCK_GOLD, IDS_DESC_BLOCK_GOLD)
                      ->setBaseItemTypeAndMaterial(32, 4));

    registerBlock(42, L"iron_block",
                  (new BlockImpl(Material::METAL, MaterialColor::METAL))
                      ->setDestroyTime(5.0f)
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::METAL)
                      ->setIconName(L"iron_block")
                      ->setNameAndDescriptionId(IDS_TILE_BLOCK_IRON, IDS_DESC_BLOCK_IRON)
                      ->setBaseItemTypeAndMaterial(32, 3));

    registerBlock(43, L"double_stone_slab",
                  (new FullStoneSlabBlock())
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setIconName(L"stoneSlab")
                      ->setNameAndDescriptionId(IDS_TILE_STONESLAB, IDS_DESC_SLAB)
                      ->setBaseItemTypeAndMaterial(21, 2));

    registerBlock(44, L"stone_slab",
                  (new HalfStoneSlabBlock())
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setIconName(L"stoneSlab")
                      ->setNameAndDescriptionId(IDS_TILE_STONESLAB, IDS_DESC_HALFSLAB)
                      ->setBaseItemTypeAndMaterial(22, 2));

    Block* brickBlock = (new BlockImpl(Material::STONE, MaterialColor::COLOR_RED))
                            ->setDestroyTime(2.0f)
                            ->setExplodeable(10.0f)
                            ->setSoundType(SoundType::STONE)
                            ->setIconName(L"brick")
                            ->setNameAndDescriptionId(IDS_TILE_BRICK, IDS_DESC_BRICK)
                            ->setBaseItemTypeAndMaterial(13, 14);
    registerBlock(45, L"brick_block", brickBlock);

    registerBlock(46, L"tnt",
                  (new TntBlock())
                      ->setDestroyTime(0.0f)
                      ->setSoundType(SoundType::GRASS)
                      ->setIconName(L"tnt")
                      ->setNameAndDescriptionId(IDS_TILE_TNT, IDS_DESC_TNT)
                      ->setBaseItemTypeAndMaterial(28, 0));

    registerBlock(47, L"bookshelf",
                  (new BookshelfBlock())
                      ->setDestroyTime(1.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->setIconName(L"bookshelf")
                      ->setIconName(L"bookshelf")
                      ->setNameAndDescriptionId(IDS_TILE_BOOKSHELF, IDS_DESC_BOOKSHELF)
                      ->setBaseItemTypeAndMaterial(46, 52));

    registerBlock(48, L"mossy_cobblestone",
                  (new BlockImpl(Material::STONE))
                      ->setIconName(L"cobblestone_mossy")
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_STONE_MOSS, IDS_DESC_MOSS_STONE)
                      ->setBaseItemTypeAndMaterial(14, 2));

    registerBlock(49, L"obsidian",
                  (new ObsidianBlock())
                      ->setDestroyTime(50.0f)
                      ->setExplodeable(2000.0f)
                      ->setTicking(1)
                      ->setSoundType(SoundType::STONE)
                      ->setIconName(L"obsidian")
                      ->setNameAndDescriptionId(IDS_TILE_OBSIDIAN, IDS_DESC_OBSIDIAN)
                      ->setBaseItemTypeAndMaterial(13, 2));

    registerBlock(50, L"torch",
                  (new TorchBlock())
                      ->setDestroyTime(0.0f)
                      ->setLightEmission(0.9375f)
                      ->setSoundType(SoundType::WOOD)
                      ->setIconName(L"torch_on")
                      ->setNameAndDescriptionId(IDS_TILE_TORCH, IDS_DESC_TORCH)
                      ->disableMipmap()
                      ->setBaseItemTypeAndMaterial(23, 1));

    registerBlock(51, L"fire",
                  (new FireBlock())
                      ->setDestroyTime(0.0f)
                      ->setLightEmission(1.0f)
                      ->setSoundType(SoundType::CLOTH)
                      ->setIconName(L"fire")
                      ->setNotCollectStatistics()
                      ->setNameAndDescriptionId(IDS_TILE_FIRE, -1)
                      ->disableMipmap());

    registerBlock(52, L"mob_spawner",
                  (new MobSpawnerBlock())
                      ->setDestroyTime(5.0f)
                      ->setSoundType(SoundType::METAL)
                      ->setIconName(L"mob_spawner")
                      ->setNotCollectStatistics()
                      ->setNameAndDescriptionId(IDS_TILE_MOB_SPAWNER, IDS_DESC_MOB_SPAWNER));

    registerBlock(
        53, L"oak_stairs",
        (new StairBlock(planks->defaultBlockState()->setValue(PlanksBlock::VARIANT, PlanksBlock::Variant::OAK)))
            ->setIconName(L"stairsWood")
            ->sendBlockData(15)
            ->setNameAndDescriptionId(IDS_TILE_STAIRS_WOOD, IDS_DESC_STAIRS)
            ->setBaseItemTypeAndMaterial(16, 1));

    registerBlock(54, L"chest",
                  (new ChestBlock(ChestBlock::TYPE_SINGLE))
                      ->setDestroyTime(2.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->setIconName(L"chest")
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_CHEST, IDS_DESC_CHEST)
                      ->setBaseItemTypeAndMaterial(36, 1));

    registerBlock(55, L"redstone_wire",
                  (new RedStoneWireBlock())
                      ->setDestroyTime(0.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setIconName(L"redstone_dust")
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_REDSTONE_DUST, IDS_DESC_REDSTONE_DUST)
                      ->disableMipmap());

    registerBlock(56, L"diamond_ore",
                  (new OreBlock())
                      ->setDestroyTime(3.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setIconName(L"diamond_ore")
                      ->setNameAndDescriptionId(IDS_TILE_ORE_DIAMOND, IDS_DESC_ORE_DIAMOND));

    registerBlock(57, L"diamond_block",
                  (new BlockImpl(Material::METAL, MaterialColor::DIAMOND))
                      ->setDestroyTime(5.0f)
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::METAL)
                      ->setIconName(L"diamond_block")
                      ->setNameAndDescriptionId(IDS_TILE_BLOCK_DIAMOND, IDS_DESC_BLOCK_DIAMOND)
                      ->setBaseItemTypeAndMaterial(32, 5));

    registerBlock(58, L"crafting_table",
                  (new CraftingTableBlock())
                      ->setDestroyTime(2.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->setIconName(L"crafting_table")
                      ->setNameAndDescriptionId(IDS_TILE_WORKBENCH, IDS_DESC_CRAFTINGTABLE)
                      ->setBaseItemTypeAndMaterial(30, 1));

    registerBlock(59, L"wheat",
                  (new CropBlock())
                      ->setIconName(L"wheat")
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_CROPS, IDS_DESC_CROPS)
                      ->disableMipmap());

    Block* farmland = (new FarmBlock())
                          ->setDestroyTime(0.6f)
                          ->setSoundType(SoundType::GRAVEL)
                          ->setIconName(L"farmland")
                          ->sendBlockData(15)
                          ->setNameAndDescriptionId(IDS_TILE_FARMLAND, IDS_DESC_FARMLAND);
    registerBlock(60, L"farmland", farmland);

    registerBlock(61, L"furnace",
                  (new FurnaceBlock(false))
                      ->setDestroyTime(3.5f)
                      ->setSoundType(SoundType::STONE)
                      ->setIconName(L"furnace")
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_FURNACE, IDS_DESC_FURNACE)
                      ->setBaseItemTypeAndMaterial(30, 2));

    registerBlock(62, L"lit_furnace",
                  (new FurnaceBlock(true))
                      ->setDestroyTime(3.5f)
                      ->setSoundType(SoundType::STONE)
                      ->setIconName(L"furnace")
                      ->setLightEmission(0.875f)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_FURNACE, IDS_DESC_FURNACE));

    registerBlock(63, L"standing_sign",
                  (new StandingSignBlock())
                      ->setDestroyTime(1.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setIconName(L"sign")
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_SIGN, IDS_DESC_SIGN));

    registerBlock(64, L"wooden_door",
                  (new DoorBlock(Material::WOOD))
                      ->setDestroyTime(3.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setIconName(L"door_wood")
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_DOOR_OAKWOOD, IDS_DESC_DOOR_WOOD));

    registerBlock(65, L"ladder",
                  (new LadderBlock())
                      ->setDestroyTime(0.4f)
                      ->setSoundType(SoundType::LADDER)
                      ->setIconName(L"ladder")
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_LADDER, IDS_DESC_LADDER)
                      ->disableMipmap()
                      ->setBaseItemTypeAndMaterial(48, 1));

    registerBlock(66, L"rail",
                  (new RailBlock())
                      ->setDestroyTime(0.7f)
                      ->setSoundType(SoundType::METAL)
                      ->setIconName(L"rail_normal")
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_RAIL, IDS_DESC_RAIL)
                      ->setBaseItemTypeAndMaterial(12, 9)
                      ->disableMipmap());

    registerBlock(67, L"stone_stairs",
                  (new StairBlock(cobblestone->defaultBlockState()))
                      ->setIconName(L"stairsStone")
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_STAIRS_STONE, IDS_DESC_STAIRS)
                      ->setBaseItemTypeAndMaterial(16, 2));

    registerBlock(68, L"wall_sign",
                  (new WallSignBlock())
                      ->setDestroyTime(1.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setIconName(L"sign")
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_SIGN, IDS_DESC_SIGN));

    registerBlock(69, L"lever",
                  (new LeverBlock())
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->setIconName(L"lever")
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_LEVER, IDS_DESC_LEVER)
                      ->setBaseItemTypeAndMaterial(45, 1));

    registerBlock(70, L"stone_pressure_plate",
                  (new PressurePlateBlock(L"stone", Material::STONE, PressurePlateBlock::MOBS))
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::STONE)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_PRESSURE_PLATE_STONE, IDS_DESC_PRESSURE_PLATE_STONE)
                      ->setBaseItemTypeAndMaterial(15, 2));

    registerBlock(71, L"iron_door",
                  (new DoorBlock(Material::METAL))
                      ->setIconName(L"door_iron")
                      ->setDestroyTime(5.0f)
                      ->setSoundType(SoundType::METAL)
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_DOOR_IRON, IDS_DESC_DOOR_IRON));

    registerBlock(72, L"wooden_pressure_plate",
                  (new PressurePlateBlock(L"planks_oak", Material::WOOD, PressurePlateBlock::EVERYTHING))
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_PRESSURE_PLATE_WOOD, IDS_DESC_PRESSURE_PLATE_WOOD)
                      ->setBaseItemTypeAndMaterial(15, 1));

    registerBlock(73, L"redstone_ore",
                  (new RedStoneOreBlock(0))
                      ->setIconName(L"redstone_ore")
                      ->setDestroyTime(3.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::STONE)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_ORE_REDSTONE, IDS_DESC_ORE_REDSTONE));

    registerBlock(74, L"lit_redstone_ore",
                  (new RedStoneOreBlock(1))
                      ->setIconName(L"redstone_ore")
                      ->setLightEmission(0.625f)
                      ->setDestroyTime(3.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::STONE)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_ORE_REDSTONE, IDS_DESC_ORE_REDSTONE));

    registerBlock(75, L"unlit_redstone_torch",
                  (new RedstoneTorchBlock(0))
                      ->setIconName(L"redstone_torch_off")
                      ->setDestroyTime(0.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_NOT_GATE, IDS_DESC_REDSTONETORCH)
                      ->disableMipmap());

    registerBlock(76, L"redstone_torch",
                  (new RedstoneTorchBlock(1))
                      ->setIconName(L"redstone_torch_on")
                      ->setDestroyTime(0.0f)
                      ->setLightEmission(0.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_NOT_GATE, IDS_DESC_REDSTONETORCH)
                      ->disableMipmap());

    registerBlock(77, L"stone_button",
                  (new StoneButtonBlock())
                      ->setIconName(L"stone")
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::STONE)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_BUTTON_STONE, IDS_DESC_BUTTON)
                      ->setBaseItemTypeAndMaterial(35, 2));

    registerBlock(78, L"snow_layer",
                  (new SnowLayerBlock())
                      ->setIconName(L"snow")
                      ->setDestroyTime(0.1f)
                      ->setSoundType(SoundType::SNOW)
                      ->sendBlockData(15)
                      ->setLightBlock(0)
                      ->setNameAndDescriptionId(IDS_TILE_SNOW, IDS_DESC_TOP_SNOW)
                      ->setBaseItemTypeAndMaterial(13, 16));

    registerBlock(79, L"ice",
                  (new IceBlock())
                      ->setIconName(L"ice")
                      ->setDestroyTime(0.5f)
                      ->setLightBlock(3)
                      ->setSoundType(SoundType::GLASS)
                      ->setSemiTransparent()
                      ->setNameAndDescriptionId(IDS_TILE_ICE, IDS_DESC_ICE));

    registerBlock(80, L"snow",
                  (new SnowBlock())
                      ->setIconName(L"snow")
                      ->setDestroyTime(0.2f)
                      ->setSoundType(SoundType::CLOTH)
                      ->setNameAndDescriptionId(IDS_TILE_SNOW, IDS_DESC_SNOW)
                      ->setBaseItemTypeAndMaterial(13, 16));

    registerBlock(81, L"cactus",
                  (new CactusBlock())
                      ->setIconName(L"cactus")
                      ->setDestroyTime(0.4f)
                      ->setSoundType(SoundType::CLOTH)
                      ->setNameAndDescriptionId(IDS_TILE_CACTUS, IDS_DESC_CACTUS)
                      ->disableMipmap());

    registerBlock(82, L"clay",
                  (new ClayBlock())
                      ->setIconName(L"clay")
                      ->setDestroyTime(0.6f)
                      ->setSoundType(SoundType::GRAVEL)
                      ->setNameAndDescriptionId(IDS_TILE_CLAY, IDS_DESC_CLAY_TILE)
                      ->setBaseItemTypeAndMaterial(13, 15));

    registerBlock(83, L"reeds",
                  (new ReedsBlock())
                      ->setIconName(L"reeds")
                      ->setDestroyTime(0.0f)
                      ->setSoundType(SoundType::GRASS)
                      ->setNotCollectStatistics()
                      ->setNameAndDescriptionId(IDS_TILE_REEDS, IDS_DESC_REEDS)
                      ->disableMipmap());

    registerBlock(84, L"jukebox",
                  (new JukeboxBlock())
                      ->setIconName(L"jukebox")
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::STONE)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_JUKEBOX, IDS_DESC_JUKEBOX)
                      ->setBaseItemTypeAndMaterial(53, 0));

    registerBlock(85, L"fence",
                  (new FenceBlock(L"planks_oak", Material::WOOD, PlanksBlock::Variant::OAK->getMaterialColor()))
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_FENCE_OAKWOOD, IDS_DESC_FENCE)
                      ->setBaseItemTypeAndMaterial(29, 1));

    registerBlock(86, L"carved_pumpkin",
                  (new PumpkinBlock())
                      ->setIconName(L"pumpkin")
                      ->setDestroyTime(1.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_CARVED_PUMPKIN, IDS_DESC_CARVED_PUMPKIN));

    Block* pumpkinBlock = (new PumpkinBlock())
                              ->setIconName(L"pumpkin")
                              ->setDestroyTime(1.0f)
                              ->setSoundType(SoundType::WOOD)
                              ->sendBlockData(15)
                              ->setNameAndDescriptionId(IDS_TILE_PUMPKIN, IDS_DESC_PUMPKIN);
    registerBlock(257, L"pumpkin", pumpkinBlock);

    registerBlock(87, L"netherrack",
                  (new NetherrackBlock())
                      ->setIconName(L"netherrack")
                      ->setDestroyTime(0.4f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_HELL_ROCK, IDS_DESC_HELL_ROCK));

    registerBlock(88, L"soul_sand",
                  (new SoulSandBlock())
                      ->setIconName(L"soul_sand")
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::SAND)
                      ->setNameAndDescriptionId(IDS_TILE_HELL_SAND, IDS_DESC_HELL_SAND));

    registerBlock(89, L"glowstone",
                  (new Glowstoneblock(Material::GLASS))
                      ->setIconName(L"glowstone")
                      ->setDestroyTime(0.3f)
                      ->setSoundType(SoundType::GLASS)
                      ->setLightEmission(1.0f)
                      ->setNameAndDescriptionId(IDS_TILE_LIGHT_GEM, IDS_DESC_GLOWSTONE)
                      ->setBaseItemTypeAndMaterial(23, 23));

    registerBlock(90, L"portal",
                  (new PortalBlock())
                      ->setDestroyTime(-1.0f)
                      ->setIconName(L"portal")
                      ->setSoundType(SoundType::GLASS)
                      ->setLightEmission(0.75f)
                      ->setNameAndDescriptionId(IDS_TILE_PORTAL, IDS_DESC_PORTAL));

    registerBlock(91, L"lit_pumpkin",
                  (new PumpkinBlock())
                      ->setIconName(L"pumpkin")
                      ->setDestroyTime(1.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setLightEmission(1.0f)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_LIT_PUMPKIN, IDS_DESC_JACKOLANTERN)
                      ->setBaseItemTypeAndMaterial(23, 22));

    registerBlock(92, L"cake",
                  (new CakeBlock())
                      ->setIconName(L"cake")
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::CLOTH)
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_CAKE, IDS_DESC_CAKE));

    registerBlock(93, L"unpowered_repeater",
                  (new RepeaterBlock(0))
                      ->setIconName(L"repeater_off")
                      ->setDestroyTime(0.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_ITEM_DIODE, IDS_DESC_REDSTONEREPEATER)
                      ->disableMipmap()
                      ->setBaseItemTypeAndMaterial(59, 0));

    registerBlock(94, L"powered_repeater",
                  (new RepeaterBlock(1))
                      ->setIconName(L"repeater_on")
                      ->setDestroyTime(0.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_ITEM_DIODE, IDS_DESC_REDSTONEREPEATER)
                      ->disableMipmap()
                      ->setBaseItemTypeAndMaterial(59, 0));

    registerBlock(95, L"stained_glass",
                  (new StainedGlassBlock(Material::GLASS))
                      ->setIconName(L"glass")
                      ->setDestroyTime(0.3f)
                      ->setSoundType(SoundType::GLASS)
                      ->setNameAndDescriptionId(IDS_TILE_STAINED_GLASS, IDS_DESC_STAINED_GLASS)
                      ->setSemiTransparent()
                      ->setBaseItemTypeAndMaterial(42, 34));

    registerBlock(96, L"trapdoor",
                  (new TrapDoorBlock(Material::WOOD, MaterialColor::WOOD))
                      ->setIconName(L"trapdoor")
                      ->setDestroyTime(3.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_TRAPDOOR_WOOD, IDS_DESC_TRAPDOOR)
                      ->setBaseItemTypeAndMaterial(6, 25));

    registerBlock(97, L"monster_egg",
                  (new MonsterEggBlock())
                      ->setIconName(L"monsterStoneEgg")
                      ->setDestroyTime(0.75f)
                      ->setNameAndDescriptionId(IDS_TILE_STONE_SILVERFISH, IDS_DESC_STONE_SILVERFISH));

    Block* stonebrick = (new StoneBrickBlock())
                            ->setIconName(L"stonebrick")
                            ->setDestroyTime(1.5f)
                            ->setExplodeable(10.0f)
                            ->setSoundType(SoundType::STONE)
                            ->setNameAndDescriptionId(IDS_TILE_STONE_BRICK_SMOOTH, IDS_DESC_STONE_BRICK_SMOOTH)
                            ->setBaseItemTypeAndMaterial(14, 31);
    registerBlock(98, L"stonebrick", stonebrick);

    registerBlock(99, L"brown_mushroom_block",
                  (new HugeMushroomBlock(Material::WOOD, MaterialColor::DIRT, brownMushroom))
                      ->setIconName(L"mushroom_block")
                      ->setDestroyTime(0.2f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_HUGE_MUSHROOM_1, IDS_DESC_MUSHROOM_BLOCK)
                      ->sendBlockData(15));

    registerBlock(100, L"red_mushroom_block",
                  (new HugeMushroomBlock(Material::WOOD, MaterialColor::COLOR_RED, redMushroom))
                      ->setIconName(L"mushroom_block")
                      ->setDestroyTime(0.2f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_HUGE_MUSHROOM_2, IDS_DESC_MUSHROOM_BLOCK)
                      ->sendBlockData(15));

    registerBlock(101, L"iron_bars",
                  (new ThinFenceBlock(L"iron_bars", L"iron_bars", Material::METAL, true))
                      ->setDestroyTime(5.0f)
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::METAL)
                      ->setNameAndDescriptionId(IDS_TILE_IRON_FENCE, IDS_DESC_IRON_FENCE)
                      ->setBaseItemTypeAndMaterial(29, 3));

    registerBlock(102, L"glass_pane",
                  (new ThinFenceBlock(L"glass", L"glass_pane_top", Material::GLASS, false))
                      ->setDestroyTime(0.3f)
                      ->setSoundType(SoundType::GLASS)
                      ->setNameAndDescriptionId(IDS_TILE_THIN_GLASS, IDS_DESC_THIN_GLASS)
                      ->setBaseItemTypeAndMaterial(42, 34));

    Block* melonBlock = (new MelonBlock())
                            ->setIconName(L"melon")
                            ->setDestroyTime(1.0f)
                            ->setSoundType(SoundType::WOOD)
                            ->setNameAndDescriptionId(IDS_TILE_MELON, IDS_DESC_MELON_BLOCK);
    registerBlock(103, L"melon_block", melonBlock);

    registerBlock(104, L"pumpkin_stem",
                  (new StemBlock(pumpkinBlock))
                      ->setIconName(L"pumpkin_stem")
                      ->setDestroyTime(0.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_PUMPKIN_STEM, -1)
                      ->disableMipmap());

    registerBlock(105, L"melon_stem",
                  (new StemBlock(melonBlock))
                      ->setIconName(L"melon_stem")
                      ->setDestroyTime(0.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_MELON_STEM, -1)
                      ->disableMipmap());

    registerBlock(106, L"vine",
                  (new VineBlock())
                      ->setIconName(L"vine")
                      ->setDestroyTime(0.2f)
                      ->setSoundType(SoundType::GRASS)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_VINE, IDS_DESC_VINE)
                      ->disableMipmap());

    registerBlock(107, L"fence_gate",
                  (new FenceGateBlock(L"planks_oak", PlanksBlock::Variant::OAK))
                      ->setIconName(L"fenceGate")
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_FENCE_GATE_OAKWOOD, IDS_DESC_FENCE_GATE)
                      ->setBaseItemTypeAndMaterial(47, 1));

    registerBlock(108, L"brick_stairs",
                  (new StairBlock(brickBlock->defaultBlockState()))
                      ->setIconName(L"stairsBrick")
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_STAIRS_BRICKS, IDS_DESC_STAIRS)
                      ->setBaseItemTypeAndMaterial(16, 14));

    registerBlock(109, L"stone_brick_stairs",
                  (new StairBlock(stonebrick->defaultBlockState()->setValue(StoneBrickBlock::VARIANT,
                                                                            StoneBrickBlock::Variant::DEFAULT)))
                      ->setIconName(L"stairsStoneBrickSmooth")
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_STAIRS_STONE_BRICKS_SMOOTH, IDS_DESC_STAIRS)
                      ->setBaseItemTypeAndMaterial(16, 31));

    registerBlock(110, L"mycelium",
                  (new MyceliumBlock())
                      ->setIconName(L"mycelium")
                      ->setDestroyTime(0.6f)
                      ->setSoundType(SoundType::GRASS)
                      ->setNameAndDescriptionId(IDS_TILE_MYCEL, IDS_DESC_MYCEL));

    registerBlock(111, L"waterlily",
                  (new WaterlilyBlock())
                      ->setIconName(L"waterlily")
                      ->setDestroyTime(0.0f)
                      ->setSoundType(SoundType::GRASS)
                      ->setNameAndDescriptionId(IDS_TILE_WATERLILY, IDS_DESC_WATERLILY)
                      ->disableMipmap());

    Block* netherBrick = (new NetherBrickBlock())
                             ->setIconName(L"nether_brick")
                             ->setDestroyTime(2.0f)
                             ->setExplodeable(10.0f)
                             ->setSoundType(SoundType::STONE)
                             ->setNameAndDescriptionId(IDS_TILE_NETHERBRICK, IDS_DESC_NETHERBRICK)
                             ->setBaseItemTypeAndMaterial(13, 32);
    registerBlock(112, L"nether_brick", netherBrick);

    registerBlock(113, L"nether_brick_fence",
                  (new FenceBlock(L"nether_brick", Material::STONE, MaterialColor::NETHER))
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_NETHERFENCE, IDS_DESC_NETHERFENCE)
                      ->setBaseItemTypeAndMaterial(29, 32));

    registerBlock(114, L"nether_brick_stairs",
                  (new StairBlock(netherBrick->defaultBlockState()))
                      ->setIconName(L"stairsNetherBrick")
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_STAIRS_NETHERBRICK, IDS_DESC_STAIRS)
                      ->setBaseItemTypeAndMaterial(16, 32));

    registerBlock(115, L"nether_wart",
                  (new NetherWartBlock())
                      ->setIconName(L"nether_wart")
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_NETHERSTALK, IDS_DESC_NETHERSTALK)
                      ->disableMipmap());

    registerBlock(116, L"enchanting_table",
                  (new EnchantmentTableBlock())
                      ->setIconName(L"enchanting_table")
                      ->setDestroyTime(5.0f)
                      ->setExplodeable(2000.0f)
                      ->setNameAndDescriptionId(IDS_TILE_ENCHANTMENTTABLE, IDS_DESC_ENCHANTMENTTABLE)
                      ->setBaseItemTypeAndMaterial(30, 36));

    registerBlock(117, L"brewing_stand",
                  (new BrewingStandBlock())
                      ->setIconName(L"brewing_stand")
                      ->setDestroyTime(0.5f)
                      ->setLightEmission(0.125f)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_BREWINGSTAND, IDS_DESC_BREWING_STAND)
                      ->setBaseItemTypeAndMaterial(30, 35));

    registerBlock(118, L"cauldron",
                  (new CauldronBlock())
                      ->setIconName(L"cauldron")
                      ->setDestroyTime(2.0f)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_CAULDRON, IDS_DESC_CAULDRON));

    registerBlock(119, L"end_portal",
                  (new EndPortalBlock(Material::PORTAL))
                      ->setDestroyTime(INDESTRUCTIBLE)
                      ->setExplodeable(6000000.0f)
                      ->setNameAndDescriptionId(IDS_TILE_END_PORTAL, IDS_DESC_END_PORTAL));

    registerBlock(120, L"end_portal_frame",
                  (new EndPortalFrameBlock())
                      ->setIconName(L"endframe")
                      ->setSoundType(SoundType::GLASS)
                      ->setLightEmission(0.125f)
                      ->setDestroyTime(INDESTRUCTIBLE)
                      ->sendBlockData(15)
                      ->setExplodeable(6000000.0f)
                      ->setNameAndDescriptionId(IDS_TILE_ENDPORTALFRAME, IDS_DESC_ENDPORTALFRAME));

    registerBlock(121, L"end_stone",
                  (new BlockImpl(Material::STONE, MaterialColor::SAND))
                      ->setIconName(L"end_stone")
                      ->setDestroyTime(3.0f)
                      ->setExplodeable(15.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_WHITESTONE, IDS_DESC_WHITESTONE));

    registerBlock(122, L"dragon_egg",
                  (new DragonEggBlock())
                      ->setDestroyTime(3.0f)
                      ->setIconName(L"dragon_egg")
                      ->setExplodeable(15.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setLightEmission(0.125f)
                      ->setNameAndDescriptionId(IDS_TILE_DRAGONEGG, IDS_DESC_DRAGONEGG));

    registerBlock(123, L"redstone_lamp",
                  (new RedstoneLampBlock(0))
                      ->setIconName(L"redstone_lamp_off")
                      ->setDestroyTime(0.3f)
                      ->setSoundType(SoundType::GLASS)
                      ->setBaseItemTypeAndMaterial(56, 0)
                      ->setNameAndDescriptionId(IDS_TILE_REDSTONE_LIGHT, IDS_DESC_REDSTONE_LIGHT));

    registerBlock(124, L"lit_redstone_lamp",
                  (new RedstoneLampBlock(1))
                      ->setIconName(L"redstone_lamp_on")
                      ->setDestroyTime(0.3f)
                      ->setSoundType(SoundType::GLASS)
                      ->setBaseItemTypeAndMaterial(56, 0)
                      ->setNameAndDescriptionId(IDS_TILE_REDSTONE_LIGHT, IDS_DESC_REDSTONE_LIGHT));

    registerBlock(125, L"double_wooden_slab",
                  (new FullWoodSlabBlock())
                      ->setIconName(L"woodSlab")
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_DESC_WOODSLAB, IDS_DESC_WOODSLAB)
                      ->setBaseItemTypeAndMaterial(21, 1));

    registerBlock(126, L"wooden_slab",
                  (new HalfWoodSlabBlock())
                      ->setIconName(L"woodSlab")
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_DESC_WOODSLAB, IDS_DESC_WOODSLAB)
                      ->setBaseItemTypeAndMaterial(22, 1));

    registerBlock(127, L"cocoa",
                  (new CocoaBlock())
                      ->setIconName(L"cocoa")
                      ->setDestroyTime(0.2f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_COCOA, IDS_DESC_COCOA));

    registerBlock(
        128, L"sandstone_stairs",
        (new StairBlock(sandstone->defaultBlockState()->setValue(SandStoneBlock::TYPE, SandStoneBlock::SMOOTH)))
            ->setIconName(L"stairsSandstone")
            ->sendBlockData(15)
            ->setNameAndDescriptionId(IDS_TILE_STAIRS_SANDSTONE, IDS_DESC_STAIRS)
            ->setBaseItemTypeAndMaterial(16, 13));

    registerBlock(129, L"emerald_ore",
                  (new OreBlock())
                      ->setIconName(L"emerald_ore")
                      ->setDestroyTime(3.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_EMERALDORE, IDS_DESC_EMERALDORE));

    registerBlock(130, L"ender_chest",
                  (new EnderChestBlock())
                      ->setDestroyTime(22.5f)
                      ->setIconName(L"enderChest")
                      ->setExplodeable(1000.0f)
                      ->setSoundType(SoundType::STONE)
                      ->sendBlockData(15)
                      ->setLightEmission(0.5f)
                      ->setNameAndDescriptionId(IDS_TILE_ENDERCHEST, IDS_DESC_ENDERCHEST)
                      ->setBaseItemTypeAndMaterial(36, 33));

    registerBlock(131, L"tripwire_hook",
                  (new TripWireHookBlock())
                      ->setIconName(L"trip_wire_source")
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_TRIPWIRE_SOURCE, IDS_DESC_TRIPWIRE_SOURCE)
                      ->setBaseItemTypeAndMaterial(45, 0));

    registerBlock(132, L"tripwire",
                  (new TripWireBlock())
                      ->setIconName(L"trip_wire")
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_TRIPWIRE, IDS_DESC_TRIPWIRE)
                      ->disableMipmap());

    registerBlock(133, L"emerald_block",
                  (new BlockImpl(Material::METAL, MaterialColor::EMERALD))
                      ->setIconName(L"emerald_block")
                      ->setDestroyTime(5.0f)
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::METAL)
                      ->setNameAndDescriptionId(IDS_TILE_EMERALDBLOCK, IDS_DESC_EMERALDBLOCK)
                      ->setBaseItemTypeAndMaterial(32, 44));

    registerBlock(
        134, L"spruce_stairs",
        (new StairBlock(planks->defaultBlockState()->setValue(PlanksBlock::VARIANT, PlanksBlock::Variant::SPRUCE)))
            ->setIconName(L"stairsWoodSpruce")
            ->sendBlockData(15)
            ->setNameAndDescriptionId(IDS_TILE_STAIRS_SPRUCEWOOD, IDS_DESC_STAIRS)
            ->setBaseItemTypeAndMaterial(16, 39));

    registerBlock(
        135, L"birch_stairs",
        (new StairBlock(planks->defaultBlockState()->setValue(PlanksBlock::VARIANT, PlanksBlock::Variant::BIRCH)))
            ->setIconName(L"stairsWoodBirch")
            ->sendBlockData(15)
            ->setNameAndDescriptionId(IDS_TILE_STAIRS_BIRCHWOOD, IDS_DESC_STAIRS)
            ->setBaseItemTypeAndMaterial(16, 40));

    registerBlock(
        136, L"jungle_stairs",
        (new StairBlock(planks->defaultBlockState()->setValue(PlanksBlock::VARIANT, PlanksBlock::Variant::JUNGLE)))
            ->setIconName(L"stairsWoodJungle")
            ->sendBlockData(15)
            ->setNameAndDescriptionId(IDS_TILE_STAIRS_JUNGLEWOOD, IDS_DESC_STAIRS)
            ->setBaseItemTypeAndMaterial(16, 41));

    registerBlock(137, L"command_block",
                  (new CommandBlock(MaterialColor::COLOR_BROWN, CommandBlock::IMPULSE))
                      ->setIconName(L"command_block")
                      ->setIndestructible()
                      ->setExplodeable(6000000.0f)
                      ->setNameAndDescriptionId(IDS_TILE_COMMAND_BLOCK, IDS_DESC_COMMAND_BLOCK));

    registerBlock(138, L"beacon",
                  (new BeaconBlock())
                      ->setIconName(L"beacon")
                      ->setLightEmission(1.0f)
                      ->setSoundType(SoundType::GLASS)
                      ->setNameAndDescriptionId(IDS_TILE_BEACON, IDS_DESC_BEACON)
                      ->setBaseItemTypeAndMaterial(61, 34));

    registerBlock(139, L"cobblestone_wall",
                  (new WallBlock(cobblestone))
                      ->setIconName(L"cobbleWall")
                      ->setNameAndDescriptionId(IDS_TILE_COBBLESTONE_WALL, IDS_DESC_COBBLESTONE_WALL)
                      ->setBaseItemTypeAndMaterial(29, 2));

    registerBlock(140, L"flower_pot",
                  (new FlowerPotBlock())
                      ->setDestroyTime(0.0f)
                      ->setIconName(L"flower_pot")
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_FLOWERPOT, IDS_DESC_FLOWERPOT));

    registerBlock(141, L"carrots",
                  (new CarrotBlock())
                      ->setIconName(L"carrots")
                      ->setNameAndDescriptionId(IDS_TILE_CARROTS, IDS_DESC_CARROTS)
                      ->disableMipmap());

    registerBlock(142, L"potatoes",
                  (new PotatoBlock())
                      ->setIconName(L"potatoes")
                      ->setNameAndDescriptionId(IDS_TILE_POTATOES, IDS_DESC_POTATO)
                      ->disableMipmap());

    registerBlock(143, L"wooden_button",
                  (new WoodButtonBlock())
                      ->setIconName(L"planks_oak")
                      ->setBaseItemTypeAndMaterial(35, 1)
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_BUTTON, IDS_DESC_BUTTON));

    registerBlock(144, L"skull",
                  (new SkullBlock())
                      ->setIconName(L"skull")
                      ->setDestroyTime(1.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_SKULL, IDS_DESC_SKULL));

    registerBlock(145, L"anvil",
                  (new AnvilBlock())
                      ->setIconName(L"anvil")
                      ->setBaseItemTypeAndMaterial(30, 3)
                      ->setDestroyTime(5.0f)
                      ->setSoundType(SoundType::ANVIL)
                      ->setExplodeable(2000.0f)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_ANVIL, IDS_DESC_ANVIL));

    registerBlock(146, L"trapped_chest",
                  (new ChestBlock(ChestBlock::TYPE_LEFT))
                      ->setBaseItemTypeAndMaterial(36, 25)
                      ->setDestroyTime(2.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_CHEST_TRAP, IDS_DESC_CHEST_TRAP));

    registerBlock(147, L"light_weighted_pressure_plate",
                  (new WeightedPressurePlateBlock(L"gold_block", Material::METAL, 15, MaterialColor::GOLD))
                      ->setBaseItemTypeAndMaterial(15, 4)
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_WEIGHTED_PLATE_LIGHT, IDS_DESC_WEIGHTED_PLATE_LIGHT));

    registerBlock(148, L"heavy_weighted_pressure_plate",
                  (new WeightedPressurePlateBlock(L"iron_block", Material::METAL, 150, MaterialColor::METAL))
                      ->setBaseItemTypeAndMaterial(15, 3)
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_WEIGHTED_PLATE_HEAVY, IDS_DESC_WEIGHTED_PLATE_HEAVY));

    registerBlock(149, L"unpowered_comparator",
                  (new ComparatorBlock(0))
                      ->setIconName(L"comparator_off")
                      ->setDestroyTime(0.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_COMPARATOR, IDS_DESC_COMPARATOR)
                      ->disableMipmap()
                      ->setBaseItemTypeAndMaterial(59, 0));

    registerBlock(150, L"powered_comparator",
                  (new ComparatorBlock(1))
                      ->setIconName(L"comparator_on")
                      ->setDestroyTime(0.0f)
                      ->setLightEmission(0.625f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_COMPARATOR, IDS_DESC_COMPARATOR)
                      ->disableMipmap()
                      ->setBaseItemTypeAndMaterial(59, 0));

    registerBlock(151, L"daylight_detector",
                  (new DaylightDetectorBlock(0))
                      ->setIconName(L"daylight_detector")
                      ->setNameAndDescriptionId(IDS_TILE_DAYLIGHT_DETECTOR, IDS_DESC_DAYLIGHT_DETECTOR));

    registerBlock(152, L"redstone_block",
                  (new PoweredBlock(Material::METAL, MaterialColor::FIRE))
                      ->setIconName(L"redstone_block")
                      ->setDestroyTime(5.0f)
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::METAL)
                      ->setNameAndDescriptionId(IDS_TILE_REDSTONE_BLOCK, IDS_DESC_REDSTONE_BLOCK)
                      ->setBaseItemTypeAndMaterial(32, 48));

    registerBlock(153, L"quartz_ore",
                  (new OreBlock(MaterialColor::NETHER))
                      ->setIconName(L"quartz_ore")
                      ->setDestroyTime(3.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_NETHER_QUARTZ, IDS_DESC_NETHER_QUARTZ_ORE));

    registerBlock(154, L"hopper",
                  (new HopperBlock())
                      ->setIconName(L"hopper")
                      ->setDestroyTime(3.0f)
                      ->setExplodeable(8.0f)
                      ->setSoundType(SoundType::METAL)
                      ->setNameAndDescriptionId(IDS_TILE_HOPPER, IDS_DESC_HOPPER)
                      ->setBaseItemTypeAndMaterial(51, 0));

    Block* quartzBlock = (new QuartzBlock())
                             ->setIconName(L"quartz_block")
                             ->setSoundType(SoundType::STONE)
                             ->setDestroyTime(0.8f)
                             ->setNameAndDescriptionId(IDS_TILE_QUARTZ_BLOCK, IDS_DESC_QUARTZ_BLOCK)
                             ->setBaseItemTypeAndMaterial(14, 45);
    registerBlock(155, L"quartz_block", quartzBlock);

    registerBlock(156, L"quartz_stairs",
                  (new StairBlock(
                       quartzBlock->defaultBlockState()->setValue(QuartzBlock::VARIANT, QuartzBlock::Variant::DEFAULT)))
                      ->setIconName(L"stairsQuartz")
                      ->setNameAndDescriptionId(IDS_TILE_STAIRS_QUARTZ, IDS_DESC_STAIRS)
                      ->setBaseItemTypeAndMaterial(16, 45));

    registerBlock(157, L"activator_rail",
                  (new PoweredRailBlock())
                      ->setIconName(L"rail_activator")
                      ->setDestroyTime(0.7f)
                      ->setSoundType(SoundType::METAL)
                      ->setNameAndDescriptionId(IDS_TILE_ACTIVATOR_RAIL, IDS_DESC_ACTIVATOR_RAIL)
                      ->setBaseItemTypeAndMaterial(12, 9));

    registerBlock(158, L"dropper",
                  (new DropperBlock())
                      ->setIconName(L"dropper")
                      ->setDestroyTime(3.5f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_DROPPER, IDS_DESC_DROPPER)
                      ->setBaseItemTypeAndMaterial(51, 0));

    registerBlock(159, L"stained_hardened_clay",
                  (new StainedHardenedClayBlock())
                      ->setIconName(L"hardened_clay_stained")
                      ->setDestroyTime(1.25f)
                      ->setExplodeable(7.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_STAINED_CLAY, IDS_DESC_STAINED_CLAY)
                      ->setBaseItemTypeAndMaterial(41, 15));

    registerBlock(160, L"stained_glass_pane",
                  (new StainedGlassPaneBlock())
                      ->setIconName(L"glass")
                      ->setDestroyTime(0.3f)
                      ->setSoundType(SoundType::GLASS)
                      ->setSemiTransparent()
                      ->setNameAndDescriptionId(IDS_TILE_STAINED_GLASS_PANE, IDS_DESC_STAINED_GLASS_PANE)
                      ->setBaseItemTypeAndMaterial(42, 34));

    registerBlock(161, L"leaves2",
                  (new NewLeafBlock())
                      ->setIconName(L"leaves2")
                      ->sendBlockData(3)
                      ->setNameAndDescriptionId(IDS_TILE_LEAVES, IDS_DESC_LEAVES));

    registerBlock(162, L"log2",
                  (new NewLogBlock())
                      ->setIconName(L"log")
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_LOG, IDS_DESC_LOG));

    registerBlock(
        163, L"acacia_stairs",
        (new StairBlock(planks->defaultBlockState()->setValue(PlanksBlock::VARIANT, PlanksBlock::Variant::ACACIA)))
            ->setIconName(L"stairsWoodAcacia")
            ->sendBlockData(15)
            ->setNameAndDescriptionId(IDS_TILE_STAIRS_ACACIAWOOD, IDS_DESC_STAIRS)
            ->setBaseItemTypeAndMaterial(16, 42));

    registerBlock(
        164, L"dark_oak_stairs",
        (new StairBlock(planks->defaultBlockState()->setValue(PlanksBlock::VARIANT, PlanksBlock::Variant::DARK_OAK)))
            ->setIconName(L"stairsWoodDarkOak")
            ->sendBlockData(15)
            ->setNameAndDescriptionId(IDS_TILE_STAIRS_DARKOAKWOOD, IDS_DESC_STAIRS)
            ->setBaseItemTypeAndMaterial(16, 43));

    registerBlock(165, L"slime",
                  (new SlimeBlock())
                      ->setSoundType(SoundType::SLIME)
                      ->setNameAndDescriptionId(IDS_TILE_BLOCK_SLIME, IDS_DESC_BLOCK_SLIME)
                      ->setBaseItemTypeAndMaterial(32, 57)
                      ->setSemiTransparent());

    registerBlock(
        166, L"barrier",
        (new BarrierBlock())->setIconName(L"barrier")->setNameAndDescriptionId(IDS_TILE_BARRIER, IDS_DESC_BARRIER));

    registerBlock(167, L"iron_trapdoor",
                  (new TrapDoorBlock(Material::METAL))
                      ->setIconName(L"iron_trapdoor")
                      ->setDestroyTime(5.0f)
                      ->setSoundType(SoundType::METAL)
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_TRAPDOOR_IRON, IDS_DESC_TRAPDOOR_IRON)
                      ->setBaseItemTypeAndMaterial(6, 25));

    Block* prismarine = (new PrismarineBlock())
                            ->setIconName(L"prismarine_rough")
                            ->setDestroyTime(1.5f)
                            ->setExplodeable(10.0f)
                            ->setSoundType(SoundType::STONE)
                            ->setNameAndDescriptionId(IDS_TILE_PRISMARINE, IDS_DESC_PRISMARINE)
                            ->setBaseItemTypeAndMaterial(13, 13);
    registerBlock(168, L"prismarine", prismarine);

    registerBlock(169, L"sea_lantern",
                  (new SeaLanternBlock(Material::GLASS))
                      ->setIconName(L"sea_lantern")
                      ->setDestroyTime(0.3f)
                      ->setSoundType(SoundType::GLASS)
                      ->setLightEmission(1.0f)
                      ->setNameAndDescriptionId(IDS_TILE_SEA_LANTERN, IDS_DESC_SEA_LANTERN)
                      ->setBaseItemTypeAndMaterial(23, 54));

    registerBlock(170, L"hay_block",
                  (new HayBlock())
                      ->setDestroyTime(0.5f)
                      ->setIconName(L"hay_block")
                      ->setSoundType(SoundType::GRASS)
                      ->setNameAndDescriptionId(IDS_TILE_HAY, IDS_DESC_HAY)
                      ->setBaseItemTypeAndMaterial(32, 53));

    registerBlock(171, L"carpet",
                  (new WoolCarpetBlock())
                      ->setIconName(L"woolCarpet")
                      ->setDestroyTime(0.1f)
                      ->setSoundType(SoundType::CLOTH)
                      ->setLightBlock(0)
                      ->setNameAndDescriptionId(IDS_TILE_CARPET, IDS_DESC_CARPET)
                      ->setBaseItemTypeAndMaterial(40, 6));

    registerBlock(172, L"hardened_clay",
                  (new HardenedClayBlock())
                      ->setIconName(L"hardened_clay")
                      ->setDestroyTime(1.25f)
                      ->setExplodeable(7.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_HARDENED_CLAY, IDS_DESC_HARDENED_CLAY)
                      ->setBaseItemTypeAndMaterial(41, 15));

    registerBlock(173, L"coal_block",
                  (new BlockImpl(Material::STONE, MaterialColor::COLOR_BLACK))
                      ->setIconName(L"coal_block")
                      ->setDestroyTime(5.0f)
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_COAL, IDS_DESC_COAL_BLOCK)
                      ->setBaseItemTypeAndMaterial(32, 49));

    registerBlock(174, L"packed_ice",
                  (new PackedIceBlock())
                      ->setIconName(L"ice_packed")
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::GLASS)
                      ->setNameAndDescriptionId(IDS_TILE_ICE_PACKED, IDS_DESC_ICE_PACKED));

    registerBlock(175, L"double_plant",
                  (new DoublePlantBlock())
                      ->setNameAndDescriptionId(IDS_TILE_DOUBLE_PLANT, IDS_DESC_DOUBLE_PLANT)
                      ->disableMipmap());

    registerBlock(176, L"standing_banner",
                  (new StandingBannerBlock())
                      ->setIconName(L"planks_oak")
                      ->setDestroyTime(1.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNotCollectStatistics()
                      ->setNameAndDescriptionId(IDS_TILE_STANDING_BANNER, IDS_DESC_STANDING_BANNER));

    registerBlock(177, L"wall_banner",
                  (new WallBannerBlock())
                      ->setIconName(L"planks_oak")
                      ->setDestroyTime(1.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNotCollectStatistics()
                      ->setNameAndDescriptionId(IDS_TILE_WALL_BANNER, IDS_DESC_WALL_BANNER));

    registerBlock(178, L"daylight_detector_inverted",
                  (new DaylightDetectorBlock(1))
                      ->setIconName(L"daylight_detector")
                      ->setNameAndDescriptionId(IDS_TILE_DAYLIGHT_DETECTOR, IDS_DESC_DAYLIGHT_DETECTOR));

    Block* redSandstone = (new RedSandStoneBlock())
                              ->setIconName(L"red_sandstone")
                              ->setSoundType(SoundType::STONE)
                              ->setDestroyTime(0.8f)
                              ->setNameAndDescriptionId(IDS_TILE_REDSANDSTONE, IDS_DESC_REDSANDSTONE)
                              ->setBaseItemTypeAndMaterial(13, 48);
    registerBlock(179, L"red_sandstone", redSandstone);

    registerBlock(180, L"red_sandstone_stairs",
                  (new StairBlock(redSandstone->defaultBlockState()->setValue(RedSandStoneBlock::TYPE,
                                                                              RedSandStoneBlock::Type::SMOOTH)))
                      ->setIconName(L"red_sandstone")
                      ->setNameAndDescriptionId(IDS_TILE_STAIRS_REDSANDSTONE, IDS_DESC_STAIRS_REDSANDSTONE)
                      ->setBaseItemTypeAndMaterial(16, 48));

    registerBlock(181, L"double_stone_slab2",
                  (new FullNewStoneSlabBlock())
                      ->setIconName(L"double_stone_slab2")
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_STONESLAB_REDSANDSTONE, IDS_DESC_STONESLAB_REDSANDSTONE)
                      ->setBaseItemTypeAndMaterial(21, 48));

    registerBlock(182, L"stone_slab2",
                  (new HalfNewStoneSlabBlock())
                      ->setIconName(L"stone_slab2")
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_STONESLAB_REDSANDSTONE, IDS_DESC_STONESLAB_REDSANDSTONE)
                      ->setBaseItemTypeAndMaterial(22, 48));

    registerBlock(183, L"spruce_fence_gate",
                  (new FenceGateBlock(L"planks_spruce", PlanksBlock::Variant::SPRUCE))
                      ->setIconName(L"fenceGate")
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_FENCE_GATE_SPRUCEWOOD, IDS_DESC_FENCE_GATE)
                      ->setBaseItemTypeAndMaterial(47, 39));

    registerBlock(184, L"birch_fence_gate",
                  (new FenceGateBlock(L"planks_birch", PlanksBlock::Variant::BIRCH))
                      ->setIconName(L"fenceGate")
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_FENCE_GATE_BIRCHWOOD, IDS_DESC_FENCE_GATE)
                      ->setBaseItemTypeAndMaterial(47, 40));

    registerBlock(185, L"jungle_fence_gate",
                  (new FenceGateBlock(L"planks_jungle", PlanksBlock::Variant::JUNGLE))
                      ->setIconName(L"fenceGate")
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_FENCE_GATE_JUNGLEWOOD, IDS_DESC_FENCE_GATE)
                      ->setBaseItemTypeAndMaterial(47, 41));

    registerBlock(186, L"dark_oak_fence_gate",
                  (new FenceGateBlock(L"planks_big_oak", PlanksBlock::Variant::DARK_OAK))
                      ->setIconName(L"fenceGate")
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_FENCE_GATE_DARKOAKWOOD, IDS_DESC_FENCE_GATE)
                      ->setBaseItemTypeAndMaterial(47, 42));

    registerBlock(187, L"acacia_fence_gate",
                  (new FenceGateBlock(L"planks_acacia", PlanksBlock::Variant::ACACIA))
                      ->setIconName(L"fenceGate")
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_FENCE_GATE_ACACIAWOOD, IDS_DESC_FENCE_GATE)
                      ->setBaseItemTypeAndMaterial(47, 43));

    registerBlock(188, L"spruce_fence",
                  (new FenceBlock(L"planks_spruce", Material::WOOD, PlanksBlock::Variant::SPRUCE->getMaterialColor()))
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_FENCE_SPRUCEWOOD, IDS_DESC_FENCE)
                      ->setBaseItemTypeAndMaterial(29, 39));

    registerBlock(189, L"birch_fence",
                  (new FenceBlock(L"planks_birch", Material::WOOD, PlanksBlock::Variant::BIRCH->getMaterialColor()))
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_FENCE_BIRCHWOOD, IDS_DESC_FENCE)
                      ->setBaseItemTypeAndMaterial(29, 40));

    registerBlock(190, L"jungle_fence",
                  (new FenceBlock(L"planks_jungle", Material::WOOD, PlanksBlock::Variant::JUNGLE->getMaterialColor()))
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_FENCE_JUNGLEWOOD, IDS_DESC_FENCE)
                      ->setBaseItemTypeAndMaterial(29, 41));

    registerBlock(
        191, L"dark_oak_fence",
        (new FenceBlock(L"planks_big_oak", Material::WOOD, PlanksBlock::Variant::DARK_OAK->getMaterialColor()))
            ->setDestroyTime(2.0f)
            ->setExplodeable(5.0f)
            ->setSoundType(SoundType::WOOD)
            ->setNameAndDescriptionId(IDS_TILE_FENCE_DARKOAKWOOD, IDS_DESC_FENCE)
            ->setBaseItemTypeAndMaterial(29, 42));

    registerBlock(192, L"acacia_fence",
                  (new FenceBlock(L"planks_acacia", Material::WOOD, PlanksBlock::Variant::ACACIA->getMaterialColor()))
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(5.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_FENCE_ACACIAWOOD, IDS_DESC_FENCE)
                      ->setBaseItemTypeAndMaterial(29, 43));

    registerBlock(193, L"spruce_door",
                  (new DoorBlock(Material::WOOD))
                      ->setIconName(L"door_spruce")
                      ->setDestroyTime(3.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_DOOR_SPRUCEWOOD, IDS_DESC_DOOR_WOOD));

    registerBlock(194, L"birch_door",
                  (new DoorBlock(Material::WOOD))
                      ->setIconName(L"door_birch")
                      ->setDestroyTime(3.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_DOOR_BIRCHWOOD, IDS_DESC_DOOR_WOOD));

    registerBlock(195, L"jungle_door",
                  (new DoorBlock(Material::WOOD))
                      ->setIconName(L"door_jungle")
                      ->setDestroyTime(3.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_DOOR_JUNGLEWOOD, IDS_DESC_DOOR_WOOD));

    registerBlock(196, L"acacia_door",
                  (new DoorBlock(Material::WOOD))
                      ->setIconName(L"door_acacia")
                      ->setDestroyTime(3.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_DOOR_ACACIAWOOD, IDS_DESC_DOOR_WOOD));

    registerBlock(197, L"dark_oak_door",
                  (new DoorBlock(Material::WOOD))
                      ->setIconName(L"door_dark_oak")
                      ->setDestroyTime(3.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_DOOR_DARKOAKWOOD, IDS_DESC_DOOR_WOOD));

    registerBlock(198, L"end_rod",
                  (new EndRodBlock())
                      ->setIconName(L"end_rod")
                      ->setDestroyTime(0.0f)
                      ->setLightEmission(0.9375f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_END_ROD, IDS_DESC_END_ROD));

    registerBlock(199, L"chorus_plant",
                  (new ChorusPlantBlock())
                      ->setIconName(L"chorus_flower_plant")
                      ->setDestroyTime(0.4f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_CHORUS_PLANT, IDS_DESC_CHORUS_PLANT));

    registerBlock(200, L"chorus_flower",
                  (new ChorusFlowerBlock())
                      ->setIconName(L"chorus_flower")
                      ->setDestroyTime(0.4f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_CHORUS_FLOWER, IDS_DESC_CHORUS_FLOWER));

    Block* purpurBlock = (new BlockImpl(Material::STONE, MaterialColor::COLOR_MAGENTA))
                             ->setIconName(L"purpur_block")
                             ->setDestroyTime(1.5f)
                             ->setExplodeable(10.0f)
                             ->setSoundType(SoundType::STONE)
                             ->setNameAndDescriptionId(IDS_TILE_PURPUR, IDS_DESC_PURPUR)
                             ->setBaseItemTypeAndMaterial(32, 62);
    registerBlock(201, L"purpur_block", purpurBlock);

    registerBlock(202, L"purpur_pillar",
                  (new RotatedPillarBlockImpl(Material::STONE, MaterialColor::COLOR_MAGENTA))
                      ->setIconTopName(L"purpur_pillar_top")
                      ->setIconName(L"purpur_pillar")
                      ->setDestroyTime(1.5f)
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_PURPUR_PILLAR, IDS_DESC_PURPUR_PILLAR)
                      ->setBaseItemTypeAndMaterial(14, 62));

    registerBlock(203, L"purpur_stairs",
                  (new StairBlock(purpurBlock->defaultBlockState()))
                      ->setNameAndDescriptionId(IDS_TILE_PURPUR_STAIRS, IDS_DESC_PURPUR_STAIRS)
                      ->setBaseItemTypeAndMaterial(16, 62));

    registerBlock(204, L"purpur_double_slab",
                  (new PurpurSlabBlockFull())
                      ->setDestroyTime(2.0f)
                      ->setIconName(L"purpur_block")
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_PURPUR_SLAB, IDS_DESC_PURPUR_SLAB)
                      ->setBaseItemTypeAndMaterial(21, 62));

    registerBlock(205, L"purpur_slab",
                  (new PurpurSlabBlockHalf())
                      ->setDestroyTime(2.0f)
                      ->setIconName(L"purpur_block")
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_PURPUR_SLAB, IDS_DESC_PURPUR_SLAB)
                      ->setBaseItemTypeAndMaterial(22, 62));

    registerBlock(206, L"end_bricks",
                  (new BlockImpl(Material::STONE, MaterialColor::SAND))
                      ->setSoundType(SoundType::STONE)
                      ->setDestroyTime(0.8f)
                      ->setIconName(L"end_bricks")
                      ->setNameAndDescriptionId(IDS_TILE_END_BRICKS, IDS_DESC_TILE_END_BRICKS)
                      ->setBaseItemTypeAndMaterial(14, 33));

    registerBlock(207, L"beetroots",
                  (new BeetrootBlock())
                      ->setIconName(L"beetroots")
                      ->disableMipmap()
                      ->setNameAndDescriptionId(IDS_TILE_BEETROOTS, IDS_DESC_TILE_BEETROOTS));

    Block* grassPath = (new GrassPathBlock())
                           ->setDestroyTime(0.65f)
                           ->setSoundType(SoundType::GRASS)
                           ->setIconName(L"grass_path")
                           ->setNotCollectStatistics()
                           ->setNameAndDescriptionId(IDS_TILE_GRASS_PATH, IDS_DESC_TILE_GRASS_PATH);
    registerBlock(208, L"grass_path", grassPath);

    registerBlock(209, L"end_gateway",
                  (new EndGatewayBlock(Material::PORTAL))
                      ->setIconName(L"stone")
                      ->setDestroyTime(INDESTRUCTIBLE)
                      ->setExplodeable(6000000.0f)
                      ->setNameAndDescriptionId(IDS_END_GATEWAY, IDS_DESC_END_GATEWAY));

    registerBlock(210, L"repeating_command_block",
                  (new CommandBlock(MaterialColor::COLOR_PURPLE, CommandBlock::REPEAT))
                      ->setIconName(L"repeating_command_block")
                      ->setIndestructible()
                      ->setExplodeable(6000000.0f)
                      ->setNameAndDescriptionId(IDS_TILE_COMMAND_BLOCK, IDS_DESC_COMMAND_BLOCK));

    registerBlock(211, L"chain_command_block",
                  (new CommandBlock(MaterialColor::COLOR_GREEN, CommandBlock::CHAIN))
                      ->setIconName(L"chain_command_block")
                      ->setIndestructible()
                      ->setExplodeable(6000000.0f)
                      ->setNameAndDescriptionId(IDS_TILE_COMMAND_BLOCK, IDS_DESC_COMMAND_BLOCK));

    registerBlock(212, L"frosted_ice",
                  (new FrostedIceBlock())
                      ->setDestroyTime(0.5f)
                      ->setLightBlock(3)
                      ->setSoundType(SoundType::GLASS)
                      ->setNameAndDescriptionId(IDS_TILE_FROSTED_ICE, IDS_DESC_TILE_FROSTED_ICE));

    registerBlock(213, L"magma",
                  (new MagmaBlock())
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::STONE)
                      ->setIconName(L"magma")
                      ->setNameAndDescriptionId(IDS_TILE_MAGMA, IDS_DESC_TILE_MAGMA)
                      ->setBaseItemTypeAndMaterial(32, 61));

    registerBlock(214, L"nether_wart_block",
                  (new BlockImpl(Material::GRASS, MaterialColor::COLOR_RED))
                      ->setDestroyTime(1.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setIconName(L"nether_wart_block")
                      ->setNameAndDescriptionId(IDS_TILE_NETHER_WART_BLOCK, IDS_DESC_TILE_NETHER_WART_BLOCK)
                      ->setBaseItemTypeAndMaterial(32, 60));

    registerBlock(215, L"red_nether_brick",
                  (new NetherBrickBlock())
                      ->setDestroyTime(2.0f)
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setIconName(L"red_nether_brick")
                      ->setNameAndDescriptionId(IDS_TILE_RED_NETHER_BRICK, IDS_DESC_TILE_RED_NETHER_BRICK)
                      ->setBaseItemTypeAndMaterial(14, 32));

    registerBlock(216, L"bone_block",
                  (new BoneBlock())
                      ->setIconName(L"bone_block")
                      ->setNameAndDescriptionId(IDS_TILE_BONE_BLOCK, IDS_DESC_TILE_BONE_BLOCK)
                      ->setBaseItemTypeAndMaterial(13, 59));

    registerBlock(217, L"structure_void",
                  (new StructureVoidBlock())->setIconName(L"structure_block")->setNameAndDescriptionId(-1, -1));

    registerBlock(218, L"observer",
                  (new ObserverBlock())
                      ->setDestroyTime(3.0f)
                      ->setNameAndDescriptionId(IDS_TILE_OBSERVER, IDS_DESC_TILE_OBSERVER));

    registerBlock(219, L"white_shulker_box",
                  (new ShulkerBoxBlock(DyeColor::WHITE))
                      ->setIconName(L"shulker_top")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_SHULKER_BOX_WHITE, IDS_TILE_SHULKER_BOX_DESC)
                      ->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(220, L"orange_shulker_box",
                  (new ShulkerBoxBlock(DyeColor::ORANGE))
                      ->setIconName(L"shulker_top")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_SHULKER_BOX_ORANGE, IDS_TILE_SHULKER_BOX_DESC)
                      ->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(221, L"magenta_shulker_box",
                  (new ShulkerBoxBlock(DyeColor::MAGENTA))
                      ->setIconName(L"shulker_top")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_SHULKER_BOX_MAGENTA, IDS_TILE_SHULKER_BOX_DESC)
                      ->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(222, L"light_blue_shulker_box",
                  (new ShulkerBoxBlock(DyeColor::LIGHT_BLUE))
                      ->setIconName(L"shulker_top")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_SHULKER_BOX_LIGHT_BLUE, IDS_TILE_SHULKER_BOX_DESC)
                      ->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(223, L"yellow_shulker_box",
                  (new ShulkerBoxBlock(DyeColor::YELLOW))
                      ->setIconName(L"shulker_top")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_SHULKER_BOX_YELLOW, IDS_TILE_SHULKER_BOX_DESC)
                      ->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(224, L"lime_shulker_box",
                  (new ShulkerBoxBlock(DyeColor::LIME))
                      ->setIconName(L"shulker_top")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_SHULKER_BOX_LIME, IDS_TILE_SHULKER_BOX_DESC)
                      ->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(225, L"pink_shulker_box",
                  (new ShulkerBoxBlock(DyeColor::PINK))
                      ->setIconName(L"shulker_top")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_SHULKER_BOX_PINK, IDS_TILE_SHULKER_BOX_DESC)
                      ->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(226, L"gray_shulker_box",
                  (new ShulkerBoxBlock(DyeColor::GRAY))
                      ->setIconName(L"shulker_top")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_SHULKER_BOX_GRAY, IDS_TILE_SHULKER_BOX_DESC)
                      ->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(227, L"silver_shulker_box",
                  (new ShulkerBoxBlock(DyeColor::SILVER))
                      ->setIconName(L"shulker_top")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_SHULKER_BOX_SILVER, IDS_TILE_SHULKER_BOX_DESC)
                      ->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(228, L"cyan_shulker_box",
                  (new ShulkerBoxBlock(DyeColor::CYAN))
                      ->setIconName(L"shulker_top")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_SHULKER_BOX_CYAN, IDS_TILE_SHULKER_BOX_DESC)
                      ->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(229, L"purple_shulker_box",
                  (new ShulkerBoxBlock(DyeColor::PURPLE))
                      ->setIconName(L"shulker_top")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_SHULKER_BOX_PURPLE, IDS_TILE_SHULKER_BOX_DESC)
                      ->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(230, L"blue_shulker_box",
                  (new ShulkerBoxBlock(DyeColor::BLUE))
                      ->setIconName(L"shulker_top")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_SHULKER_BOX_BLUE, IDS_TILE_SHULKER_BOX_DESC)
                      ->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(231, L"brown_shulker_box",
                  (new ShulkerBoxBlock(DyeColor::BROWN))
                      ->setIconName(L"shulker_top")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_SHULKER_BOX_BROWN, IDS_TILE_SHULKER_BOX_DESC)
                      ->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(232, L"green_shulker_box",
                  (new ShulkerBoxBlock(DyeColor::GREEN))
                      ->setIconName(L"shulker_top")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_SHULKER_BOX_GREEN, IDS_TILE_SHULKER_BOX_DESC)
                      ->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(233, L"red_shulker_box",
                  (new ShulkerBoxBlock(DyeColor::RED))
                      ->setIconName(L"shulker_top")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_SHULKER_BOX_RED, IDS_TILE_SHULKER_BOX_DESC)
                      ->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(234, L"black_shulker_box",
                  (new ShulkerBoxBlock(DyeColor::BLACK))
                      ->setIconName(L"shulker_top")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_SHULKER_BOX_BLACK, IDS_TILE_SHULKER_BOX_DESC)
                      ->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(
        235, L"white_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::WHITE))
            ->setIconName(L"glazed_terracotta_white")
            ->setNameAndDescriptionId(IDS_TILE_GLAZED_TERRACOTTA_WHITE, IDS_TILE_GLAZED_TERRACOTTA_WHITE_DESC));

    registerBlock(
        236, L"orange_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::ORANGE))
            ->setIconName(L"glazed_terracotta_orange")
            ->setNameAndDescriptionId(IDS_TILE_GLAZED_TERRACOTTA_ORANGE, IDS_TILE_GLAZED_TERRACOTTA_ORANGE_DESC));

    registerBlock(
        237, L"magenta_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::MAGENTA))
            ->setIconName(L"glazed_terracotta_magenta")
            ->setNameAndDescriptionId(IDS_TILE_GLAZED_TERRACOTTA_MAGENTA, IDS_TILE_GLAZED_TERRACOTTA_MAGENTA_DESC));

    registerBlock(238, L"light_blue_glazed_terracotta",
                  (new GlazedTerracottaBlock(DyeColor::LIGHT_BLUE))
                      ->setIconName(L"glazed_terracotta_light_blue")
                      ->setNameAndDescriptionId(IDS_TILE_GLAZED_TERRACOTTA_LIGHT_BLUE,
                                                IDS_TILE_GLAZED_TERRACOTTA_LIGHT_BLUE_DESC));

    registerBlock(
        239, L"yellow_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::YELLOW))
            ->setIconName(L"glazed_terracotta_yellow")
            ->setNameAndDescriptionId(IDS_TILE_GLAZED_TERRACOTTA_YELLOW, IDS_TILE_GLAZED_TERRACOTTA_YELLOW_DESC));

    registerBlock(240, L"lime_glazed_terracotta",
                  (new GlazedTerracottaBlock(DyeColor::LIME))
                      ->setIconName(L"glazed_terracotta_lime")
                      ->setNameAndDescriptionId(IDS_TILE_GLAZED_TERRACOTTA_LIME, IDS_TILE_GLAZED_TERRACOTTA_LIME_DESC));

    registerBlock(241, L"pink_glazed_terracotta",
                  (new GlazedTerracottaBlock(DyeColor::PINK))
                      ->setIconName(L"glazed_terracotta_pink")
                      ->setNameAndDescriptionId(IDS_TILE_GLAZED_TERRACOTTA_PINK, IDS_TILE_GLAZED_TERRACOTTA_PINK_DESC));

    registerBlock(242, L"gray_glazed_terracotta",
                  (new GlazedTerracottaBlock(DyeColor::GRAY))
                      ->setIconName(L"glazed_terracotta_gray")
                      ->setNameAndDescriptionId(IDS_TILE_GLAZED_TERRACOTTA_GRAY, IDS_TILE_GLAZED_TERRACOTTA_GRAY_DESC));

    registerBlock(
        243, L"silver_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::SILVER))
            ->setIconName(L"glazed_terracotta_silver")
            ->setNameAndDescriptionId(IDS_TILE_GLAZED_TERRACOTTA_SILVER, IDS_TILE_GLAZED_TERRACOTTA_SILVER_DESC));

    registerBlock(244, L"cyan_glazed_terracotta",
                  (new GlazedTerracottaBlock(DyeColor::CYAN))
                      ->setIconName(L"glazed_terracotta_cyan")
                      ->setNameAndDescriptionId(IDS_TILE_GLAZED_TERRACOTTA_CYAN, IDS_TILE_GLAZED_TERRACOTTA_CYAN_DESC));

    registerBlock(
        245, L"purple_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::PURPLE))
            ->setIconName(L"glazed_terracotta_purple")
            ->setNameAndDescriptionId(IDS_TILE_GLAZED_TERRACOTTA_PURPLE, IDS_TILE_GLAZED_TERRACOTTA_PURPLE_DESC));

    registerBlock(246, L"blue_glazed_terracotta",
                  (new GlazedTerracottaBlock(DyeColor::BLUE))
                      ->setIconName(L"glazed_terracotta_blue")
                      ->setNameAndDescriptionId(IDS_TILE_GLAZED_TERRACOTTA_BLUE, IDS_TILE_GLAZED_TERRACOTTA_BLUE_DESC));

    registerBlock(
        247, L"brown_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::BROWN))
            ->setIconName(L"glazed_terracotta_brown")
            ->setNameAndDescriptionId(IDS_TILE_GLAZED_TERRACOTTA_BROWN, IDS_TILE_GLAZED_TERRACOTTA_BROWN_DESC));

    registerBlock(
        248, L"green_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::GREEN))
            ->setIconName(L"glazed_terracotta_green")
            ->setNameAndDescriptionId(IDS_TILE_GLAZED_TERRACOTTA_GREEN, IDS_TILE_GLAZED_TERRACOTTA_GREEN_DESC));

    registerBlock(249, L"red_glazed_terracotta",
                  (new GlazedTerracottaBlock(DyeColor::RED))
                      ->setIconName(L"glazed_terracotta_red")
                      ->setNameAndDescriptionId(IDS_TILE_GLAZED_TERRACOTTA_RED, IDS_TILE_GLAZED_TERRACOTTA_RED_DESC));

    registerBlock(
        250, L"black_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::BLACK))
            ->setIconName(L"glazed_terracotta_black")
            ->setNameAndDescriptionId(IDS_TILE_GLAZED_TERRACOTTA_BLACK, IDS_TILE_GLAZED_TERRACOTTA_BLACK_DESC));

    registerBlock(251, L"concrete",
                  (new ColoredBlock(Material::STONE))
                      ->setDestroyTime(1.8f)
                      ->setSoundType(SoundType::STONE)
                      ->setIconName(L"concrete")
                      ->setNameAndDescriptionId(IDS_TILE_CONCRETE_BLACK, IDS_TILE_CONCRETE_DESC));

    registerBlock(252, L"concrete_powder",
                  (new ConcretePowderBlock())
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::SAND)
                      ->setIconName(L"concrete_powder")
                      ->setNameAndDescriptionId(IDS_TILE_CONCRETE_POWDER_BLACK, IDS_TILE_CONCRETE_POWDER_DESC)
                      ->setBaseItemTypeAndMaterial(57, 69));

    registerBlock(255, L"structure_block",
                  (new StructureBlock())
                      ->setIconName(L"structure_block")
                      ->setIndestructible()
                      ->setExplodeable(6000000.0f)
                      ->setNameAndDescriptionId(-1, -1));

    registerBlock(256, L"conduit",
                  (new ConduitBlock())
                      ->setDestroyTime(3.0f)
                      ->setLightEmission(1.0f)
                      ->setNameAndDescriptionId(0xDA1C2954, 0x2336B52A)
                      ->setBaseItemTypeAndMaterial(61, 2));

    registerBlock(258, L"kelp",
                  (new KelpBlock())
                      ->setIconName(L"kelp_top")
                      ->setSoundType(SoundType::GRASS)
                      ->setDestroyTime(0.0f)
                      ->setNameAndDescriptionId(IDS_TILE_KELP, IDS_DESC_TILE_KELP));

    registerBlock(259, L"coral_block",
                  (new CoralBlock())
                      ->setIconName(L"coral_red")
                      ->setDestroyTime(7.0f)
                      ->setNameAndDescriptionId(IDS_TILE_CORAL_BLOCK_BLUE, IDS_DESC_TILE_CORAL_BLOCK));

    registerBlock(263, L"coral",
                  (new Coral())
                      ->setIconName(L"coral")
                      ->setDestroyTime(0.0f)
                      ->setNameAndDescriptionId(IDS_TILE_CORAL_BLUE, IDS_DESC_TILE_CORAL)
                      ->disableMipmap());

    registerBlock(264, L"coral_fan",
                  (new CoralFan())
                      ->setIconName(L"coral_fan")
                      ->setDestroyTime(0.0f)
                      ->setExplodeable(1.5f)
                      ->setNameAndDescriptionId(IDS_TILE_CORAL_FAN_BLUE_FAN, IDS_DESC_TILE_CORAL_FAN)
                      ->disableMipmap());

    registerBlock(265, L"coral_fan_dead",
                  (new CoralFan())
                      ->setIconName(L"coral_fan_dead")
                      ->setDestroyTime(0.0f)
                      ->setExplodeable(1.5f)
                      ->setNameAndDescriptionId(IDS_TILE_CORAL_FAN_DEAD_BLUE_FAN, IDS_DESC_TILE_CORAL_FAN_DEAD)
                      ->disableMipmap());

    registerBlock(266, L"coral_fan_hang",
                  (new CoralFanHang())
                      ->setIconName(L"coral_fan_hang")
                      ->setDestroyTime(0.0f)
                      ->setExplodeable(1.5f)
                      ->setNameAndDescriptionId(IDS_TILE_CORAL_FAN_BLUE_FAN, IDS_DESC_TILE_CORAL_FAN)
                      ->disableMipmap());

    registerBlock(267, L"coral_fan_hang2",
                  (new CoralFanHang())
                      ->setIconName(L"coral_fan_hang2")
                      ->setDestroyTime(0.0f)
                      ->setExplodeable(1.5f)
                      ->setNameAndDescriptionId(IDS_TILE_CORAL_FAN_PINK_FAN, IDS_DESC_TILE_CORAL_FAN)
                      ->disableMipmap());

    registerBlock(268, L"coral_fan_hang3",
                  (new CoralFanHang())
                      ->setIconName(L"coral_fan_hang3")
                      ->setDestroyTime(0.0f)
                      ->setExplodeable(1.5f)
                      ->setNameAndDescriptionId(IDS_TILE_CORAL_FAN_PURPLE_FAN, IDS_DESC_TILE_CORAL_FAN)
                      ->disableMipmap());

    registerBlock(269, L"dried_kelp_block",
                  (new DriedKelpBlock(Material::BAMBOO))
                      ->setSoundType(SoundType::GRASS)
                      ->setIconName(L"dried_kelp_block")
                      ->setDestroyTime(0.6f)
                      ->setNameAndDescriptionId(IDS_TILE_DRIED_KELP, IDS_DESC_TILE_DRIED_KELP)
                      ->setBaseItemTypeAndMaterial(32, 73));

    registerBlock(270, L"sea_grass",
                  (new SeaGrass())
                      ->setIconName(L"seagrass")
                      ->setSoundType(SoundType::GRASS)
                      ->setDestroyTime(0.0f)
                      ->setNameAndDescriptionId(IDS_TILE_SEA_GRASS, IDS_DESC_TILE_SEA_GRASS));

    registerBlock(271, L"sea_pickle",
                  (new SeaPickle())
                      ->setIconName(L"sea_pickle")
                      ->setSoundType(SoundType::SLIME)
                      ->setDestroyTime(0.4f)
                      ->setNameAndDescriptionId(IDS_TILE_SEA_PICKLE, IDS_DESC_TILE_SEA_PICKLE));

    registerBlock(272, L"bubble_column_segment",
                  (new BubbleColumnSegment(Material::BUBBLE_COLUMN))
                      ->setIconName(L"bubble_column_segment")
                      ->setTicking(1)
                      ->setNameAndDescriptionId(IDS_TILE_BUBBLE_COLUMN_SEGMENT, IDS_DESC_TILE_BUBBLE_COLUMN_SEGMENT));

    registerBlock(273, L"blue_ice",
                  (new BlueIceBlock(Material::ICE_SOLID))
                      ->setIconName(L"blue_ice")
                      ->setDestroyTime(2.8f)
                      ->setSoundType(SoundType::GLASS)
                      ->setLightEmission(0.3f)
                      ->setNameAndDescriptionId(IDS_TILE_BLUE_ICE, IDS_DESC_TILE_BLUE_ICE)
                      ->setBaseItemTypeAndMaterial(32, 74));

    registerBlock(274, L"spruce_trapdoor",
                  (new TrapDoorBlock(Material::WOOD, MaterialColor::PODZOL))
                      ->setIconName(L"spruce_trapdoor")
                      ->setDestroyTime(3.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_TRAPDOOR_SPRUCE, IDS_DESC_TRAPDOOR)
                      ->setBaseItemTypeAndMaterial(6, 25));

    registerBlock(275, L"birch_trapdoor",
                  (new TrapDoorBlock(Material::WOOD, MaterialColor::SAND))
                      ->setIconName(L"birch_trapdoor")
                      ->setDestroyTime(3.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_TRAPDOOR_BIRCH, IDS_DESC_TRAPDOOR)
                      ->setBaseItemTypeAndMaterial(6, 25));

    registerBlock(276, L"jungle_trapdoor",
                  (new TrapDoorBlock(Material::WOOD, MaterialColor::DIRT))
                      ->setIconName(L"jungle_trapdoor")
                      ->setDestroyTime(3.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_TRAPDOOR_JUNGLE, IDS_DESC_TRAPDOOR)
                      ->setBaseItemTypeAndMaterial(6, 25));

    registerBlock(277, L"acacia_trapdoor",
                  (new TrapDoorBlock(Material::WOOD, MaterialColor::COLOR_ORANGE))
                      ->setIconName(L"acacia_trapdoor")
                      ->setDestroyTime(3.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_TRAPDOOR_ACACIA, IDS_DESC_TRAPDOOR)
                      ->setBaseItemTypeAndMaterial(6, 25));

    registerBlock(278, L"dark_oak_trapdoor",
                  (new TrapDoorBlock(Material::WOOD, MaterialColor::COLOR_BROWN))
                      ->setIconName(L"dark_oak_trapdoor")
                      ->setDestroyTime(3.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNotCollectStatistics()
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_TRAPDOOR_DARK_OAK, IDS_DESC_TRAPDOOR)
                      ->setBaseItemTypeAndMaterial(6, 25));

    registerBlock(279, L"turtle_egg",
                  (new TurtleEggBlock())
                      ->setSoundType(SoundType::STONE)
                      ->setIconName(L"turtle_egg_hatch_0")
                      ->setDestroyTime(0.4f)
                      ->setNameAndDescriptionId(IDS_TILE_TURTLE_EGG, IDS_DESC_TILE_TURTLE_EGG));

    registerBlock(291, L"prismarine_stairs",
                  (new StairBlock(prismarine->defaultBlockState()->setValue(PrismarineBlock::VARIANT,
                                                                            PrismarineBlock::Variant::ROUGH)))
                      ->setIconName(L"prismarine_stairs")
                      ->setNameAndDescriptionId(IDS_TILE_STAIRS_PRISMARINE, IDS_DESC_STAIRS_PRISMARINE)
                      ->setBaseItemTypeAndMaterial(16, 54));

    registerBlock(292, L"prismarine_bricks_stairs",
                  (new StairBlock(prismarine->defaultBlockState()->setValue(PrismarineBlock::VARIANT,
                                                                            PrismarineBlock::Variant::BRICKS)))
                      ->setIconName(L"prismarine_bricks")
                      ->setNameAndDescriptionId(IDS_TILE_STAIRS_PRISMARINE_BRICKS, IDS_DESC_STAIRS_PRISMARINE_BRICKS)
                      ->setBaseItemTypeAndMaterial(16, 54));

    registerBlock(293, L"dark_prismarine_stairs",
                  (new StairBlock(prismarine->defaultBlockState()->setValue(PrismarineBlock::VARIANT,
                                                                            PrismarineBlock::Variant::DARK)))
                      ->setIconName(L"dark_prismarine")
                      ->setNameAndDescriptionId(IDS_TILE_STAIRS_DARK_PRISMARINE, IDS_DESC_STAIRS_DARK_PRISMARINE)
                      ->setBaseItemTypeAndMaterial(16, 54));

    registerBlock(295, L"stripped_log_spruce",
                  (new StrippedLogBlock())
                      ->setIconTopName(L"stripped_log_spruce_top")
                      ->setIconName(L"stripped_log_spruce")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_STRIPPED_LOG_SPRUCE, IDS_DESC_STRIPPED_LOG_SPRUCE));

    registerBlock(296, L"stripped_log_birch",
                  (new StrippedLogBlock())
                      ->setIconTopName(L"stripped_log_birch_top")
                      ->setIconName(L"stripped_log_birch")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_STRIPPED_LOG_BIRCH, IDS_DESC_STRIPPED_LOG_BIRCH));

    registerBlock(297, L"stripped_log_jungle",
                  (new StrippedLogBlock())
                      ->setIconTopName(L"stripped_log_jungle_top")
                      ->setIconName(L"stripped_log_jungle")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_STRIPPED_LOG_JUNGLE, IDS_DESC_STRIPPED_LOG_JUNGLE));

    registerBlock(298, L"stripped_log_acacia",
                  (new StrippedLogBlock())
                      ->setIconTopName(L"stripped_log_acacia_top")
                      ->setIconName(L"stripped_log_acacia")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_STRIPPED_LOG_ACACIA, IDS_DESC_STRIPPED_LOG_ACACIA));

    registerBlock(299, L"stripped_log_dark_oak",
                  (new StrippedLogBlock())
                      ->setIconTopName(L"stripped_log_dark_oak_top")
                      ->setIconName(L"stripped_log_dark_oak")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_STRIPPED_LOG_DARK_OAK, IDS_DESC_STRIPPED_LOG_DARK_OAK));

    registerBlock(300, L"stripped_log_oak",
                  (new StrippedLogBlock())
                      ->setIconTopName(L"stripped_log_oak_top")
                      ->setIconName(L"stripped_log_oak")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->setNameAndDescriptionId(IDS_TILE_STRIPPED_LOG_OAK, IDS_DESC_STRIPPED_LOG_OAK));

    registerBlock(301, L"acacia_pressure_plate",
                  (new PressurePlateBlock(L"planks_acacia", Material::WOOD, PressurePlateBlock::EVERYTHING))
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_PRESSURE_PLATE_ACACIA, IDS_DESC_TILE_PRESSURE_PLATE_ACACIA)
                      ->setBaseItemTypeAndMaterial(15, 42));

    registerBlock(302, L"birch_pressure_plate",
                  (new PressurePlateBlock(L"planks_birch", Material::WOOD, PressurePlateBlock::EVERYTHING))
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_PRESSURE_PLATE_BIRCH, IDS_TILE_OAKPLANKSSENSITIVE_TITLE)
                      ->setBaseItemTypeAndMaterial(15, 40));

    registerBlock(303, L"dark_oak_pressure_plate",
                  (new PressurePlateBlock(L"planks_big_oak", Material::WOOD, PressurePlateBlock::EVERYTHING))
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_PRESSURE_PLATE_DARK_OAK, IDS_DESC_TILE_PRESSURE_PLATE_DARK_OAK)
                      ->setBaseItemTypeAndMaterial(15, 43));

    registerBlock(304, L"jungle_pressure_plate",
                  (new PressurePlateBlock(L"planks_jungle", Material::WOOD, PressurePlateBlock::EVERYTHING))
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_PRESSURE_PLATE_JUNGLE, IDS_DESC_TILE_PRESSURE_PLATE_JUNGLE)
                      ->setBaseItemTypeAndMaterial(15, 41));

    registerBlock(305, L"spruce_pressure_plate",
                  (new PressurePlateBlock(L"planks_spruce", Material::WOOD, PressurePlateBlock::EVERYTHING))
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_PRESSURE_PLATE_SPRUCE, IDS_DESC_TILE_PRESSURE_PLATE_SPRUCE)
                      ->setBaseItemTypeAndMaterial(15, 39));

    registerBlock(306, L"acacia_button",
                  (new WoodButtonBlock())
                      ->setIconName(L"planks_acacia")
                      ->setBaseItemTypeAndMaterial(35, 42)
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_BUTTON_ACACIA, IDS_DESC_BUTTON));

    registerBlock(307, L"birch_button",
                  (new WoodButtonBlock())
                      ->setIconName(L"planks_birch")
                      ->setBaseItemTypeAndMaterial(35, 40)
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_BUTTON_BIRCH, IDS_DESC_BUTTON));

    registerBlock(308, L"dark_oak_button",
                  (new WoodButtonBlock())
                      ->setIconName(L"planks_big_oak")
                      ->setBaseItemTypeAndMaterial(35, 43)
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_BUTTON_DARK_OAK, IDS_DESC_BUTTON));

    registerBlock(309, L"jungle_button",
                  (new WoodButtonBlock())
                      ->setIconName(L"planks_jungle")
                      ->setBaseItemTypeAndMaterial(35, 41)
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_BUTTON_JUNGLE, IDS_DESC_BUTTON));

    registerBlock(310, L"spruce_button",
                  (new WoodButtonBlock())
                      ->setIconName(L"planks_spruce")
                      ->setBaseItemTypeAndMaterial(35, 39)
                      ->setDestroyTime(0.5f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_BUTTON_SPRUCE, IDS_DESC_BUTTON));

    registerBlock(311, L"prismarine_double_slab",
                  (new PrismarineSlabBlockFull())
                      ->setDestroyTime(2.0f)
                      ->setIconName(L"prismarine_block")
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_SLAB_PRISMARINE, IDS_DESC_SLAB_PRISMARINE)
                      ->setBaseItemTypeAndMaterial(21, 54));

    registerBlock(312, L"prismarine_slab",
                  (new PrismarineSlabBlockHalf())
                      ->setDestroyTime(2.0f)
                      ->setIconName(L"prismarine_block")
                      ->setExplodeable(10.0f)
                      ->setSoundType(SoundType::STONE)
                      ->setNameAndDescriptionId(IDS_TILE_SLAB_PRISMARINE, IDS_DESC_SLAB_PRISMARINE)
                      ->setBaseItemTypeAndMaterial(22, 54));

    registerBlock(313, L"bark_spruce",
                  (new RotatedPillarBlockImpl(Material::WOOD))
                      ->setIconTopName(L"log_spruce")
                      ->setIconName(L"log_spruce")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_LOG, IDS_DESC_LOG));

    registerBlock(314, L"bark_birch",
                  (new RotatedPillarBlockImpl(Material::WOOD))
                      ->setIconTopName(L"log_birch")
                      ->setIconName(L"log_birch")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_LOG, IDS_DESC_LOG));

    registerBlock(315, L"bark_jungle",
                  (new RotatedPillarBlockImpl(Material::WOOD))
                      ->setIconTopName(L"log_jungle")
                      ->setIconName(L"log_jungle")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_LOG, IDS_DESC_LOG));

    registerBlock(316, L"bark_acacia",
                  (new RotatedPillarBlockImpl(Material::WOOD))
                      ->setIconTopName(L"log_acacia")
                      ->setIconName(L"log_acacia")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_LOG, IDS_DESC_LOG));

    registerBlock(317, L"bark_dark_oak",
                  (new RotatedPillarBlockImpl(Material::WOOD))
                      ->setIconTopName(L"log_big_oak")
                      ->setIconName(L"log_big_oak")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_LOG, IDS_DESC_LOG));

    registerBlock(318, L"bark_oak",
                  (new RotatedPillarBlockImpl(Material::WOOD))
                      ->setIconTopName(L"log_oak")
                      ->setIconName(L"log_oak")
                      ->setDestroyTime(2.0f)
                      ->setSoundType(SoundType::WOOD)
                      ->sendBlockData(15)
                      ->setNameAndDescriptionId(IDS_TILE_LOG, IDS_DESC_LOG));

#ifdef MINECRAFT_PC
    PcModContentRegisterBlocks();
#endif
    const boost::unordered_map<ResourceLocation, Block*>& registry = blockRegistry->keySet();

    for (boost::unordered_map<ResourceLocation, Block*>::const_iterator it = registry.begin(); it != registry.end();
         ++it)
    {
        Block* block = it->second;

        if (block->mMaterial == Material::AIR)
        {
            block->mUseNeighbourBrightness = false;
            continue;
        }

        bool isStair = dynamic_cast<StairBlock*>(block) != NULL;
        bool isSlab = dynamic_cast<HalfSlabBlock*>(block) != NULL;
        bool isFarmlandOrPath = block == farmland || block == grassPath;
        bool isTranslucent = block->mTranslucent;
        bool isLightTransparent = block->mLightBlock == 0;

        block->mUseNeighbourBrightness = isStair || isSlab || isFarmlandOrPath || isTranslucent || isLightTransparent;

        for (int data = 0; data < 16; data++)
        {
            block->getBlockState(data);
        }
    }

    for (boost::unordered_map<ResourceLocation, Block*>::const_iterator it = registry.begin(); it != registry.end();
         ++it)
    {
        Block* block = it->second;
        arrayWithLength<BlockStateDefinition::BlockStateImpl*> states =
            *block->getBlockStateDefinition()->getPossibleBlockStates();

        for (unsigned int i = 0; i < states.length; ++i)
        {
            BlockState* state = states.data[i];
            if (state != NULL)
                blockStateRegistry->addMapping(state, (blockRegistry->getId(block) << 4)
                                                          | block->convertBlockStateToLegacyData(state));
        }
    }

    Stats::buildItemStats();
}

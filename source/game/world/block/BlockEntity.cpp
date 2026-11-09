#include "game/world/block/BlockEntity.hpp"
#include "game/core/EntityType.hpp"

#include <map>
#include <boost/unordered_map.hpp>

#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/MappedRegistry.hpp"
#include "game/world/block/FurnaceBlockEntity.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/block/EnderChestBlockEntity.hpp"
#include "game/world/block/JukeboxBlock.hpp"
#include "game/world/block/DispenserBlockEntity.hpp"
#include "game/world/block/DropperBlockEntity.hpp"
#include "game/world/block/SignBlockEntity.hpp"
#include "game/world/block/MobSpawnerBlockEntity.hpp"
#include "game/client/audio/MusicBlockEntity.hpp"
#include "game/world/block/PistonMovingBlockEntity.hpp"
#include "game/world/block/BrewingStandBlockEntity.hpp"
#include "game/entity/mob/EnchantmentTableEntity.hpp"
#include "game/world/block/TheEndPortalBlockEntity.hpp"
#include "game/world/block/BeaconBlockEntity.hpp"
#include "game/world/block/SkullBlockEntity.hpp"
#include "game/world/block/DaylightDetectorBlockEntity.hpp"
#include "game/world/block/HopperBlockEntity.hpp"
#include "game/world/block/ComparatorBlockEntity.hpp"
#include "game/world/block/FlowerPotBlockEntity.hpp"
#include "game/world/block/BannerBlockEntity.hpp"
#include "game/entity/mob/TheEndGatewayEntity.hpp"
#include "game/world/block/CommandBlockEntity.hpp"
#include "game/world/block/ShulkerBoxBlockEntity.hpp"
#include "game/world/block/BedBlockEntity.hpp"
#include "game/world/block/CauldronBlockEntity.hpp"
#include "game/world/block/ConduitBlockEntity.hpp"

namespace
{
typedef boost::unordered_map<std::wstring, BlockEntity::Creator> CreatorMap;
typedef MappedRegistry<ResourceLocation, unsigned long long> TypeRegistry;
CreatorMap gCreators;
TypeRegistry gTypes;
}

BlockEntity::BlockEntity()
{
    mLevel = NULL;
    mPos = BlockPos::ZERO;
    mBlock = NULL;
    mRemoved = false;
    mData = -1;
    mRenderRemoveStage = 0;
}

void BlockEntity::rotate(Rotation* rotation)
{
}

void BlockEntity::mirror(Mirror* mirror)
{
}

void BlockEntity::setRenderRemoveStage(unsigned char stage)
{
    mRenderRemoveStage = stage;
}

void BlockEntity::upgradeRenderRemoveStage()
{
    if (mRenderRemoveStage == 1)
    {
        mRenderRemoveStage = 2;
    }
}

bool BlockEntity::shouldRemoveForRender()
{
    return mRenderRemoveStage == 2;
}

Level* BlockEntity::getLevel()
{
    return mLevel;
}

void BlockEntity::setLevel(Level* level)
{
    mLevel = level;
}

bool BlockEntity::hasLevel()
{
    return mLevel != NULL;
}

void BlockEntity::load(CompoundTag* tag)
{
    mPos = BlockPos(tag->getInt(L"x"), tag->getInt(L"y"), tag->getInt(L"z"));
}

CompoundTag* BlockEntity::saveMetadata(CompoundTag* tag)
{
    ResourceLocation key = gTypes.getKey(GetType());
    tag->putString(L"id", key.toString());
    tag->putInt(L"x", mPos.x);
    tag->putInt(L"y", mPos.y);
    tag->putInt(L"z", mPos.z);
    return tag;
}

CompoundTag* BlockEntity::save(CompoundTag* tag)
{
    return saveMetadata(tag);
}

void BlockEntity::tick()
{
}

void BlockEntity::setLevelForLoading(Level* level)
{
}

int BlockEntity::getData()
{
    if (mData == -1)
    {
        const BlockState* state = mLevel->getBlockState(mPos);
        mData = state->getBlock()->convertBlockStateToLegacyData(state);
    }
    return mData;
}

void BlockEntity::setData(int data)
{
    mData = data;
}

void BlockEntity::setChanged()
{
    if (mLevel != NULL)
    {
        const BlockState* state = mLevel->getBlockState(mPos);
        mData = state->getBlock()->convertBlockStateToLegacyData(state);
        mLevel->blockEntityChanged(mPos, shared_from_this());
        if (getBlock() != Blocks::air)
        {
            mLevel->updateNeighbourForOutputSignal(mPos, getBlock());
        }
    }
}

double BlockEntity::distanceToSqr(double x, double y, double z)
{
    const double dx = (double)mPos.x + 0.5 - x;
    const double dy = (double)mPos.y + 0.5 - y;
    const double dz = (double)mPos.z + 0.5 - z;
    return dx * dx + dy * dy + dz * dz;
}

double BlockEntity::getViewDistance()
{
    return 4096.0;
}

BlockPos BlockEntity::getBlockPos()
{
    return mPos;
}

Block* BlockEntity::getBlock()
{
    if (mBlock == NULL && mLevel != NULL)
    {
        mBlock = mLevel->getBlockState(mPos)->getBlock();
    }
    return mBlock;
}

boost::shared_ptr<Packet> BlockEntity::getUpdatePacket()
{
    return boost::shared_ptr<Packet>();
}

CompoundTag* BlockEntity::getUpdateTag()
{
    CompoundTag* tag = new CompoundTag();
    return saveMetadata(tag);
}

CompoundTag* BlockEntity::getUpdateTagForPistonMovement()
{
    return getUpdateTag();
}

bool BlockEntity::isRemoved()
{
    return mRemoved;
}

void BlockEntity::setRemoved()
{
    mRemoved = true;
}

void BlockEntity::clearRemoved()
{
    mRemoved = false;
}

bool BlockEntity::triggerEvent(int id, int param)
{
    return false;
}

void BlockEntity::clearCache()
{
    mBlock = NULL;
    mData = -1;
}

void BlockEntity::setPosition(const BlockPos& position)
{
    mPos = position;
}

bool BlockEntity::onlyOpCanSetNbt()
{
    return false;
}

std::wstring BlockEntity::getDisplayName()
{
    return L"";
}

void BlockEntity::clone(boost::shared_ptr<BlockEntity> other)
{
    other->mLevel = mLevel;
    other->mPos = mPos;
    other->mData = mData;
    other->mBlock = mBlock;
}

void BlockEntity::operator=(const BlockEntity& other)
{
    mRemoved = other.mRemoved;
    mRenderRemoveStage = other.mRenderRemoveStage;
    mLevel = other.mLevel;
    mPos = other.mPos;
    mData = other.mData;
    mBlock = other.mBlock;
}

void BlockEntity::staticCtor()
{
    Register(1, FurnaceBlockEntity::create, BlockEntityType::FURNACE, L"furnace");
    Register(2, ChestBlockEntity::create, BlockEntityType::CHEST, L"chest");
    Register(3, EnderChestBlockEntity::create, BlockEntityType::ENDER_CHEST, L"ender_Chest");
    Register(4, JukeboxBlock::Entity::create, BlockEntityType::JUKEBOX, L"jukebox");
    Register(5, DispenserBlockEntity::create, BlockEntityType::DISPENSER, L"dispenser");
    Register(6, DropperBlockEntity::create, BlockEntityType::DROPPER, L"dropper");
    Register(7, SignBlockEntity::create, BlockEntityType::SIGN, L"sign");
    Register(8, MobSpawnerBlockEntity::create, BlockEntityType::MOB_SPAWNER, L"mob_spawner");
    Register(9, MusicBlockEntity::create, BlockEntityType::MUSIC, L"noteblock");
    Register(10, PistonMovingBlockEntity::create, BlockEntityType::PISTON_MOVING, L"piston");
    Register(11, BrewingStandBlockEntity::create, BlockEntityType::BREWING_STAND, L"brewing_stand");
    Register(12, EnchantmentTableEntity::create, BlockEntityType::ENCHANTMENT_TABLE_ENTITY, L"enchanting_table");
    Register(13, TheEndPortalBlockEntity::create, BlockEntityType::THE_END_PORTAL, L"end_portal");
    Register(14, BeaconBlockEntity::create, BlockEntityType::BEACON, L"beacon");
    Register(15, SkullBlockEntity::create, BlockEntityType::SKULL, L"skull");
    Register(16, DaylightDetectorBlockEntity::create, BlockEntityType::DAYLIGHT_DETECTOR, L"daylight_detector");
    Register(17, HopperBlockEntity::create, BlockEntityType::HOPPER, L"hopper");
    Register(18, ComparatorBlockEntity::create, BlockEntityType::COMPARATOR, L"comparator");
    Register(19, FlowerPotBlockEntity::create, BlockEntityType::FLOWER_POT, L"flower_pot");
    Register(20, BannerBlockEntity::create, BlockEntityType::BANNER, L"banner");
    Register(22, TheEndGatewayEntity::create, BlockEntityType::THE_END_GATEWAY_ENTITY, L"end_gateway");
    Register(23, CommandBlockEntity::create, BlockEntityType::COMMAND, L"command_block");
    Register(24, ShulkerBoxBlockEntity::create, BlockEntityType::SHULKER_BOX, L"shulker_box");
    Register(25, BedBlockEntity::create, BlockEntityType::BED, L"bed");
    Register(26, CauldronBlockEntity::create, BlockEntityType::CAULDRON, L"cauldron");
    Register(27, ConduitBlockEntity::create, BlockEntityType::CONDUIT, L"conduit");
}

void BlockEntity::Register(int id, Creator creator, unsigned long long type, std::wstring name)
{
    ResourceLocation location(name);
    gCreators.insert(CreatorMap::value_type(location.toString(), creator));
    gTypes.registerMapping(id, location, type);
}

ResourceLocation BlockEntity::getKey(unsigned long long type)
{
    return gTypes.getKey(type);
}

boost::shared_ptr<BlockEntity> BlockEntity::loadStatic(Level* level, CompoundTag* tag)
{
    boost::shared_ptr<BlockEntity> entity;
    const std::wstring name = tag->getString(L"id");
    CreatorMap::iterator it = gCreators.find(name);
    if (it != gCreators.end())
    {
        entity = boost::shared_ptr<BlockEntity>(it->second());
        if (entity)
        {
            entity->setLevelForLoading(level);
            entity->load(tag);
        }
    }
    return entity;
}

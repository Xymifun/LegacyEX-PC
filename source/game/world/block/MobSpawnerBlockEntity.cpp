#include "game/world/block/MobSpawnerBlockEntity.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/UniqueTypeWalker.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Blocks.hpp"

MobSpawnerBlockEntity::MobSpawnerBlockEntity() : BlockEntity(), mSpawner(new BlockEntityMobSpawner(this))
{
}

MobSpawnerBlockEntity::~MobSpawnerBlockEntity()
{
    delete mSpawner;
}

unsigned long long MobSpawnerBlockEntity::GetType()
{
    return BlockEntityType::MOB_SPAWNER;
}

BlockEntity* MobSpawnerBlockEntity::create()
{
    return new MobSpawnerBlockEntity();
}

MobSpawnerBlockEntity::BlockEntityMobSpawner* MobSpawnerBlockEntity::getSpawner()
{
    return mSpawner;
}

void MobSpawnerBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_BlockEntity,
                     new UniqueTypeWalker(BlockEntity::getKey(BlockEntityType::MOB_SPAWNER)));
}

void MobSpawnerBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mSpawner->load(tag);
}

CompoundTag* MobSpawnerBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    mSpawner->save(tag);
    return tag;
}

void MobSpawnerBlockEntity::tick()
{
    mSpawner->tick();
    BlockEntity::tick();
}

boost::shared_ptr<Packet> MobSpawnerBlockEntity::getUpdatePacket()
{
    return boost::shared_ptr<Packet>(
        new ClientboundBlockEntityDataPacket(mPos, 1, mLevel->mDimension->getType()->getId(), getUpdateTag()));
}

CompoundTag* MobSpawnerBlockEntity::getUpdateTag()
{
    CompoundTag* tag = save(new CompoundTag());
    delete tag->get(L"SpawnPotentials");
    tag->remove(L"SpawnPotentials");
    return tag;
}

bool MobSpawnerBlockEntity::triggerEvent(int id, int param)
{
    if (mSpawner->onEventTriggered(id))
    {
        return true;
    }
    return BlockEntity::triggerEvent(id, param);
}

bool MobSpawnerBlockEntity::onlyOpCanSetNbt()
{
    return true;
}

boost::shared_ptr<BlockEntity> MobSpawnerBlockEntity::clone()
{
    boost::shared_ptr<MobSpawnerBlockEntity> result(new MobSpawnerBlockEntity());
    BlockEntity::clone(result);
    return result;
}

void MobSpawnerBlockEntity::setEntityId(const std::wstring& entityId)
{
    mSpawner->setEntityId(ResourceLocation(entityId));
}

MobSpawnerBlockEntity::BlockEntityMobSpawner::BlockEntityMobSpawner(MobSpawnerBlockEntity* blockEntity)
    : BaseMobSpawner(),
      mBlockEntity(blockEntity)
{
}

void MobSpawnerBlockEntity::BlockEntityMobSpawner::broadcastEvent(int event)
{
    mBlockEntity->mLevel->blockEvent(mBlockEntity->mPos, Blocks::mob_spawner, event, 0);
}

Level* MobSpawnerBlockEntity::BlockEntityMobSpawner::getLevel()
{
    return mBlockEntity->mLevel;
}

BlockPos MobSpawnerBlockEntity::BlockEntityMobSpawner::getPos()
{
    return mBlockEntity->mPos;
}

void MobSpawnerBlockEntity::BlockEntityMobSpawner::setNextSpawnData(SpawnData* data)
{
    BaseMobSpawner::setNextSpawnData(data);
    if (getLevel() != NULL)
    {
        const BlockState* state = mBlockEntity->getLevel()->getBlockState(getPos());
        getLevel()->sendBlockUpdated(mBlockEntity->mPos, state, state, 4, true);
    }
}

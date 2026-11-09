#include "game/entity/mob/TheEndGatewayEntity.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>
#include <vector>

#include "game/core/Mth.hpp"
#include "game/core/ThrownEnderpearl.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/util/NbtUtils.hpp"
#include "game/util/Random.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/TheEndDimension.hpp"
#include "game/world/block/EndGatewayBlock.hpp"
#include "game/world/block/Iterator.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/EndGatewayFeature.hpp"
#include "game/world/generation/EndIslandFeature.hpp"

TheEndGatewayEntity::TheEndGatewayEntity() : mAge(0), mCooldown(0), mExitPortal(NULL), mExactTeleport(false)
{
}

TheEndGatewayEntity::~TheEndGatewayEntity()
{
    delete mExitPortal;
}

BlockEntity* TheEndGatewayEntity::create()
{
    return new TheEndGatewayEntity();
}

bool TheEndGatewayEntity::isCoolingDown()
{
    return mCooldown > 0;
}

BlockPos TheEndGatewayEntity::findTallestBlock(Level* level, const BlockPos& origin, int radius, bool allowBedrock)
{
    BlockPos* tallest = NULL;
    for (int x = -radius; x < radius; ++x)
    {
        for (int z = -radius; z < radius; ++z)
        {
            if (x == 0 && z == 0 && !allowBedrock)
                continue;

            for (int y = 255; y > (tallest == NULL ? 0 : tallest->y); --y)
            {
                BlockPos candidate(origin.x + x, y, origin.z + z);
                const BlockState* state = level->getBlockState(candidate);
                if (state->isSolidBlockingCube() && (allowBedrock || state->getBlock() != Blocks::bedrock))
                {
                    delete tallest;
                    tallest = new BlockPos(candidate);
                    break;
                }
            }
        }
    }

    BlockPos result = tallest == NULL ? origin : *tallest;
    delete tallest;
    return result;
}

void TheEndGatewayEntity::SetExitPortal(BlockPos* position)
{
    delete mExitPortal;
    mExitPortal = position;
}

BlockPos* TheEndGatewayEntity::findValidSpawnInChunk(LevelChunk* chunk)
{
    BlockPos from = BlockPos(chunk->x << 4, 30, chunk->z << 4).offset(1, 0, 1);
    int top = chunk->getHighestSectionPosition();
    BlockPos to = BlockPos(chunk->x * 16 + 15, top + 15, chunk->z * 16 + 15).offset(-1, 0, -1);

    BlockPos* spawn = NULL;
    double spawnDistance = 0.0;
    Iterator<BlockPos*>* positions = BlockPos::between(from, to);
    while (positions->hasNext())
    {
        BlockPos pos = *positions->next();
        if (chunk->getBlockState(pos)->getBlock() == Blocks::end_stone
            && !chunk->getBlockState(pos.above(1))->isSolidBlockingCube()
            && !chunk->getBlockState(pos.above(2))->isSolidBlockingCube())
        {
            double distance = pos.distSqrToCenter(0.0, 0.0, 0.0);
            if (spawn == NULL || distance < spawnDistance)
            {
                delete spawn;
                spawn = new BlockPos(pos);
                spawnDistance = distance;
            }
        }
    }
    return spawn;
}

void TheEndGatewayEntity::createExitPortal(const BlockPos& position)
{
    Random random;
    int portalNumber = mLevel->getBlockState(getBlockPos())->getValue(EndGatewayBlock::PORTAL_NUMBER);
    EndGatewayFeature(portalNumber).place(mLevel, random, position);

    boost::shared_ptr<BlockEntity> blockEntity = mLevel->getBlockEntity(position);
    if (blockEntity != NULL)
    {
        bool isGateway = eTYPE_DERIVED_FROM(BlockEntityType::THE_END_GATEWAY_ENTITY, blockEntity->GetType());
        if (isGateway)
        {
            boost::shared_ptr<TheEndGatewayEntity> gateway = boost::static_pointer_cast<TheEndGatewayEntity>(
                blockEntity);
            gateway->SetExitPortal(new BlockPos(getBlockPos()));
            gateway->setChanged();
        }
    }
}

void TheEndGatewayEntity::findExitPortal()
{
    int portalNumber = mLevel->getBlockState(getBlockPos())->getValue(EndGatewayBlock::PORTAL_NUMBER);
    Random random(mLevel->getSeed() + portalNumber);
    ChunkPos chunkPos(0, 0);
    for (unsigned int attempt = 0; mExitPortal == NULL && attempt < 1000; ++attempt)
    {
        chunkPos = TheEndDimension::AREAS[portalNumber + 1];
        int xOffset = random.nextInt(2) + 3;
        if (!random.nextBoolean())
            xOffset = -xOffset;
        int zOffset = random.nextInt(2) + 3;
        if (!random.nextBoolean())
            zOffset = -zOffset;
        chunkPos.x += xOffset;
        chunkPos.z += zOffset;

        if (!mLevel->mIsClientSide && mLevel->getChunk(chunkPos.x, chunkPos.z)->isEmpty())
            mLevel->getChunkSource()->create(chunkPos.x, chunkPos.z);

        LevelChunk* chunk = mLevel->getChunk(chunkPos.x, chunkPos.z);
        LevelChunk::BlockEntityMap* blockEntities = chunk->getBlockEntities();
        if (!blockEntities->empty())
        {
            for (LevelChunk::BlockEntityMap::iterator it = blockEntities->begin(); it != blockEntities->end(); ++it)
            {
                if (it->second->GetType() == BlockEntityType::THE_END_GATEWAY_ENTITY)
                {
                    boost::shared_ptr<TheEndGatewayEntity> gateway = boost::static_pointer_cast<TheEndGatewayEntity>(
                        it->second);
                    SetExitPortal(new BlockPos(gateway->getBlockPos()));
                    setChanged();
                    return;
                }
            }
        }
        SetExitPortal(findValidSpawnInChunk(chunk));
    }

    if (mExitPortal == NULL)
    {
        SetExitPortal(new BlockPos(chunkPos.getMiddleBlockPosition(75)));
        Random islandRandom(mExitPortal->compressBlockPos());
        EndIslandFeature().place(mLevel, islandRandom, *mExitPortal);
    }

    BlockPos tallest = findTallestBlock(mLevel, chunkPos.getMiddleBlockPosition(0), 7, true);
    SetExitPortal(new BlockPos(tallest.above(10)));
    createExitPortal(*mExitPortal);
    setChanged();
}

BlockPos TheEndGatewayEntity::findExitPosition()
{
    return findTallestBlock(mLevel, *mExitPortal, 5, false).above();
}

void TheEndGatewayEntity::triggerCooldown()
{
    if (!mLevel->mIsClientSide)
    {
        mCooldown = 40;
        mLevel->blockEvent(getBlockPos(), getBlock(), 1, 0);
        setChanged();
    }
}

void TheEndGatewayEntity::teleportEntity(boost::shared_ptr<Entity> entity)
{
    if (mLevel->mIsClientSide)
        return;
    if (isCoolingDown())
        return;
    bool isPearl = eTYPE_DERIVED_FROM(EntityType::THROWN_ENDERPEARL, entity->GetType());
    if (isPearl && boost::static_pointer_cast<ThrownEnderpearl>(entity)->getOwner() != NULL)
    {
        teleportEntity(boost::static_pointer_cast<ThrownEnderpearl>(entity)->getOwner());
        entity->remove();
        return;
    }
    mCooldown = 100;
    if (mExitPortal == NULL && mLevel->mDimension->getType() == DimensionType::THE_END)
        findExitPortal();
    if (mExitPortal != NULL)
    {
        if (entity->isPassenger())
        {
            entity->stopRidingBeforeTeleport();
        }
        else if (entity->isVehicle())
        {
            std::vector<boost::weak_ptr<Entity> >* passengers = entity->getPassengers();
            while (!passengers->empty())
            {
                boost::shared_ptr<Entity> passenger = passengers->front().lock();
                passenger->stopRiding();
            }
        }
        mLevel->getChunkSource()->getOrCreateChunk(mExitPortal->x >> 4, mExitPortal->z >> 4, false);
        BlockPos target;
        if (mExactTeleport)
        {
            target = *mExitPortal;
        }
        else
        {
            target = findExitPosition();
            if (target.equals(*mExitPortal) || target.equals(mExitPortal->above()))
            {
                mLevel->setBlockAndUpdate(mExitPortal->offset(0, 1, 0), Blocks::bedrock->defaultBlockState());
                mLevel->setBlockAndUpdate(mExitPortal->offset(0, 2, 0), Blocks::bedrock->defaultBlockState());
                mLevel->setBlockAndUpdate(mExitPortal->offset(0, 3, 0), Blocks::air->defaultBlockState());
                mLevel->setBlockAndUpdate(mExitPortal->offset(0, 4, 0), Blocks::air->defaultBlockState());
                target = mExitPortal->offset(0, 3, 0);
            }
        }
        entity->teleportTo((double)target.x, (double)target.y, (double)target.z);
    }
    triggerCooldown();
}

CompoundTag* TheEndGatewayEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putLong(L"Age", mAge);
    if (mExitPortal != NULL)
        tag->put(L"ExitPortal", NbtUtils::writeBlockPos(*mExitPortal));
    if (mExactTeleport)
        tag->putBoolean(L"ExactTeleport", mExactTeleport);
    return tag;
}

void TheEndGatewayEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mAge = tag->getLong(L"Age");
    if (tag->contains(L"ExitPortal", Tag::TAG_COMPOUND))
        SetExitPortal(new BlockPos(NbtUtils::readBlockPos(tag->getCompound(L"ExitPortal"))));
    mExactTeleport = tag->getBoolean(L"ExactTeleport");
}

double TheEndGatewayEntity::getViewDistance()
{
    return 65536.0;
}

bool TheEndGatewayEntity::isSpawning()
{
    return mAge < 200;
}

void TheEndGatewayEntity::tick()
{
    bool spawning = isSpawning();
    bool cooling = isCoolingDown();
    ++mAge;
    if (cooling)
    {
        --mCooldown;
    }
    else if (!mLevel->mIsClientSide)
    {
        std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(Entity),
                                                                                       AABB::newTemp(getBlockPos()));
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
        {
            bool isCrystal = (*it)->GetType() == EntityType::ENDER_CRYSTAL;
            if (isCrystal)
                continue;
            bool isDragon = (*it)->GetType() == EntityType::ENDER_DRAGON;
            if (isDragon)
                continue;
            teleportEntity(*it);
            break;
        }
        delete entities;

        if (mAge % 2400 == 0)
            triggerCooldown();
    }
    if (spawning != isSpawning() || cooling != isCoolingDown())
        setChanged();
}

float TheEndGatewayEntity::getSpawnPercent(float partialTick)
{
    return Mth::clamp(((float)mAge + partialTick) / 200.0F, 0.0F, 1.0F);
}

float TheEndGatewayEntity::getCooldownPercent(float partialTick)
{
    return 1.0F - Mth::clamp(((float)mCooldown - partialTick) / 40.0F, 0.0F, 1.0F);
}

boost::shared_ptr<Packet> TheEndGatewayEntity::getUpdatePacket()
{
    return boost::shared_ptr<Packet>(
        new ClientboundBlockEntityDataPacket(mPos, 8, mLevel->mDimension->getType()->getId(), getUpdateTag()));
}

CompoundTag* TheEndGatewayEntity::getUpdateTag()
{
    return save(new CompoundTag());
}

bool TheEndGatewayEntity::triggerEvent(int id, int param)
{
    if (id == 1)
    {
        mCooldown = 40;
        return true;
    }
    return BlockEntity::triggerEvent(id, param);
}

void TheEndGatewayEntity::setChanged()
{
    BlockEntity::setChanged();
    if (mLevel != NULL && !mLevel->mIsClientSide)
        mLevel->updateObservingBlocksAt(mPos, getBlock());
}

bool TheEndGatewayEntity::shouldRenderFace(const Direction* direction)
{
    return getBlock()->defaultBlockState()->shouldRenderFace(mLevel, getBlockPos(), direction);
}

int TheEndGatewayEntity::getParticleAmount()
{
    int amount = 0;
    for (std::vector<const Direction*>::iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end(); ++it)
    {
        amount += shouldRenderFace(*it) ? 1 : 0;
    }
    return amount;
}

boost::shared_ptr<BlockEntity> TheEndGatewayEntity::clone()
{
    boost::shared_ptr<TheEndGatewayEntity> copy(new TheEndGatewayEntity());
    BlockEntity::clone(copy);
    copy->mAge = mAge;
    copy->mCooldown = mCooldown;
    copy->mExitPortal = new BlockPos(*mExitPortal);
    return copy;
}

unsigned long long TheEndGatewayEntity::GetType()
{
    return BlockEntityType::THE_END_GATEWAY_ENTITY;
}

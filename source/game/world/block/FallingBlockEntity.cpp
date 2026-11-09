#include "game/world/block/FallingBlockEntity.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/DamageSource.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/Tag.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/AnvilBlock.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EntityDataSerializer.hpp"
#include "game/world/block/FallingBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

const EntityDataAccessor<BlockPos>* FallingBlockEntity::DATA_START_POS = NULL;

FallingBlockEntity::FallingBlockEntity(Level* level, double x, double y, double z, const BlockState* blockState)
    : Entity(level, true)
{
    _init();
    mBlockState = blockState;
    mBlocksBuilding = true;
    setSize(0.98f, 0.98f);
    setPos(x, y + (1.0f - mBbHeight) * 0.5f, z);
    mXd = 0.0;
    mYd = 0.0;
    mZd = 0.0;
    mXo = x;
    mYo = y;
    mZo = z;
    mXOld = x;
    mYOld = y;
    mZOld = z;
    setStartPos(BlockPos(this));
}

FallingBlockEntity::FallingBlockEntity(Level* level) : Entity(level, true)
{
    _init();
}

FallingBlockEntity::~FallingBlockEntity()
{
    delete mBlockEntityData;
}

void FallingBlockEntity::staticCtor()
{
    DATA_START_POS = SynchedEntityData::defineId(EntityType::FALLING_BLOCK_ENTITY, EntityType::ENTITY,
                                                 EntityDataSerializers::BLOCK_POS);
}

void FallingBlockEntity::_init()
{
    fjDerivedCtorCalls();
    mBlockState = NULL;
    mTime = 0;
    mDropItem = true;
    mCancelDrop = false;
    mHurtsEntities = false;
    mFallHurtMax = 40;
    mFallHurtAmount = 2.0f;
    mBlockEntityData = NULL;
    mIgnoreVerticalCollision = mLevel->mIsClientSide;
}

void FallingBlockEntity::setStartPos(const BlockPos& pos)
{
    mEntityData->set(DATA_START_POS, pos, true);
}

void FallingBlockEntity::defineSynchedData()
{
    mEntityData->define(DATA_START_POS, BlockPos::ZERO);
}

bool FallingBlockEntity::isAttackable()
{
    return false;
}

bool FallingBlockEntity::makeStepSound()
{
    return false;
}

bool FallingBlockEntity::isPickable()
{
    return !mRemoved;
}

void FallingBlockEntity::tick()
{
    PIXBeginNamedEvent(0.0F, "Falling Block Entity tick");

    Block* block = mBlockState->getBlock();
    if (mBlockState->getMaterial() == Material::AIR)
    {
        remove();
        PIXEndNamedEvent();
        return;
    }

    mXo = mX;
    mYo = mY;
    mZo = mZ;
    if (mTime++ == 0)
    {
        BlockPos startPos(shared_from_this());
        if (mLevel->getBlockState(startPos)->getBlock() == block)
        {
            mLevel->removeBlock(startPos);
        }
        else if (!mLevel->mIsClientSide)
        {
            remove();
            PIXEndNamedEvent();
            return;
        }
    }

    if (!isNoGravity())
    {
        mYd -= 0.04f;
    }

    move(eMoverType_Self, mXd, mYd, mZd, false);

    if (!mLevel->mIsClientSide)
    {
        MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
        if (masterGameMode != NULL && masterGameMode->IsInKillBox(shared_from_this()))
        {
            remove();
        }

        BlockPos pos(shared_from_this());
        bool concretePowder = mBlockState->getBlock() == Blocks::concrete_powder;
        bool inWater = concretePowder && mLevel->getBlockState(pos)->getMaterial() == Material::WATER;
        double speedSq = mXd * mXd + mYd * mYd + mZd * mZd;
        if (concretePowder && speedSq > 1.0)
        {
            HitResult* hitResult = mLevel->clip(Vec3::newTemp(mXo, mYo, mZo), Vec3::newTemp(mX, mY, mZ), true);
            if (hitResult != NULL)
            {
                if (mLevel->getBlockState(hitResult->getBlockPos())->getMaterial() == Material::WATER)
                {
                    pos = hitResult->getBlockPos();
                    inWater = true;
                }
                delete hitResult;
            }
        }

        if (mOnGround || inWater)
        {
            const BlockState* landedOn = mLevel->getBlockState(pos);
            if (!inWater && FallingBlock::isFree(mLevel->getBlockState(BlockPos(mX, mY - 0.01, mZ))))
            {
                mOnGround = false;
                return;
            }

            mXd *= 0.7f;
            mZd *= 0.7f;
            mYd *= -0.5;
            if (landedOn->getBlock() != Blocks::piston_extension)
            {
                remove();
                if (!mCancelDrop)
                {
                    if (mLevel->mayPlace(block, pos, true, Direction::UP, boost::shared_ptr<Entity>())
                        && (inWater || !FallingBlock::isFree(mLevel->getBlockState(pos.below())))
                        && mLevel->setBlock(pos, mBlockState, Block::UPDATE_ALL, false))
                    {
                        FallingBlock* fallingBlock = dynamic_cast<FallingBlock*>(block);
                        if (fallingBlock != NULL)
                        {
                            fallingBlock->onLand(mLevel, pos, mBlockState, landedOn);
                        }

                        if (mBlockEntityData != NULL && block->mBlockEntity)
                        {
                            boost::shared_ptr<BlockEntity> blockEntity = mLevel->getBlockEntity(pos);
                            if (blockEntity != NULL)
                            {
                                CompoundTag* blockEntityTag = blockEntity->save(new CompoundTag());
                                boost::unordered_map<std::wstring, Tag*>* tags = mBlockEntityData->getAllTagsMap();
                                for (boost::unordered_map<std::wstring, Tag*>::iterator it = tags->begin();
                                     it != tags->end(); ++it)
                                {
                                    Tag* tag = it->second;
                                    if (it->first.compare(L"x") != 0 && it->first.compare(L"y") != 0
                                        && it->first.compare(L"z") != 0)
                                    {
                                        blockEntityTag->put(it->first, tag->copy());
                                    }
                                }
                                delete tags;
                                blockEntity->load(blockEntityTag);
                                blockEntity->setChanged();
                            }
                        }
                    }
                    else if (mDropItem && mLevel->getGameRules()->getBoolean(GameRules::RULE_DOENTITYDROPS))
                    {
                        spawnAtLocation(boost::shared_ptr<ItemInstance>(
                                            new ItemInstance(block, 1, block->getSpawnResourcesAuxValue(mBlockState))),
                                        0.0f);
                    }
                }
                else if (dynamic_cast<FallingBlock*>(block) != NULL)
                {
                    static_cast<FallingBlock*>(block)->onBroken(mLevel, pos);
                }
            }
        }
        else
        {
            if (mTime > 100 && !mLevel->mIsClientSide && (pos.y < 1 || pos.y > 256))
            {
                if (mDropItem && mLevel->getGameRules()->getBoolean(GameRules::RULE_DOENTITYDROPS))
                {
                    spawnAtLocation(boost::shared_ptr<ItemInstance>(
                                        new ItemInstance(block, 1, block->getSpawnResourcesAuxValue(mBlockState))),
                                    0.0f);
                }
                remove();
            }
        }
    }

    mXd *= 0.98f;
    mYd *= 0.98f;
    mZd *= 0.98f;
    PIXEndNamedEvent();
}

void FallingBlockEntity::causeFallDamage(float fallDistance, float damageMultiplier)
{
    Block* block = mBlockState->getBlock();
    if (!mHurtsEntities)
    {
        return;
    }

    int distance = (int)(fallDistance - 1.0f);
    if (fallDistance - 1.0f > (float)distance)
    {
        ++distance;
    }

    if (distance <= 0)
    {
        return;
    }

    std::vector<boost::shared_ptr<Entity> >* entities = new std::vector<boost::shared_ptr<Entity> >(
        *mLevel->getEntities(shared_from_this(), mBoundingBox, NULL));
    bool anvil = block == Blocks::anvil;
    const DamageSource* damageSource = anvil ? DamageSource::ANVIL : DamageSource::FALLING_BLOCK;
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        (*it)->hurt(damageSource, (float)std::min(Mth::floor((float)distance * mFallHurtAmount), mFallHurtMax));
    }
    delete entities;

    if (anvil && mRandom->nextFloat() < (float)distance * 0.05f + 0.05f)
    {
        int anvilDamage = mBlockState->getValue(AnvilBlock::DAMAGE) + 1;
        if (anvilDamage > 2)
        {
            mCancelDrop = true;
        }
        else
        {
            mBlockState = mBlockState->setValue(AnvilBlock::DAMAGE, anvilDamage);
        }
    }
}

void FallingBlockEntity::addDataWalkers(DataFixerUpper*)
{
}

void FallingBlockEntity::addAdditonalSaveData(CompoundTag* tag)
{
    Block* block = mBlockState != NULL ? mBlockState->getBlock() : Blocks::air;
    ResourceLocation location = Block::blockRegistry->getKey(block);
    tag->putString(L"Block", location.toString());
    tag->putByte(L"Data", (unsigned char)block->convertBlockStateToLegacyData(mBlockState));
    tag->putInt(L"Time", (unsigned char)mTime);
    tag->putBoolean(L"DropItem", mDropItem);
    tag->putBoolean(L"HurtEntities", mHurtsEntities);
    tag->putFloat(L"FallHurtAmount", mFallHurtAmount);
    tag->putInt(L"FallHurtMax", mFallHurtMax);
    if (mBlockEntityData != NULL)
    {
        tag->put(L"TileEntityData", mBlockEntityData);
    }
}

void FallingBlockEntity::readAdditionalSaveData(CompoundTag* tag)
{
    int data = tag->getByte(std::wstring(L"Data"));
    if (tag->contains(std::wstring(L"Block"), 8))
    {
        mBlockState = Block::byString(tag->getString(std::wstring(L"Block")))->getBlockState(data);
    }
    else if (tag->contains(std::wstring(L"TileID"), 99))
    {
        mBlockState = Block::byId(tag->getInt(std::wstring(L"TileID")))->getBlockState(data);
    }
    else
    {
        mBlockState = Block::byId(tag->getByte(std::wstring(L"Tile")))->getBlockState(data);
    }

    mTime = tag->getInt(std::wstring(L"Time")) & 0xff;

    Block* block = mBlockState->getBlock();
    if (tag->contains(std::wstring(L"HurtEntities"), 99))
    {
        mHurtsEntities = tag->getBoolean(std::wstring(L"HurtEntities"));
        mFallHurtAmount = tag->getFloat(std::wstring(L"FallHurtAmount"));
        mFallHurtMax = tag->getInt(std::wstring(L"FallHurtMax"));
    }
    else if (block == Blocks::anvil)
    {
        mHurtsEntities = true;
    }

    if (tag->contains(std::wstring(L"DropItem"), 99))
    {
        mDropItem = tag->getBoolean(std::wstring(L"DropItem"));
    }

    if (tag->contains(std::wstring(L"TileEntityData"), 10))
    {
        mBlockEntityData = tag->getCompound(std::wstring(L"TileEntityData"));
    }

    if (block == NULL || block->defaultBlockState()->getMaterial() == Material::AIR)
    {
        mBlockState = Blocks::sand->defaultBlockState();
    }
}

Level* FallingBlockEntity::getLevel()
{
    return mLevel;
}

void FallingBlockEntity::setHurtsEntities(bool hurtsEntities)
{
    mHurtsEntities = hurtsEntities;
}

const BlockState* FallingBlockEntity::getBlock()
{
    return mBlockState;
}

bool FallingBlockEntity::displayFireAnimation()
{
    return false;
}

bool FallingBlockEntity::onlyOpCanSetNbt()
{
    return true;
}

Entity* FallingBlockEntity::create(Level* level)
{
    return new FallingBlockEntity(level);
}

unsigned long long FallingBlockEntity::GetType()
{
    return EntityType::FALLING_BLOCK_ENTITY;
}

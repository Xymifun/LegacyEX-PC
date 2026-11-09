#include "game/core/LightningBolt.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameRules.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/PlayerList.hpp"

LightningBolt::LightningBolt(Level* level, double x, double y, double z, bool visualOnly)
    : GlobalEntity(level),
      mLife(0),
      mSeed(0),
      mFlashes(0),
      mVisualOnly(visualOnly)
{
    fjDerivedCtorCalls();
    moveTo(x, y, z, 0.0f, 0.0f);

    mLife = 2;
    mSeed = mRandom->nextLong();
    mFlashes = 1;

    BlockPos pos(mX, mY, mZ);

    if (!visualOnly && !level->mIsClientSide && level->getGameRules()->getBoolean(GameRules::RULE_DOFIRETICK)
        && (level->getDifficulty() == Difficulty::NORMAL || level->getDifficulty() == Difficulty::HARD)
        && level->hasChunksAt(pos, 10))
    {
        if (gMinecraftServer->getPlayers()->isTrackingBlock(pos, level->mDimension)
            && level->getBlockState(pos)->getMaterial() == Material::AIR && Blocks::fire->mayPlace(level, pos))
        {
            level->setBlockAndUpdate(pos, Blocks::fire->defaultBlockState());
        }

        for (int i = 0; i < 4; ++i)
        {
            BlockPos firePos = pos.offset(mRandom->nextInt(3) - 1, mRandom->nextInt(3) - 1, mRandom->nextInt(3) - 1);

            if (gMinecraftServer->getPlayers()->isTrackingBlock(pos, level->mDimension)
                && level->getBlockState(firePos)->getMaterial() == Material::AIR && Blocks::fire->mayPlace(level, pos))
            {
                level->setBlockAndUpdate(firePos, Blocks::fire->defaultBlockState());
            }
        }
    }
}

unsigned long long LightningBolt::GetType()
{
    return EntityType::LIGHTNING_BOLT;
}

void LightningBolt::defineSynchedData()
{
}

void LightningBolt::tick()
{
    Entity::tick();

    if (mLife == 2)
    {
        mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, SoundEvents::LIGHTNING_THUNDER,
                          SoundSource::eSoundSource_Weather, 10000.0f, 0.8f + mRandom->nextFloat() * 0.2f, 16.0f);
        mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, SoundEvents::LIGHTNING_IMPACT,
                          SoundSource::eSoundSource_Weather, 2.0f, 0.5f + mRandom->nextFloat() * 0.2f, 16.0f);
    }

    --mLife;

    if (mLife < 0)
    {
        if (mFlashes == 0)
        {
            remove();
        }
        else if (mLife < -mRandom->nextInt(10))
        {
            --mFlashes;
            mLife = 1;

            if (!mVisualOnly && !mLevel->mIsClientSide)
            {
                mSeed = mRandom->nextLong();
                BlockPos pos(shared_from_this());

                if (mLevel->getGameRules()->getBoolean(GameRules::RULE_DOFIRETICK) && mLevel->hasChunksAt(pos, 10)
                    && gMinecraftServer->getPlayers()->isTrackingBlock(pos, mLevel->mDimension)
                    && mLevel->getBlockState(pos)->getBlock()->mMaterial == Material::AIR
                    && Blocks::fire->mayPlace(mLevel, pos))
                {
                    mLevel->setBlockAndUpdate(pos, Blocks::fire->defaultBlockState());
                }
            }
        }
    }

    if (mLife >= 0)
    {
        if (mLevel->mIsClientSide)
        {
            mLevel->setSkyFlashTime(2);
        }
        else if (!mVisualOnly)
        {
            std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntities(
                shared_from_this(), AABB::newTemp(mX - 3.0, mY - 3.0, mZ - 3.0, mX + 3.0, mY + 9.0, mZ + 3.0), NULL);

            std::vector<boost::shared_ptr<Entity> >::iterator it;
            std::vector<boost::shared_ptr<Entity> >::iterator end;
            for (end = entities->end(), it = entities->begin(); it != end; ++it)
            {
                boost::shared_ptr<Entity> entity = *it;

                if (std::find(mHitEntities.begin(), mHitEntities.end(), entity->getId()) == mHitEntities.end())
                {
                    entity->thunderHit(this);
                    mHitEntities.push_back(entity->getId());
                }
            }
        }
    }
}

bool LightningBolt::shouldRender(double distanceSqr)
{
    return mLife >= 0;
}

void LightningBolt::readAdditionalSaveData(CompoundTag* tag)
{
}

void LightningBolt::addAdditonalSaveData(CompoundTag* tag)
{
}

SoundSource::ESoundSource LightningBolt::getSoundSource()
{
    return SoundSource::eSoundSource_Weather;
}

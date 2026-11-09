#include "game/core/CaveSpider.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/Stray.hpp"
#include "game/entity/mob/AbstractSkeleton.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/Skeleton.hpp"
#include "game/entity/mob/WitherSkeleton.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/BlockPos.hpp"


CaveSpider::CaveSpider(Level* level) : Spider(level, false)
{
    fjDerivedCtorCalls();
    setSize(0.7f, 0.5f);
}

Entity* CaveSpider::create(Level* level)
{
    return new CaveSpider(level);
}

void CaveSpider::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::CAVE_SPIDER);
}

void CaveSpider::registerAttributes()
{
    Spider::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(12.0);
}

bool CaveSpider::doHurtTarget(boost::shared_ptr<Entity> target)
{
    if (Monster::doHurtTarget(target))
    {
        if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, target->GetType()))
        {
            int duration;

            if (mLevel->getDifficulty() == Difficulty::NORMAL)
            {
                duration = 7;
            }
            else if (mLevel->getDifficulty() == Difficulty::HARD)
            {
                duration = 15;
            }
            else
            {
                return true;
            }

            boost::dynamic_pointer_cast<LivingEntity>(target)->addEffect(
                new MobEffectInstance(MobEffects::POISON, duration * 20, 0), boost::shared_ptr<Entity>());
        }

        return true;
    }

    return false;
}

MobGroupData* CaveSpider::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& name)
{
    if (mLevel->mRandom->nextInt(100) == 0)
    {
        boost::shared_ptr<AbstractSkeleton> rider;

        if (mLevel->canSeeSky(BlockPos(mX, mY, mZ)) && !mLevel->isDay() && mLevel->mRandom->rollPercentageChance(80)
            && (mLevel->getBiome(BlockPos(mX, mY, mZ)) == Biome::ICE_PLAINS
                || mLevel->getBiome(BlockPos(mX, mY, mZ)) == Biome::ICE_MOUNTAINS
                || mLevel->getBiome(BlockPos(mX, mY, mZ)) == Biome::ICE_PLAINS_SPIKES
                || mLevel->getBiome(BlockPos(mX, mY, mZ)) == Biome::FROZEN_RIVER
                || mLevel->getBiome(BlockPos(mX, mY, mZ)) == Biome::COLD_BEACH
                || mLevel->getBiome(BlockPos(mX, mY, mZ)) == Biome::FROZEN_OCEAN
                || mLevel->getBiome(BlockPos(mX, mY, mZ)) == Biome::DEEP_FROZEN_OCEAN))
        {
            rider = boost::shared_ptr<Stray>(new Stray(mLevel));
        }
        else if (mLevel->mRandom->rollPercentageChance(80) && mLevel->getBiome(BlockPos(mX, mY, mZ)) == Biome::HELL)
        {
            rider = boost::shared_ptr<WitherSkeleton>(new WitherSkeleton(mLevel));
        }
        else
        {
            rider = boost::shared_ptr<Skeleton>(new Skeleton(mLevel));
        }

        rider->moveTo(mX, mY, mZ, mYRot, 0.0f);
        rider->finalizeMobSpawn(difficulty, NULL, L"");
        mLevel->addEntity(rider);
        rider->startRiding(shared_from_this());
    }

    return data;
}

float CaveSpider::getEyeHeight()
{
    return 0.45f;
}

ResourceLocation* CaveSpider::getDefaultLootTable()
{
    return &BuiltInLootTables::CAVE_SPIDER;
}

unsigned long long CaveSpider::GetType()
{
    return EntityType::CAVE_SPIDER;
}

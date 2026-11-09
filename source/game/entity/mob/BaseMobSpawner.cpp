#include "game/entity/mob/BaseMobSpawner.hpp"
#include "game/world/LevelEvent.hpp"

#include <algorithm>

#include "game/client/render/ParticleType.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/EntityType.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/WeighedRandom.hpp"
#include "game/world/Level.hpp"
#include "game/world/chunk/OldChunkStorage.hpp"

BaseMobSpawner::BaseMobSpawner()
{
    mDelay = 20;
    mNextSpawnData = new SpawnData();
    mSpin = moSpin = 0.0;
    mMinSpawnDelay = 200;
    mMaxSpawnDelay = 800;
    mSpawnCount = 4;
    mDisplayEntity = boost::shared_ptr<Entity>();
    mMaxNearbyEntities = 6;
    mRequiredPlayerRange = 16;
    mSpawnRange = 4;
}

BaseMobSpawner::~BaseMobSpawner()
{
    std::vector<SpawnData*>::iterator found = std::find(mSpawnPotentials.begin(), mSpawnPotentials.end(),
                                                        mNextSpawnData);
    if (found == mSpawnPotentials.end())
        delete mNextSpawnData;
    for (std::vector<SpawnData*>::iterator it = mSpawnPotentials.begin(); it != mSpawnPotentials.end(); ++it)
        delete *it;
}

ResourceLocation BaseMobSpawner::getEntityId()
{
    return ResourceLocation(mNextSpawnData->getTag()->getString(L"id"));
}

void BaseMobSpawner::setEntityId(const ResourceLocation& entityId)
{
    if (EntityIO::isValidEntityType(entityId))
        mNextSpawnData->getTag()->putString(L"id", entityId.toString());
}

bool BaseMobSpawner::isNearPlayer()
{
    BlockPos pos = getPos();
    return getLevel()->hasNearPlayer(pos.x + 0.5, pos.y + 0.5, pos.z + 0.5, static_cast<double>(mRequiredPlayerRange));
}

bool BaseMobSpawner::hasMob()
{
    CompoundTag* tag = mNextSpawnData->getTag();
    ResourceLocation entityId(tag->getString(L"id"));
    return EntityIO::isValidEntityType(entityId);
}

void BaseMobSpawner::tick()
{
    if (!isNearPlayer())
    {
        moSpin = mSpin;
        return;
    }
    if (!hasMob())
        return;
    BlockPos origin = getPos();
    if (getLevel()->mIsClientSide)
    {
        double x = (float)origin.x + getLevel()->mRandom->nextFloat();
        double y = (float)origin.y + getLevel()->mRandom->nextFloat();
        double z = (float)origin.z + getLevel()->mRandom->nextFloat();
        getLevel()->addParticle(PARTICLE_SMOKE, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
        getLevel()->addParticle(PARTICLE_FLAME, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
        if (mDelay > 0)
            --mDelay;
        moSpin = mSpin;
        mSpin = static_cast<double>(static_cast<int>(mSpin + 1000.0f / (static_cast<float>(mDelay) + 200.0f)) % 360);
        return;
    }
    if (mDelay == -1)
        delay();
    if (mDelay > 0)
    {
        --mDelay;
        return;
    }
    bool spawned = false;
    for (int i = 0; i < mSpawnCount; ++i)
    {
        CompoundTag* spawnTag = mNextSpawnData->getTag();
        ListTag* position = spawnTag->fjSafeGetListTag(L"Pos", Tag::TAG_DOUBLE);
        Level* level = getLevel();
        int count = position->size();
        double x = count >= 1
                       ? position->getDouble(0)
                       : origin.x + (level->mRandom->nextDouble() - level->mRandom->nextDouble()) * mSpawnRange + 0.5;
        double y = count >= 2 ? position->getDouble(1) : static_cast<double>(origin.y + level->mRandom->nextInt(3) - 1);
        double z = count >= 3
                       ? position->getDouble(2)
                       : origin.z + (level->mRandom->nextDouble() - level->mRandom->nextDouble()) * mSpawnRange + 0.5;
        boost::shared_ptr<Entity> entity = OldChunkStorage::loadEntity(spawnTag, level, x, y, z, false);
        if (entity == NULL)
            return;

        std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(
            typeid(*entity.get()), AABB::newTemp(origin.x, origin.y, origin.z, origin.x + 1, origin.y + 1, origin.z + 1)
                                       ->grow(mSpawnRange, mSpawnRange, mSpawnRange));
        int nearby = static_cast<int>(entities->size());
        delete entities;
        if (nearby >= mMaxNearbyEntities || !getLevel()->canCreateMore(entity->GetType(), Level::eSpawnType_Egg))
        {
            broadcastEvent(2);
            delay();
            return;
        }

        boost::shared_ptr<Mob> mob = eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType())
                                         ? boost::static_pointer_cast<Mob>(entity)
                                         : boost::shared_ptr<Mob>();
        entity->moveTo(entity->mX, entity->mY, entity->mZ, level->mRandom->nextFloat() * 360.0f, 0.0f);
        if (mob == NULL || (mob->checkSpawnRules() && mob->checkSpawnObstruction()))
        {
            if (mNextSpawnData->getTag()->size() == 1 && mNextSpawnData->getTag()->contains(L"id", Tag::TAG_STRING)
                && eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType()))
            {
                boost::shared_ptr<Mob> spawnedMob = boost::static_pointer_cast<Mob>(entity);
                spawnedMob->finalizeMobSpawn(level->getCurrentDifficultyAt(BlockPos(entity)), NULL, std::wstring(L""));
            }
            OldChunkStorage::addEntityToWorld(entity, level);
            level->levelEvent(LevelEvent::PARTICLES_MOBBLOCK_SPAWN, origin, 0);
            mob->playAmbientSound();
            if (mob != NULL)
                mob->spawnAnim();
            spawned = true;
        }
    }
    if (spawned)
        delay();
}

void BaseMobSpawner::delay()
{
    if (mMaxSpawnDelay <= mMinSpawnDelay)
    {
        mDelay = mMinSpawnDelay;
    }
    else
    {
        mDelay = mMinSpawnDelay + getLevel()->mRandom->nextInt(mMaxSpawnDelay - mMinSpawnDelay);
    }
    if (!mSpawnPotentials.empty())
    {
        SpawnData* data = static_cast<SpawnData*>(WeighedRandom::getRandomItem(
            getLevel()->mRandom, reinterpret_cast<std::vector<WeighedRandomItem*>*>(&mSpawnPotentials)));
        setNextSpawnData(data);
    }
    broadcastEvent(1);
}

void BaseMobSpawner::load(CompoundTag* tag)
{
    mDelay = tag->getShort(L"Delay");
    mSpawnPotentials.clear();
    if (tag->contains(L"SpawnPotentials"))
    {
        ListTag* list = tag->getList(L"SpawnPotentials", Tag::TAG_COMPOUND);
        for (int i = 0; i < list->size(); ++i)
            mSpawnPotentials.push_back(new SpawnData(list->getCompound(i)));
    }
    if (tag->contains(L"SpawnData", Tag::TAG_COMPOUND))
    {
        setNextSpawnData(new SpawnData(1, tag->getCompound(L"SpawnData")));
    }
    else if (!mSpawnPotentials.empty())
    {
        setNextSpawnData(static_cast<SpawnData*>(WeighedRandom::getRandomItem(
            getLevel()->mRandom, reinterpret_cast<std::vector<WeighedRandomItem*>*>(&mSpawnPotentials))));
    }
    for (std::vector<SpawnData*>::iterator it = mSpawnPotentials.begin(); it != mSpawnPotentials.end(); ++it)
    {
        if (mNextSpawnData->getTag()->getString(L"id") == (*it)->getTag()->getString(L"id"))
        {
            setNextSpawnData(*it);
            break;
        }
    }
    if (tag->contains(L"MinSpawnDelay", Tag::TAG_ANY_NUMERIC))
    {
        mMinSpawnDelay = tag->getShort(L"MinSpawnDelay");
        mMaxSpawnDelay = tag->getShort(L"MaxSpawnDelay");
        mSpawnCount = tag->getShort(L"SpawnCount");
    }
    if (tag->contains(L"MaxNearbyEntities", Tag::TAG_ANY_NUMERIC))
    {
        mMaxNearbyEntities = tag->getShort(L"MaxNearbyEntities");
        mRequiredPlayerRange = tag->getShort(L"RequiredPlayerRange");
    }
    if (tag->contains(L"SpawnRange", Tag::TAG_ANY_NUMERIC))
        mSpawnRange = tag->getShort(L"SpawnRange");
    if (getLevel() != NULL)
        mDisplayEntity = boost::shared_ptr<Entity>();
}

CompoundTag* BaseMobSpawner::save(CompoundTag* tag)
{
    ResourceLocation entityId = getEntityId();
    if (!entityId.isDefault() && !entityId.getPath().empty())
    {
        tag->putShort(L"Delay", static_cast<short>(mDelay));
        tag->putShort(L"MinSpawnDelay", static_cast<short>(mMinSpawnDelay));
        tag->putShort(L"MaxSpawnDelay", static_cast<short>(mMaxSpawnDelay));
        tag->putShort(L"SpawnCount", static_cast<short>(mSpawnCount));
        tag->putShort(L"MaxNearbyEntities", static_cast<short>(mMaxNearbyEntities));
        tag->putShort(L"RequiredPlayerRange", static_cast<short>(mRequiredPlayerRange));
        tag->putShort(L"SpawnRange", static_cast<short>(mSpawnRange));
        tag->put(L"SpawnData", mNextSpawnData->getTag()->copy());
        ListTag* list = new ListTag();
        if (mSpawnPotentials.empty())
            list->add(mNextSpawnData->save());
        else
        {
            std::vector<SpawnData*>::const_iterator it = mSpawnPotentials.begin();
            for (; it != mSpawnPotentials.end(); ++it)
                list->add((*it)->save());
        }
        tag->put(L"SpawnPotentials", list);
    }
    return tag;
}

boost::shared_ptr<Entity> BaseMobSpawner::getOrCreateDisplayEntity(Level* level)
{
    if (!mDisplayEntity)
        mDisplayEntity = OldChunkStorage::loadEntity(mNextSpawnData->getTag(), getLevel(), false);
    return mDisplayEntity;
}

int BaseMobSpawner::onEventTriggered(int event)
{
    if (event == 2)
    {
        if (getLevel()->mIsClientSide)
        {
            BlockPos pos = getPos();
            getLevel()->levelEvent(LevelEvent::SOUND_DISPENSER_FAIL, pos, 0);

            Random random;
            for (int i = 0; i < 20; ++i)
            {
                double x = (float)pos.x + getLevel()->mRandom->nextFloat();
                double y = (float)pos.y + getLevel()->mRandom->nextFloat();
                double z = (float)pos.z + getLevel()->mRandom->nextFloat();
                double xd = random.nextGaussian() * 0.02;
                double yd = random.nextGaussian() * 0.02;
                double zd = random.nextGaussian() * 0.02;
                getLevel()->addParticle(PARTICLE_EXPLODE, x, y, z, xd, yd, zd, arrayWithLength<int>());
            }
        }
        return 1;
    }

    if (event == 1 && getLevel()->mIsClientSide)
    {
        mDelay = mMinSpawnDelay;
        return 1;
    }

    return 0;
}

void BaseMobSpawner::setNextSpawnData(SpawnData* data)
{
    if (std::find(mSpawnPotentials.begin(), mSpawnPotentials.end(), mNextSpawnData) == mSpawnPotentials.end())
        delete mNextSpawnData;
    mNextSpawnData = data;
}

double BaseMobSpawner::getSpin()
{
    return mSpin;
}

double BaseMobSpawner::getoSpin()
{
    return moSpin;
}

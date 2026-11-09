#include "game/world/generation/VillageSiege.hpp"

#include <string>
#include <vector>

#include "game/core/Mth.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/NaturalMobSpawner.hpp"
#include "game/entity/mob/Zombie.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/Village.hpp"
#include "game/world/generation/Villages.hpp"

VillageSiege::VillageSiege(Level* level) : mVillage()
{
    mHasSetupSiege = false;
    mSiegeState = -1;
    mSiegeCount = 0;
    mNextSpawnTime = 0;
    mVillage = boost::weak_ptr<Village>();
    mSpawnX = mSpawnY = mSpawnZ = 0;
    mLevel = level;
}

Vec3* VillageSiege::findRandomSpawnPos(const BlockPos& pos)
{
    boost::shared_ptr<Village> village = mVillage.lock();
    if (!village)
    {
        return NULL;
    }

    for (int attempt = 0; attempt < 10; ++attempt)
    {
        const BlockPos candidate = pos.offset(mLevel->mRandom->nextInt(16) - 8, mLevel->mRandom->nextInt(6) - 3,
                                              mLevel->mRandom->nextInt(16) - 8);
        if (village->isInside(candidate)
            && NaturalMobSpawner::isSpawnPositionOk(Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND, mLevel, candidate, 10, 10))
        {
            return Vec3::newTemp((double)candidate.x, (double)candidate.y, (double)candidate.z);
        }
    }

    return NULL;
}

bool VillageSiege::tryToSetupSiege()
{
    MutableBlockPos spawnPos(mSpawnX, mSpawnY, mSpawnZ);

    for (std::vector<boost::shared_ptr<Player> >::iterator it = mLevel->mPlayers.begin(); it != mLevel->mPlayers.end();
         ++it)
    {
        if ((*it)->isSpectator())
        {
            continue;
        }

        boost::shared_ptr<Village> village = mLevel->mVillages->getClosestVillage(BlockPos(*it), 1);
        mVillage = village;

        if (!village)
        {
            continue;
        }

        if (village->getDoorCount() < 10)
        {
            continue;
        }

        if (village->getStableAge() < 20)
        {
            continue;
        }

        if (village->getPopulationSize() < 20)
        {
            continue;
        }

        const BlockPos* center = village->getCenter();
        const int centerX = center->x;
        const int centerY = center->y;
        const int centerZ = center->z;
        const float radius = (float)village->getRadius();

        int attempt = 10;
        for (;;)
        {
            const float angle = mLevel->mRandom->nextFloat() * 6.2831855f;
            mSpawnX = centerX + (int)((double)(Mth::cos(angle) * radius) * 0.9);
            mSpawnY = centerY;
            mSpawnZ = centerZ + (int)((double)(Mth::sin(angle) * radius) * 0.9);
            spawnPos.set(mSpawnX, mSpawnY, mSpawnZ);

            bool insideOtherVillage = false;
            std::vector<boost::shared_ptr<Village> >* villages = mLevel->mVillages->getVillages();
            for (std::vector<boost::shared_ptr<Village> >::iterator villageIt = villages->begin();
                 villageIt != villages->end(); ++villageIt)
            {
                if (*villageIt != village && (*villageIt)->isInside(spawnPos))
                {
                    insideOtherVillage = true;
                    break;
                }
            }

            if (insideOtherVillage)
            {
                if (attempt != 1)
                {
                    --attempt;
                    continue;
                }
                return false;
            }

            if (findRandomSpawnPos(spawnPos) != NULL)
            {
                mNextSpawnTime = 0;
                mSiegeCount = 20;
                return true;
            }
            break;
        }
    }

    return false;
}

bool VillageSiege::trySpawn()
{
    Vec3* spawnPos = findRandomSpawnPos(BlockPos(mSpawnX, mSpawnY, mSpawnZ));
    if (spawnPos == NULL)
    {
        return false;
    }

    boost::shared_ptr<Zombie> zombie;
    {
        zombie = boost::shared_ptr<Zombie>(new Zombie(mLevel));
        zombie->finalizeMobSpawn(mLevel->getCurrentDifficultyAt(BlockPos(zombie)), NULL, std::wstring(L""));
    }
    zombie->moveTo(spawnPos->x, spawnPos->y, spawnPos->z, mLevel->mRandom->nextFloat() * 360.0f, 0.0f);
    mLevel->addEntity(zombie);

    boost::shared_ptr<Village> village = mVillage.lock();
    if (!village)
    {
        return false;
    }

    const BlockPos center = *village->getCenter();
    zombie->restrictTo(center, village->getRadius());
    return true;
}

void VillageSiege::tick()
{
    if (mLevel->isDay())
    {
        mSiegeState = 0;
        return;
    }

    if (mSiegeState == 2)
    {
        return;
    }

    if (mSiegeState == 0)
    {
        const float timeOfDay = mLevel->getTimeOfDay(0.0f);
        if (timeOfDay < 0.5 || timeOfDay > 0.501)
        {
            return;
        }

        mSiegeState = mLevel->mRandom->nextInt(10) == 0 ? 1 : 2;
        mHasSetupSiege = false;
        if (mSiegeState == 2)
        {
            return;
        }
    }

    if (!mHasSetupSiege)
    {
        if (!tryToSetupSiege())
        {
            return;
        }

        mHasSetupSiege = true;
    }

    if (mNextSpawnTime > 0)
    {
        --mNextSpawnTime;
        return;
    }

    mNextSpawnTime = 2;

    if (mSiegeCount > 0)
    {
        trySpawn();
        --mSiegeCount;
    }
    else
    {
        mSiegeState = 2;
    }
}

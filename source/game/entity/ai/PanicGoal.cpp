#include "game/entity/ai/PanicGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/util/RandomPos.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/entity/mob/PathfinderMob.hpp"

PanicGoal::PanicGoal(PathfinderMob* mob, double speedModifier) : mMob(mob), mSpeedModifier(speedModifier)
{
    mWantedX = mWantedY = mWantedZ = 0.0;
    setRequiredControlFlags(1);
}

BlockPos* PanicGoal::lookForWater(Level* level, Entity* entity, int range, int verticalRange)
{
    BlockPos origin(entity);
    int originX = origin.x;
    int originY = origin.y;
    int originZ = origin.z;
    float bestDistance = static_cast<float>(range * range * verticalRange * 2);
    BlockPos pos(0, 0, 0);
    BlockPos* result = NULL;

    for (int x = originX - range; x <= originX + range; ++x)
    {
        for (int y = originY - verticalRange; y <= originY + verticalRange; ++y)
        {
            for (int z = originZ - range; z <= originZ + range; ++z)
            {
                pos.x = x;
                pos.y = y;
                pos.z = z;
                if (level->getBlockState(pos)->getMaterial() == Material::WATER)
                {
                    const float distance = static_cast<float>(
                        (x - originX) * (x - originX) + (y - originY) * (y - originY) + (z - originZ) * (z - originZ));
                    if (distance < bestDistance)
                    {
                        bestDistance = distance;
                        result = new BlockPos(pos);
                    }
                }
            }
        }
    }

    return result;
}

bool PanicGoal::findRandomPosition()
{
    Vec3* position = RandomPos::getPos(boost::dynamic_pointer_cast<PathfinderMob>(mMob->shared_from_this()), 5, 4, -1);
    if (position == NULL)
    {
        return false;
    }

    mWantedX = position->x;
    mWantedY = position->y;
    mWantedZ = position->z;
    return true;
}

bool PanicGoal::canUse()
{
    if (mMob->getLastHurtByMob().get() == NULL && !mMob->isOnFire())
    {
        return false;
    }

    if (mMob->isOnFire())
    {
        BlockPos* water = lookForWater(mMob->mLevel, mMob, 5, 4);
        if (water != NULL)
        {
            mWantedX = static_cast<double>(water->x);
            mWantedY = static_cast<double>(water->y);
            mWantedZ = static_cast<double>(water->z);
            delete water;
            return true;
        }
    }

    return findRandomPosition();
}

void PanicGoal::start()
{
    mMob->getNavigation()->moveTo(mWantedX, mWantedY, mWantedZ, mSpeedModifier);
}

bool PanicGoal::canContinueToUse()
{
    return !mMob->getNavigation()->isDone();
}

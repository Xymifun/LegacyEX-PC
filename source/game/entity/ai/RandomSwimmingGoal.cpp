#include "game/entity/ai/RandomSwimmingGoal.hpp"

#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <cmath>

#include "game/util/Math.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/Random.hpp"
#include "game/util/RandomPos.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"

RandomSwimmingGoal::RandomSwimmingGoal(PathfinderMob* mob, double speedModifier, int interval, int xz, int y)
    : RandomStrollGoal(mob, speedModifier, interval),
      mXz(xz),
      mY(y)
{
}

RandomSwimmingGoal::~RandomSwimmingGoal()
{
}

void RandomSwimmingGoal::getWaterHeights(MutableBlockPos pos, int& topY, float& waterY, float& minY, float& maxY)
{
    Level* level = ((Entity*)mMob)->mLevel;
    int top = level->getAboveTopSolidBlock(pos, false, false);
    topY = top;
    waterY = (float)top;

    while (waterY < (float)level->getMaxBuildHeight())
    {
        pos.setY((int)waterY);
        if (level->getBlockState(pos)->getMaterial() != Material::WATER)
        {
            waterY -= 1.0f;
            break;
        }
        waterY += 1.0f;
    }

    maxY = std::min(waterY - 1.5f, (float)((Entity*)mMob)->mY + (float)mY);
    minY = std::max(waterY - 20.0f, (float)((Entity*)mMob)->mY - (float)mY);
    if (minY <= (float)topY)
    {
        minY = (float)topY + 0.5f;
    }
    if (maxY < minY)
    {
        maxY = minY;
    }
}

Vec3* RandomSwimmingGoal::getPosition()
{
    Vec3* target = NULL;
    if (((Entity*)mMob)->isInWater() || ((Entity*)mMob)->isOverWater())
    {
        boost::shared_ptr<PathfinderMob> mob = boost::static_pointer_cast<PathfinderMob>(
            ((Entity*)mMob)->shared_from_this());
        int attempts = 0;
        int topY = 0;
        float waterY;
        float minY;
        float maxY;

        do
        {
            target = RandomPos::getPos(mob, mXz, mY, -1);
            if (target != NULL)
            {
                getWaterHeights(MutableBlockPos(target), topY, waterY, minY, maxY);
                target->y = (double)mMob->getRandom()->nextFloat(minY, maxY);
            }
            ++attempts;
        } while (target != NULL && attempts < 30
                 && ((Entity*)mMob)->mLevel->getBlock(BlockPos(target))->mMaterial != Material::WATER);

        if (target != NULL)
        {
            Vec3* delta = target->subtract(((Entity*)mMob)->getPos(0.0f));
            const double horizontalDistance = std::sqrt(delta->x * delta->x + delta->z * delta->z);
            const double angle = Math::wrapDegrees(-std::atan2(delta->y, horizontalDistance));
            if (((Entity*)mMob)
                        ->mLevel->getLiquidBlock(BlockPos((int)target->x, (int)std::ceil(target->y), (int)target->z))
                        ->mMaterial
                    != Material::WATER
                || (angle > 50.0 && angle < 130.0) || (angle < -50.0 && angle > -130.0))
            {
                target = NULL;
            }
        }
    }
    return target;
}

bool RandomSwimmingGoal::canContinueToUse()
{
    if (mMob->getNavigation()->isRiverFollowing() || !((Entity*)mMob)->isInWaterOrBubble())
    {
        return false;
    }
    return RandomStrollGoal::canContinueToUse();
}

bool RandomSwimmingGoal::canUse()
{
    if (mMob->getNavigation()->isRiverFollowing() || !((Entity*)mMob)->isInWaterOrBubble())
    {
        return false;
    }
    return RandomStrollGoal::canUse();
}

#include "game/entity/ai/WaterAvoidingRandomFlyingGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Mth.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/RandomPos.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/MutableBlockPos.hpp"

WaterAvoidingRandomFlyingGoal::WaterAvoidingRandomFlyingGoal(PathfinderMob* mob, double speedModifier)
    : WaterAvoidingRandomStrollGoal(mob, speedModifier)
{
}

WaterAvoidingRandomFlyingGoal::~WaterAvoidingRandomFlyingGoal()
{
}

Vec3* WaterAvoidingRandomFlyingGoal::getTreePos()
{
    BlockPos origin(mMob);
    MutableBlockPos above(0, 0, 0);
    MutableBlockPos below(0, 0, 0);

    const int minX = Mth::floor(mMob->mX - 3.0);
    const int minY = Mth::floor(mMob->mY - 6.0);
    const int minZ = Mth::floor(mMob->mZ - 3.0);
    const int maxX = Mth::floor(mMob->mX + 3.0);
    const int maxY = Mth::floor(mMob->mY + 6.0);
    const int maxZ = Mth::floor(mMob->mZ + 3.0);

    MutableBlockPos candidate(minX, minY, minZ);

    for (int x = minX; x <= maxX; ++x)
    {
        candidate.setX(x);
        for (int y = minY; y <= maxY; ++y)
        {
            candidate.setY(y);
            for (int z = minZ; z <= maxZ; ++z)
            {
                candidate.setZ(z);
                if (origin.equals(candidate))
                {
                    continue;
                }

                Block* block = mMob->mLevel->getBlock(*below.set(candidate)->move(Direction::DOWN));
                if ((block != NULL && dynamic_cast<LeafBlock*>(block) != NULL || block == Blocks::log
                     || block == Blocks::log2)
                    && mMob->mLevel->isEmptyBlock(candidate)
                    && mMob->mLevel->isEmptyBlock(*above.set(candidate)->move(Direction::UP)))
                {
                    return Vec3::newTemp(candidate.x, candidate.y, candidate.z);
                }
            }
        }
    }

    return NULL;
}

Vec3* WaterAvoidingRandomFlyingGoal::getPosition()
{
    Vec3* position = NULL;
    if (mMob->isInWater() || mMob->isOverWater())
    {
        position = RandomPos::getLandPos(boost::static_pointer_cast<PathfinderMob>(mMob->shared_from_this()), 15, 15,
                                         -1);
    }

    if (mMob->getRandom()->nextFloat() >= mProbability)
    {
        position = getTreePos();
    }

    if (position == NULL)
    {
        return WaterAvoidingRandomStrollGoal::getPosition();
    }

    return position;
}

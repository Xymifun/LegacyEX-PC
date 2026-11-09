#include "game/entity/mob/AgableWaterAnimal.hpp"

#include <algorithm>

#include "game/entity/mob/Entity.hpp"
#include "game/entity/ai/WaterBoundPathNavigation.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/Material.hpp"

AgableWaterAnimal::AgableWaterAnimal(Level* level) : AgableMob(level)
{
}

bool AgableWaterAnimal::isWaterMob()
{
    return true;
}

bool AgableWaterAnimal::checkSpawnRules()
{
    MutableBlockPos position(getPos(0.0f));
    while (mLevel->getLiquidBlock(position.above())->mMaterial == Material::WATER)
    {
        position = MutableBlockPos(position.above());
    }

    int y = position.y - mRandom->nextInt(1, 5);
    position.setY(std::max(static_cast<int>(getPos(0.0f)->y), y));
    setPos(position.x, position.y, position.z);

    return !mLevel->containsAnysolidBlocking(mBoundingBox) && mLevel->containsMaterial(mBoundingBox, Material::WATER);
}

bool AgableWaterAnimal::checkSpawnObstruction()
{
    return mLevel->isUnobstructed(mBoundingBox, shared_from_this());
}

int AgableWaterAnimal::getAmbientSoundInterval()
{
    return 120;
}

int AgableWaterAnimal::getExperienceReward(boost::shared_ptr<Player>)
{
    return mLevel->mRandom->nextInt(3) + 1;
}

PathNavigation* AgableWaterAnimal::createNavigation(Level* level)
{
    return new WaterBoundPathNavigation(this, level, true, false);
}

void AgableWaterAnimal::aiStep()
{
    Mob::aiStep();
    if (mOnGround && !isInWater())
    {
        mYd += 0.5;
        mXd += (double)((mRandom->nextFloat() * 2.0f - 1.0f) * 0.2f);
        mZd += (double)((mRandom->nextFloat() * 2.0f - 1.0f) * 0.2f);
        mYRot = mRandom->nextFloat() * 360.0f;
        mOnGround = false;
    }
}

bool AgableWaterAnimal::isPushedByWater()
{
    return false;
}

bool AgableWaterAnimal::breathesAir()
{
    return false;
}

#include "game/entity/mob/WaterAnimal.hpp"
#include "game/client/audio/SoundEvents.hpp"

#include <algorithm>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/ai/WaterBoundPathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"

WaterAnimal::WaterAnimal(Level* level) : PathfinderMob(level)
{
    mSwimAmplitude = 0.0f;
    mSwimFrequency = 0.0f;
}

bool WaterAnimal::isWaterMob()
{
    return true;
}

bool WaterAnimal::checkSpawnRules()
{
    BlockPos position(getPos(0.0f));
    while (mLevel->getLiquidBlock(position.above())->mMaterial == Material::WATER)
    {
        position = MutableBlockPos(position.above());
    }

    int y = position.y - mRandom->nextInt(1, 5);
    MutableBlockPos& spawnPosition = static_cast<MutableBlockPos&>(position);
    int currentY = static_cast<int>(getPos(0.0f)->y);
    spawnPosition.setY(currentY < y ? y : currentY);
    setPos(spawnPosition.x, spawnPosition.y, spawnPosition.z);

    return !mLevel->containsAnysolidBlocking(mBoundingBox) && mLevel->containsMaterial(mBoundingBox, Material::WATER);
}

bool WaterAnimal::checkSpawnObstruction()
{
    return mLevel->isUnobstructed(mBoundingBox, shared_from_this());
}

int WaterAnimal::getAmbientSoundInterval()
{
    return 120;
}

bool WaterAnimal::removeWhenFarAway()
{
    return true;
}

int WaterAnimal::getExperienceReward(boost::shared_ptr<Player>)
{
    return mLevel->mRandom->nextInt(3) + 1;
}

PathNavigation* WaterAnimal::createNavigation(Level* level)
{
    return new WaterBoundPathNavigation(this, level, true, false);
}

const SoundEvent* WaterAnimal::getFlopSound() const
{
    return SoundEvents::FISH_FLOP;
}

void WaterAnimal::aiStep()
{
    Mob::aiStep();
    if (mOnGround && !isInWater())
    {
        mYd += 0.5;
        mXd += (mRandom->nextFloat() * 2.0f - 1.0f) * 0.2f;
        mZd += (mRandom->nextFloat() * 2.0f - 1.0f) * 0.2f;
        mYRot = mRandom->nextFloat() * 360.0f;
        mLevel->playSound(boost::shared_ptr<Player>(), BlockPos(this), getFlopSound(),
                          SoundSource::eSoundSource_Neutral, 1.0f, 1.0f, 16.0f);
        mOnGround = false;
    }
}

void WaterAnimal::baseTick()
{
    int airSupply = getAirSupply();
    Mob::baseTick();
    if (isAlive() && !isInWaterOrBubble())
    {
        setAirSupply(airSupply - 1);
        if (getAirSupply() == -20)
        {
            setAirSupply(0);
            hurt(DamageSource::DROWN, 2.0f);
        }
    }
    else
    {
        setAirSupply(300);
    }
}

bool WaterAnimal::isPushedByWater()
{
    return false;
}

bool WaterAnimal::breathesAir()
{
    return false;
}

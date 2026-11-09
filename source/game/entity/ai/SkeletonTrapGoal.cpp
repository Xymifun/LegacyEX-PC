#include "game/entity/ai/SkeletonTrapGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/DifficultyInstance.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/AbstractSkeleton.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/core/LightningBolt.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SkeletonHorse.hpp"
#include "game/entity/mob/Skeleton.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"

SkeletonTrapGoal::SkeletonTrapGoal(SkeletonHorse* horse) : mHorse(horse)
{
}

bool SkeletonTrapGoal::canUse()
{
    return mHorse->mLevel->hasNearPlayer(mHorse->mX, mHorse->mY, mHorse->mZ, 10.0);
}

boost::shared_ptr<Skeleton> SkeletonTrapGoal::createSkeleton(const DifficultyInstance& difficulty,
                                                             const boost::shared_ptr<AbstractHorse>& horse)
{
    boost::shared_ptr<Skeleton> skeleton(new Skeleton(horse->mLevel));
    skeleton->finalizeMobSpawn(difficulty, NULL, std::wstring(L""));
    skeleton->setPos(horse->mX, horse->mY, horse->mZ);
    skeleton->mInvulnerableTime = 60;
    skeleton->setPersistenceRequired(true);

    if (skeleton->getItemSlot(EquipmentSlot::HEAD)->isEmpty())
        skeleton->setItemSlot(EquipmentSlot::HEAD, not_null_ptr<ItemInstance>(new ItemInstance(Items::ironHelmet)));

    skeleton->setItemSlot(
        EquipmentSlot::MAINHAND,
        EnchantmentHelper::enchantItem(
            skeleton->getRandom(), skeleton->getMainHandItem(),
            (int)(5.0F + difficulty.getSpecialMultiplier() * (float)skeleton->getRandom()->nextInt(18)), false));
    skeleton->setItemSlot(
        EquipmentSlot::HEAD,
        EnchantmentHelper::enchantItem(
            skeleton->getRandom(), skeleton->getItemSlot(EquipmentSlot::HEAD),
            (int)(5.0F + difficulty.getSpecialMultiplier() * (float)skeleton->getRandom()->nextInt(18)), false));

    skeleton->mLevel->addEntity(skeleton);
    return skeleton;
}

boost::shared_ptr<AbstractHorse> SkeletonTrapGoal::createHorse(const DifficultyInstance& difficulty)
{
    boost::shared_ptr<SkeletonHorse> horse(new SkeletonHorse(mHorse->mLevel));
    horse->finalizeMobSpawn(difficulty, NULL, std::wstring(L""));
    horse->setPos(mHorse->mX, mHorse->mY, mHorse->mZ);
    horse->mInvulnerableTime = 60;
    horse->setPersistenceRequired(true);
    horse->setTamed(true);
    horse->setAge(0);
    horse->mLevel->addEntity(horse);
    return horse;
}

void SkeletonTrapGoal::tick()
{
    DifficultyInstance difficulty(mHorse->mLevel->getCurrentDifficultyAt(BlockPos(mHorse)));

    mHorse->setTrap(false);
    mHorse->setTamed(true);
    mHorse->setAge(0);

    mHorse->mLevel->addGlobalEntity(
        boost::shared_ptr<LightningBolt>(new LightningBolt(mHorse->mLevel, mHorse->mX, mHorse->mY, mHorse->mZ, true)));

    boost::shared_ptr<Skeleton> skeleton = createSkeleton(
        difficulty, boost::static_pointer_cast<AbstractHorse>(mHorse->shared_from_this()));
    skeleton->startRiding(mHorse->shared_from_this());

    for (int i = 0; i < 3; ++i)
    {
        boost::shared_ptr<AbstractHorse> trapHorse = createHorse(difficulty);
        boost::shared_ptr<Skeleton> trapSkeleton = createSkeleton(difficulty, trapHorse);
        trapSkeleton->startRiding(trapHorse);
        trapHorse->push(mHorse->getRandom()->nextGaussian() * 0.5, 0.0, mHorse->getRandom()->nextGaussian() * 0.5);
    }
}

#include "game/entity/ai/BreedGoal.hpp"

#include <boost/shared_ptr.hpp>
#include <cfloat>
#include <string>
#include <typeinfo>
#include <vector>

#include "game/core/Path.hpp"
#include "game/core/ExperienceOrb.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/Sensing.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"

BreedGoal::BreedGoal(Animal* animal, double speedModifier) : mPartner()
{
    mPartnerType = &typeid(*animal);
    mTicks = 0;
    mSpeedModifier = speedModifier;
    mAnimal = animal;
    mLevel = animal->mLevel;
    setRequiredControlFlags(3);
}

BreedGoal::BreedGoal(Animal* animal, double speedModifier, const std::type_info& partnerType) : mPartner()
{
    mPartnerType = &partnerType;
    mTicks = 0;
    mSpeedModifier = speedModifier;
    mAnimal = animal;
    mLevel = animal->mLevel;
    setRequiredControlFlags(3);
}

boost::shared_ptr<Animal> BreedGoal::getFreePartner()
{
    AABB* area = mAnimal->mBoundingBox->grow(8.0);
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(*mPartnerType, area);
    double nearest = DBL_MAX;
    boost::shared_ptr<Animal> partner;
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Animal> candidate = boost::dynamic_pointer_cast<Animal>(*it);
        if (mAnimal->canMate(candidate) && mAnimal->distanceToSqr(candidate) < nearest)
        {
            partner = candidate;
            nearest = mAnimal->distanceToSqr(candidate);
        }
    }
    delete entities;
    return partner;
}

bool BreedGoal::canUse()
{
    if (!mAnimal->isInLove())
        return false;

    mPartner = getFreePartner();
    return !mPartner.expired();
}

bool BreedGoal::canContinueToUse()
{
    boost::shared_ptr<Animal> partner = mPartner.lock();
    return partner && partner->isAlive() && partner->isInLove() && mTicks < 60;
}

void BreedGoal::stop()
{
    mPartner.reset();
    mTicks = 0;
}

void BreedGoal::tick()
{
    ++mTicks;
    boost::shared_ptr<Animal> partner = mPartner.lock();
    mAnimal->getLookControl()->setLookAt(partner, 10.0f, static_cast<float>(mAnimal->getMaxHeadXRot()));
    mAnimal->getNavigation()->moveTo(partner, mSpeedModifier);
    if (mTicks >= 60)
    {
        BlockPos pos(partner);
        Path* path = mAnimal->getNavigation()->getPath();
        bool atEnd = false;
        if (path != NULL)
        {
            atEnd = path->endsIn(Vec3::newTemp(&pos));
            atEnd = atEnd && path->IsWithinStepsToTheEnd(1);
        }
        if (atEnd)
        {
            bool seen = mAnimal->getSensing()->canSee(partner);
            if (seen && mAnimal->distanceToSqr(partner) < 9.0)
                breed();
        }
    }
}

void BreedGoal::breed()
{
    boost::shared_ptr<Animal> partner = mPartner.lock();
    mAnimal->setDespawnProtected();
    partner->setDespawnProtected();
    if (!mLevel->canCreateMore(mAnimal->GetType(), Level::eSpawnType_Breed))
    {
        mAnimal->resetLove();
        partner->resetLove();
        return;
    }

    boost::shared_ptr<AgableMob> child = mAnimal->getBreedOffspring(partner);

    boost::shared_ptr<Player> player = mAnimal->getLoveCause();
    if (!player && partner->getLoveCause())
        player = partner->getLoveCause();
    if (player)
    {
        player->awardStat(GenericStats::breedEntity(child->GetType()),
                          GenericStats::param_breedEntity(child->GetType()));
    }

    mAnimal->setAge(6000);
    partner->setAge(6000);
    mAnimal->resetLove();
    partner->resetLove();
    child->setAge(-24000);
    child->moveTo(mAnimal->mX, mAnimal->mY, mAnimal->mZ, 0.0f, 0.0f);
    child->setDespawnProtected();
    mLevel->addEntity(child);

    Random* random = mAnimal->getRandom();
    for (int i = 0; i < 7; ++i)
    {
        double xa = random->nextGaussian() * 0.02;
        double ya = random->nextGaussian() * 0.02;
        double za = random->nextGaussian() * 0.02;
        double xOffset = random->nextDouble() * mAnimal->mBbWidth * 2.0 - mAnimal->mBbWidth;
        double yOffset = random->nextDouble() * mAnimal->mBbHeight + 0.5;
        double zOffset = random->nextDouble() * mAnimal->mBbWidth * 2.0 - mAnimal->mBbWidth;
        arrayWithLength<int> particleData;
        mLevel->addParticle(PARTICLE_HEART, mAnimal->mX + xOffset, mAnimal->mY + yOffset, mAnimal->mZ + zOffset, xa, ya,
                            za, particleData);
    }

    if (mLevel->getGameRules()->getBoolean(GameRules::RULE_DOMOBLOOT))
    {
        boost::shared_ptr<ExperienceOrb> orb(
            new ExperienceOrb(mLevel, mAnimal->mX, mAnimal->mY, mAnimal->mZ, random->nextInt(7) + 1));
        mLevel->addEntity(orb);
    }
}

void BreedGoal::setLevel(Level* level)
{
    mLevel = level;
}

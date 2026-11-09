#include "game/entity/projectile/DragonFireball.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/client/render/ParticleType.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/HitResult.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/AreaEffectCloud.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/data/DataFixerUpper.hpp"

const double DragonFireball::SPLASH_RANGE = 4.0;

DragonFireball::DragonFireball(Level* level) : Fireball(level)
{
    fjDerivedCtorCalls();
    setSize(1.0f, 1.0f);
}

DragonFireball::DragonFireball(Level* level, boost::shared_ptr<LivingEntity> owner, double xd, double yd, double zd)
    : Fireball(level, owner, xd, yd, zd)
{
    fjDerivedCtorCalls();
    setSize(1.0f, 1.0f);
}

DragonFireball::DragonFireball(Level* level, double x, double y, double z, double xd, double yd, double zd)
    : Fireball(level, x, y, z, xd, yd, zd)
{
    fjDerivedCtorCalls();
    setSize(1.0f, 1.0f);
}

void DragonFireball::addDataWalkers(DataFixerUpper* fixer)
{
    Fireball::addDataWalkers(fixer, EntityType::DRAGON_FIREBALL);
}

void DragonFireball::onHit(const HitResult* result)
{
    if ((result->mEntity == NULL || result->mEntity != mOwner) && !mLevel->mIsClientSide)
    {
        std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(
            typeid(LivingEntity), mBoundingBox->grow(4.0, 2.0, 4.0));

        boost::shared_ptr<AreaEffectCloud> cloud(new AreaEffectCloud(mLevel, mX, mY, mZ));
        cloud->setOwner(mOwner);
        cloud->setParticle(PARTICLE_DRAGON_BREATH);
        cloud->setRadius(3.0f);
        cloud->setParticleSpawnRateOnTick(10);
        cloud->setDuration(200);
        cloud->setRadiusPerTick((7.0f - cloud->getRadius()) / static_cast<float>(cloud->getDuration()));
        cloud->addEffect(new MobEffectInstance(MobEffects::HARM, 1, 1));

        if (entities != NULL && !entities->empty())
        {
            for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
            {
                boost::shared_ptr<Entity> entity = *it;
                double distance = distanceToSqr(entity);
                if (distance < 16.0)
                {
                    cloud->setPos(entity->mX, entity->mY, entity->mZ);
                    break;
                }
            }
        }

        mLevel->levelEvent(LevelEvent::PARTICLES_DRAGON_FIREBALL_SPLASH, BlockPos(mX, mY, mZ), 0);
        mLevel->addEntity(cloud);
        delete entities;
        remove();
    }
}

bool DragonFireball::isPickable()
{
    return false;
}

bool DragonFireball::hurt(const DamageSource*, float)
{
    return false;
}

const ParticleType* DragonFireball::getTrailParticleType()
{
    return PARTICLE_DRAGON_BREATH;
}

bool DragonFireball::shouldBurn()
{
    return false;
}

unsigned long long DragonFireball::GetType()
{
    return EntityType::DRAGON_FIREBALL;
}

Entity* DragonFireball::create(Level* level)
{
    return new DragonFireball(level);
}

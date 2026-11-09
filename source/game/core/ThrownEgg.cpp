#include "game/core/ThrownEgg.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/core/Chicken.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/HitResult.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/arrayWithLength.hpp"

ThrownEgg::ThrownEgg(Level* level) : ThrowableProjectile(level)
{
    _init();
}

ThrownEgg::ThrownEgg(Level* level, boost::shared_ptr<LivingEntity> owner) : ThrowableProjectile(level, owner)
{
    _init();
}

ThrownEgg::ThrownEgg(Level* level, double x, double y, double z) : ThrowableProjectile(level, x, y, z)
{
    _init();
}

Entity* ThrownEgg::create(Level* level)
{
    return new ThrownEgg(level);
}

bool ThrownEgg::onHit(const HitResult* result)
{
    Projectile::onHit(result);

    if (result->mEntity)
    {
        DamageSource* damageSource = DamageSource::CreateThrown(shared_from_this(), Projectile::mOwner);
        result->mEntity->hurt(damageSource, 0.0f);
        delete damageSource;
    }

    if (!mLevel->mIsClientSide)
    {
        if (mRandom->nextInt(8) == 0 && mLevel->canCreateMore(EntityType::CHICKEN, Level::eSpawnType_Breed))
        {
            int count = 1;
            if (mRandom->nextInt(32) == 0)
                count = 4;

            if (count == 0)
                count = 1;

            for (int i = 0; i < count; ++i)
            {
                boost::shared_ptr<Chicken> chicken(new Chicken(mLevel));
                chicken->setAge(-24000);
                chicken->moveTo(mX, mY, mZ, mYRot, 0.0f);
                chicken->setDespawnProtected();
                mLevel->addEntity(chicken);
            }
        }

        mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::DEATH, 0);
        remove();
    }
    else
    {
        fjSetParticleSpawnPos(mX, mY, mZ);
    }

    return true;
}

void ThrownEgg::addDataWalkers(DataFixerUpper*)
{
}

void ThrownEgg::_init()
{
    fjDerivedCtorCalls();
}

void ThrownEgg::handleEntityEvent(unsigned char event, int data)
{
    if (event == EntityEvent::DEATH)
    {
        Vec3* position = fjGetParticleSpawnPos();
        for (int i = 0; i < 8; ++i)
        {
            int itemId[] = {Item::getId(Items::egg)};
            arrayWithLength<int> particleData(itemId, 1);
            mLevel->addParticle(PARTICLE_ICON_CRACK, position->x, position->y, position->z,
                                (mRandom->nextFloat() - 0.5f) * 0.08, (mRandom->nextFloat() - 0.5f) * 0.08,
                                (mRandom->nextFloat() - 0.5f) * 0.08, particleData);
        }
    }
    else
    {
        Entity::handleEntityEvent(event, data);
    }
}

unsigned long long ThrownEgg::GetType()
{
    return EntityType::THROWN_EGG;
}

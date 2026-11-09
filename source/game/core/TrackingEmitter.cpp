#include "game/core/TrackingEmitter.hpp"
#include "game/core/EntityType.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/arrayWithLength.hpp"

TrackingEmitter::TrackingEmitter(Level* level, boost::shared_ptr<Entity> entity, const ParticleType* particleType,
                                 int lifetime)
    : Particle(level, entity->mX, entity->mBoundingBox->mMinY + entity->mBbHeight * 0.5f, entity->mZ, entity->mXd,
               entity->mYd, entity->mZd)
{
    mEntity = entity;
    this->lifetime = lifetime;
    mParticleType = particleType;
    tick();
}

void TrackingEmitter::tick()
{
    for (int i = 0; i < 16; ++i)
    {
        double xa = (double)random.nextFloat() * 2.0 - 1.0;
        double ya = (double)random.nextFloat() * 2.0 - 1.0;
        double za = (double)random.nextFloat() * 2.0 - 1.0;

        if (xa * xa + ya * ya + za * za > 1.0)
        {
            continue;
        }

        double x = mEntity->mX + xa * (double)mEntity->mBbWidth / 4.0;
        double y = mEntity->mBoundingBox->mMinY + (double)mEntity->mBbHeight * 0.5
                   + ya * (double)mEntity->mBbHeight / 4.0;
        double z = mEntity->mZ + za * (double)mEntity->mBbWidth / 4.0;
        level->addParticle(mParticleType, false, x, y, z, xa, ya + 0.2, za, arrayWithLength<int>());
    }

    if (++age >= lifetime)
    {
        remove();
    }
}

void TrackingEmitter::render(BufferBuilder*, const boost::shared_ptr<Entity>&, float, float, float, float, float, float)
{
}

int TrackingEmitter::getParticleTexture()
{
    return 3;
}

uint64_t TrackingEmitter::GetType()
{
    return ParticleKind::TRACKING_EMITTER;
}

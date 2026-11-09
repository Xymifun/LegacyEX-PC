#include "game/client/render/HugeExplosionSeedParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"

HugeExplosionSeedParticle::HugeExplosionSeedParticle(Level* level, double x, double y, double z, double xa, double ya,
                                                     double za)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    lifetime = 8;
}

void HugeExplosionSeedParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera,
                                       float partialTicks, float xa, float ya, float za, float xa2, float za2)
{
}

void HugeExplosionSeedParticle::tick()
{
    Minecraft::GetInstance()->mAnimateTickLevel = level;

    for (int i = 0; i < 6; ++i)
    {
        double xx = x + (random.nextDouble() - random.nextDouble()) * 4.0;
        double yy = y + (random.nextDouble() - random.nextDouble()) * 4.0;
        double zz = z + (random.nextDouble() - random.nextDouble()) * 4.0;
        arrayWithLength<int> params;
        level->addParticle(PARTICLE_LARGE_EXPLODE, xx, yy, zz, (float)age / (float)lifetime, 0.0, 0.0, params);
    }

    Minecraft::GetInstance()->mAnimateTickLevel = NULL;
    ++age;
    if (age == lifetime)
    {
        remove();
    }
}

int HugeExplosionSeedParticle::getParticleTexture()
{
    return 1;
}

uint64_t HugeExplosionSeedParticle::GetType()
{
    return ParticleKind::HUGE_EXPLOSION_SEED;
}

Particle* HugeExplosionSeedParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y,
                                                              double z, double xa, double ya, double za,
                                                              arrayWithLength<int> params)
{
    return new HugeExplosionSeedParticle(level, x, y, z, xa, ya, za);
}

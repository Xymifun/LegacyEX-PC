#include "game/client/render/SpitParticle.hpp"
#include "game/core/EntityType.hpp"

SpitParticle::SpitParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : ExplodeParticle(level, x, y, z, xa, ya, za)
{
    gravity = 0.5f;
}

uint64_t SpitParticle::GetType()
{
    return ParticleKind::SPIT;
}

void SpitParticle::tick()
{
    ExplodeParticle::tick();
    yd -= 0.04 * (double)gravity + 0.004;
}

Particle* SpitParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                 double xa, double ya, double za, arrayWithLength<int> params)
{
    return new SpitParticle(level, x, y, z, xa, ya, za);
}

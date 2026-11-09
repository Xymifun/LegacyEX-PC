#include "game/client/render/TotemParticle.hpp"
#include "game/core/EntityType.hpp"

TotemParticle::TotemParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : SimpleAnimatedParticle(level, x, y, z, 176, 8, -0.05f)
{
    xd = xa;
    yd = ya;
    zd = za;
    size *= 0.75f;
    lifetime = random.nextInt(12) + 60;

    if (random.nextInt(4) == 0)
    {
        Particle::setColor(random.nextFloat() * 0.2f + 0.6f, random.nextFloat() * 0.3f + 0.6f,
                           random.nextFloat() * 0.2f);
        setBaseAirFriction(0.6f);
    }
    else
    {
        Particle::setColor(random.nextFloat() * 0.2f + 0.1f, random.nextFloat() * 0.3f + 0.4f,
                           random.nextFloat() * 0.2f);
        setBaseAirFriction(0.6f);
    }
}

uint64_t TotemParticle::GetType()
{
    return ParticleKind::TOTEM;
}

Particle* TotemParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                  double xa, double ya, double za, arrayWithLength<int> params)
{
    return new TotemParticle(level, x, y, z, xa, ya, za);
}

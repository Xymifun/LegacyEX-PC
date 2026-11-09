#include "game/client/render/SuspendedTownParticle.hpp"
#include "game/core/EntityType.hpp"

SuspendedTownParticle::SuspendedTownParticle(Level* level, double x, double y, double z, double xa, double ya,
                                             double za)
    : Particle(level, x, y, z, xa, ya, za)
{
    float brightness = random.nextFloat() * 0.1f + 0.2f;
    rCol = brightness;
    gCol = brightness;
    bCol = brightness;
    setMiscTex(0);
    setSize(0.02f, 0.02f);
    size *= random.nextFloat() * 0.6f + 0.5f;
    xd *= 0.02;
    yd *= 0.02;
    zd *= 0.02;
    lifetime = (int)(20.0 / (random.nextDouble() * 0.8 + 0.2));
}

void SuspendedTownParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
    move(xd, yd, zd);
    xd *= 0.99;
    yd *= 0.99;
    zd *= 0.99;
    if (lifetime-- <= 0)
        remove();
}

void SuspendedTownParticle::move(double xa, double ya, double za)
{
    setBoundingBox(getBoundingBox()->move(xa, ya, za));
    setLocationFromBoundingbox();
}

uint64_t SuspendedTownParticle::GetType()
{
    return ParticleKind::SUSPENDED_TOWN;
}

Particle* SuspendedTownParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y,
                                                          double z, double xa, double ya, double za,
                                                          arrayWithLength<int> params)
{
    return new SuspendedTownParticle(level, x, y, z, xa, ya, za);
}

Particle* SuspendedTownParticle::HappyVillagerProvider::createParticle(ePARTICLE_TYPE type, Level* level, double x,
                                                                       double y, double z, double xa, double ya,
                                                                       double za, arrayWithLength<int> params)
{
    SuspendedTownParticle* particle = new SuspendedTownParticle(level, x, y, z, xa, ya, za);
    particle->setMiscTex(0x52);
    particle->setColor(1.0f, 1.0f, 1.0f);
    return particle;
}

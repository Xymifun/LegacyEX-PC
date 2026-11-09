#include "game/client/render/ConduitParticle.hpp"
#include "game/world/Level.hpp"

ConduitParticle::ConduitParticle(Level* level, double x, double y, double z, double xa, double ya, double za,
                                 arrayWithLength<int> params)
    : Particle(level, x, y, z, xa, ya, za),
      m_startX(x),
      m_startY(y),
      m_startZ(z)
{
    xd = xa;
    yd = ya;
    zd = za;
    xo = x;
    yo = y;
    zo = z;
    setPos(x + xa, y + ya, z + za);

    Random* levelRandom = level->mRandom;
    float colour = levelRandom->nextFloat(0.4f, 1.0f);
    m_conduitParticleValue = size = levelRandom->nextFloat(0.2f, 0.7f);
    rCol = colour * 0.9f;
    gCol = colour * 0.9f;
    bCol = colour;
    float lifetimeScale = levelRandom->nextFloat(0.5f, 2.5f);
    float lifetimeValue = lifetimeScale * 20.0f;
    hasPhysics = false;
    lifetime = (int)lifetimeValue;

    uvs = TextureUVCoordinateSet::fromOldSystem(levelRandom->nextInt(4) * 3 + 0xd0 + levelRandom->nextInt(0, 3));
}

void ConduitParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    double progress = (double)age / (double)lifetime;
    progress = 1.0 - progress;
    double fall = 1.0 - progress;
    fall = fall * fall * fall;
    setPos(m_startX + xd * progress, m_startY + yd * progress - fall * 1.2, m_startZ + zd * progress);

    if (age++ >= lifetime)
        remove();
}

uint64_t ConduitParticle::GetType()
{
    return PARTICLE_TYPE_CONDUIT;
}

Particle* ConduitParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                    double xa, double ya, double za, arrayWithLength<int> params)
{
    return new ConduitParticle(level, x, y, z, xa, ya, za, params);
}

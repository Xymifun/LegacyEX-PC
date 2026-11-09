#include "game/client/render/LavaParticle.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/core/BufferBuilder.hpp"
#include "game/world/Level.hpp"

LavaParticle::LavaParticle(Level* level, double x, double y, double z) : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    xd *= 0.8;
    yd *= 0.8;
    zd *= 0.8;

    yd = random.nextDouble() * 0.4 + 0.05;
    rCol = 1.0f;
    gCol = 1.0f;
    bCol = 1.0f;
    mOriginalSize = size *= random.nextFloat() * 2.0f + 0.2f;
    lifetime = (int)std::floor(16.0 / (random.nextDouble() * 0.8 + 0.2));
    setMiscTex(0x31);
}

int LavaParticle::getLightColor(float partialTicks)
{
    return Particle::getLightColor(partialTicks) & 0xff0000 | 0xf0;
}

float LavaParticle::getBrightness(float partialTicks)
{
    return 1.0f;
}

void LavaParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                          float ya, float za, float xa2, float za2)
{
    float progress = ((float)age + partialTicks) / (float)lifetime;
    size = mOriginalSize * (1.0f - progress * progress);
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void LavaParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime)
        remove();

    float progress = (float)age / (float)lifetime;
    if (random.nextFloat() > progress)
        level->addParticle(PARTICLE_SMOKE, x, y, z, xd, yd, zd, arrayWithLength<int>());

    yd -= 0.03;
    move(xd, yd, zd);
    xd *= 0.999f;
    yd *= 0.999f;
    zd *= 0.999f;

    if (onGround)
    {
        xd *= 0.7f;
        zd *= 0.7f;
    }
}

uint64_t LavaParticle::GetType()
{
    return ParticleKind::LAVA;
}

Particle* LavaParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                 double xa, double ya, double za, arrayWithLength<int> params)
{
    return new LavaParticle(level, x, y, z);
}

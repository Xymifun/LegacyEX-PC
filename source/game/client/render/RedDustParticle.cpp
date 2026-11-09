#include "game/client/render/RedDustParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Mth.hpp"

RedDustParticle::RedDustParticle(Level* level, double x, double y, double z, float red, float green, float blue)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    init(level, x, y, z, 1.0f, red, green, blue);
}

void RedDustParticle::init(Level* level, double x, double y, double z, float alpha, float red, float green, float blue)
{
    xd *= 0.1;
    yd *= 0.1;
    zd *= 0.1;

    if (red == 0.0f && green == 0.0f && blue == 0.0f)
        red = 1.0f;

    float scale = random.nextFloat() * 0.4f + 0.6f;
    rCol = (random.nextFloat() * 0.2f + 0.8f) * red * scale;
    gCol = (random.nextFloat() * 0.2f + 0.8f) * green * scale;
    bCol = (random.nextFloat() * 0.2f + 0.8f) * blue * scale;
    const float particleSize = Particle::size * 0.75f * alpha;
    Particle::size = particleSize;
    size = particleSize;
    lifetime = (int)(8.0 / (random.nextDouble() * 0.8 + 0.2));
    lifetime = (int)((float)lifetime * alpha);
    setMiscTex(7 - (age << 3) / lifetime);
}

void RedDustParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks,
                             float xa, float ya, float za, float xa2, float za2)
{
    float progress = ((float)age + partialTicks) / (float)lifetime * 32.0f;
    progress = Mth::clamp(progress, 0.0f, 1.0f);
    Particle::size = size * progress;
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void RedDustParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime)
        remove();

    setMiscTex(7 - (age << 3) / lifetime);
    move(xd, yd, zd);

    if (y == yo)
    {
        xd *= 1.1;
        zd *= 1.1;
    }

    xd *= 0.96;
    yd *= 0.96;
    zd *= 0.96;

    if (onGround)
    {
        xd *= 0.7f;
        zd *= 0.7f;
    }
}

uint64_t RedDustParticle::GetType()
{
    return ParticleKind::RED_DUST;
}

Particle* RedDustParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                    double xa, double ya, double za, arrayWithLength<int> params)
{
    return new RedDustParticle(level, x, y, z, (float)xa, (float)ya, (float)za);
}

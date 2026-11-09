#include "game/client/render/SmokeParticle.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"

SmokeParticle::SmokeParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    init(level, x, y, z, xa, ya, za, 1.0f);
}

SmokeParticle::SmokeParticle(Level* level, double x, double y, double z, double xa, double ya, double za, float scale)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    init(level, x, y, z, xa, ya, za, scale);
}

void SmokeParticle::init(Level* level, double x, double y, double z, double xa, double ya, double za, float scale)
{
    xd = xd * 0.1 + xa;
    yd = yd * 0.1 + ya;
    zd = zd * 0.1 + za;

    const float randomScale = random.nextFloat();
    const unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Particle_Smoke);
    const float red = ((float)(int)((colour >> 16) & 255)) / 255.0f;
    const float green = ((float)(int)((colour >> 8) & 255)) / 255.0f;
    const float blue = ((float)(int)(colour & 255)) / 255.0f;
    rCol = red * randomScale;
    gCol = green * randomScale;
    bCol = blue * randomScale;

    Particle::size = Particle::size * 0.75f * scale;
    size = Particle::size;
    lifetime = (int)((float)(int)floor(8.0 / (random.nextDouble() * 0.8 + 0.2)) * scale);
    setMiscTex(7);
}

void SmokeParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                           float ya, float za, float xa2, float za2)
{
    float progress = ((float)age + partialTicks) / (float)lifetime * 32.0f;
    progress = Mth::clamp(progress, 0.0f, 1.0f);
    Particle::size = size * progress;
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void SmokeParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
    if (age++ >= lifetime)
    {
        remove();
    }

    setMiscTex(7 - (age << 3) / lifetime);
    yd += 0.004;
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
        xd *= 0.7;
        zd *= 0.7;
    }
}

uint64_t SmokeParticle::GetType()
{
    return ParticleKind::SMOKE;
}

Particle* SmokeParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                  double xa, double ya, double za, arrayWithLength<int> params)
{
    return new SmokeParticle(level, x, y, z, xa, ya, za);
}

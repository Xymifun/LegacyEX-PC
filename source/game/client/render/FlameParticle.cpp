#include "game/client/render/FlameParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/BufferBuilder.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Math.hpp"

FlameParticle::FlameParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : Particle(level, x, y, z, xa, ya, za)
{
    xd = xd * 0.01f + xa;
    yd = yd * 0.01f + ya;
    zd = zd * 0.01f + za;
    this->x += (random.nextFloat() - random.nextFloat()) * 0.05f;
    this->y += (random.nextFloat() - random.nextFloat()) * 0.05f;
    this->z += (random.nextFloat() - random.nextFloat()) * 0.05f;
    mOriginalSize = size;
    rCol = 1.0f;
    gCol = 1.0f;
    bCol = 1.0f;
    lifetime = (int)(8.0 / (Math::random() * 0.8 + 0.2)) + 4;
    setMiscTex(0x30);
}

void FlameParticle::move(double xa, double ya, double za)
{
    setBoundingBox(getBoundingBox()->move(xa, ya, za));
    setLocationFromBoundingbox();
}

void FlameParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                           float ya, float za, float xa2, float za2)
{
    float progress = ((float)age + partialTicks) / (float)lifetime;
    size = mOriginalSize * (1.0f - progress * progress * 0.5f);
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

int FlameParticle::getLightColor(float partialTicks)
{
    float progress = ((float)age + partialTicks) / (float)lifetime;
    progress = Mth::clamp(progress, 0.0f, 1.0f);
    int light = Particle::getLightColor(partialTicks);
    int block = light & 255;
    int sky = (light >> 16) & 255;
    block += (int)(progress * 15.0f * 16.0f);
    if (block > 240)
        block = 240;
    return block | (sky << 16);
}

float FlameParticle::getBrightness(float partialTicks)
{
    float progress = Mth::clamp(((float)age + partialTicks) / (float)lifetime, 0.0f, 1.0f);
    return Particle::getBrightness(partialTicks) * progress + (1.0f - progress);
}

void FlameParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
    if (age++ >= lifetime)
        remove();
    move(xd, yd, zd);
    xd *= 0.96f;
    yd *= 0.96f;
    zd *= 0.96f;
    if (!onGround)
        return;
    xd *= 0.7f;
    zd *= 0.7f;
}

uint64_t FlameParticle::GetType()
{
    return ParticleKind::FLAME;
}

Particle* FlameParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                  double xa, double ya, double za, arrayWithLength<int> params)
{
    return new FlameParticle(level, x, y, z, xa, ya, za);
}

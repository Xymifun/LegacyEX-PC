#include "game/client/render/HeartParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Mth.hpp"

HeartParticle::HeartParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    init(level, x, y, z, xa, ya, za, 2.0f);
}

void HeartParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                           float ya, float za, float xa2, float za2)
{
    float progress = ((age + partialTicks) / lifetime) * 32.0f;
    progress = Mth::clamp(progress, 0.0f, 1.0f);
    size = scale * progress;
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void HeartParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime)
    {
        remove();
    }

    move(xd, yd, zd);
    if (y == yo)
    {
        xd *= 1.1;
        zd *= 1.1;
    }

    xd *= 0.86f;
    yd *= 0.86f;
    zd *= 0.86f;

    if (onGround)
    {
        xd *= 0.7f;
        zd *= 0.7f;
    }
}

void HeartParticle::init(Level* level, double x, double y, double z, double xa, double ya, double za, float scale)
{
    xd *= 0.01f;
    yd *= 0.01f;
    zd *= 0.01f;
    yd += 0.1;
    size *= 0.75f;
    size *= scale;
    this->scale = size;
    lifetime = 16;
    setMiscTex(80);
}

uint64_t HeartParticle::GetType()
{
    return ParticleKind::HEART;
}

Particle* HeartParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                  double xa, double ya, double za, arrayWithLength<int> params)
{
    return new HeartParticle(level, x, y, z, xa, ya, za);
}

Particle* HeartParticle::AngryVillagerProvider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y,
                                                               double z, double xa, double ya, double za,
                                                               arrayWithLength<int> params)
{
    HeartParticle* particle = new HeartParticle(level, x, y + 0.5, z, xa, ya, za);
    particle->setMiscTex(81);
    particle->setColor(1.0f, 1.0f, 1.0f);
    return particle;
}

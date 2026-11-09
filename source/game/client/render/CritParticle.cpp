#include "game/client/render/CritParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Mth.hpp"

CritParticle::CritParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : Particle(level, x, y, z, xa, ya, za)
{
    _init(level, x, y, z, xa, ya, za, 1.0f);
}

void CritParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                          float ya, float za, float xa2, float za2)
{
    float fade = ((float)age + partialTicks) / (float)lifetime * 32.0f;
    fade = Mth::clamp(fade, 0.0f, 1.0f);
    size = sizeMultiplier * fade;
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void CritParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime)
        remove();

    move(xd, yd, zd);

    gCol *= 0.96f;
    bCol *= 0.9f;
    xd *= 0.7f;
    yd *= 0.7f;
    zd *= 0.7f;
    yd -= 0.02f;

    if (onGround)
    {
        xd *= 0.7f;
        zd *= 0.7f;
    }
}

void CritParticle::_init(Level* level, double x, double y, double z, double xa, double ya, double za, float scale)
{
    xd *= 0.1f;
    yd *= 0.1f;
    zd *= 0.1f;
    xd += xa * 0.4;
    yd += ya * 0.4;
    zd += za * 0.4;

    float color = (float)(random.nextDouble() * 0.3f + 0.6f);
    rCol = color;
    gCol = color;
    bCol = color;

    size = size * 0.75f * scale;
    sizeMultiplier = size;
    lifetime = (int)((int)(6.0 / (random.nextDouble() * 0.8 + 0.6)) * scale);
    hasPhysics = true;
    setMiscTex(65);
}

uint64_t CritParticle::GetType()
{
    return ParticleKind::CRIT;
}

Particle* CritParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                 double xa, double ya, double za, arrayWithLength<int> params)
{
    CritParticle* particle = new CritParticle(level, x, y, z, xa, ya, za);
    particle->tick();
    return particle;
}

Particle* CritParticle::MagicProvider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                      double xa, double ya, double za, arrayWithLength<int> params)
{
    CritParticle* particle = new CritParticle(level, x, y, z, xa, ya, za);
    particle->tick();
    particle->setColor(particle->getRedCol() * 0.3f, particle->getGreenCol() * 0.8f, particle->getBlueCol());
    particle->setMiscTex(66);
    return particle;
}

Particle* CritParticle::DamageIndicatorProvider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y,
                                                                double z, double xa, double ya, double za,
                                                                arrayWithLength<int> params)
{
    CritParticle* particle = new CritParticle(level, x, y, z, xa, ya, za);
    particle->tick();
    particle->setLifetime(20);
    particle->setMiscTex(67);
    return particle;
}

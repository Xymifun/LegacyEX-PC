#include "game/client/render/SpellParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/util/Math.hpp"
#include "game/world/Level.hpp"

SpellParticle::SpellParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : Particle(level, x, y, z, (0.5 - Math::random()) * 128.0, Math::random() * 255.0, (0.5 - Math::random()) * 128.0)
{
    yd *= static_cast<double>(0.2f);
    if (xa == 0.0 && za == 0.0)
    {
        xd *= static_cast<double>(0.1f);
        zd *= static_cast<double>(0.1f);
    }
    size *= 0.75f;
    lifetime = (int)(8.0 / (Math::random() * 0.8 + 0.2));
    baseTex = 0x80;
    setMiscTex(0x87 - (age << 3) / lifetime);
}

bool SpellParticle::isTransparent()
{
    return true;
}

void SpellParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
    if (age++ >= lifetime)
        remove();

    setMiscTex(baseTex - (age << 3) / lifetime + 7);
    yd += 0.004;
    move(xd, yd, zd);
    if (y == yo)
    {
        xd *= 1.1;
        zd *= 1.1;
    }
    xd *= 0.9599999785423279;
    yd *= 0.9599999785423279;
    zd *= 0.9599999785423279;
    if (onGround)
    {
        xd *= 0.699999988079071;
        zd *= 0.699999988079071;
    }
}

void SpellParticle::setBaseTex(int texture)
{
    baseTex = texture;
}

uint64_t SpellParticle::GetType()
{
    return ParticleKind::SPELL;
}

Particle* SpellParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                  double xa, double ya, double za, arrayWithLength<int> params)
{
    return new SpellParticle(level, x, y, z, xa, ya, za);
}

Particle* SpellParticle::MobProvider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                     double xa, double ya, double za, arrayWithLength<int> params)
{
    SpellParticle* particle = new SpellParticle(level, x, y, z, xa, ya, za);
    particle->setColor((float)xa, (float)ya, (float)za);
    return particle;
}

Particle* SpellParticle::AmbientMobProvider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y,
                                                            double z, double xa, double ya, double za,
                                                            arrayWithLength<int> params)
{
    SpellParticle* particle = new SpellParticle(level, x, y, z, xa, ya, za);
    particle->setAlpha(0.15f);
    particle->setColor((float)xa, (float)ya, (float)za);
    return particle;
}

Particle* SpellParticle::InstantProvider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y,
                                                         double z, double xa, double ya, double za,
                                                         arrayWithLength<int> params)
{
    SpellParticle* particle = new SpellParticle(level, x, y, z, xa, ya, za);
    particle->setBaseTex(0x90);
    return particle;
}

Particle* SpellParticle::WinningInstantProvider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y,
                                                                double z, double xa, double ya, double za,
                                                                arrayWithLength<int> params)
{
    SpellParticle* particle = new SpellParticle(level, x, y, z, xa, ya, za);
    particle->setBaseTex(0x90);
    if (params.length == 4)
    {
        particle->setColor((float)params.data[0] / 255.0f, (float)params.data[1] / 255.0f,
                           (float)params.data[2] / 255.0f);
        particle->setAlpha((float)params.data[3] / 255.0f);
        delete[] params.data;
    }
    particle->xd = xa;
    particle->zd = za;
    return particle;
}

Particle* SpellParticle::WitchProvider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                       double xa, double ya, double za, arrayWithLength<int> params)
{
    SpellParticle* particle = new SpellParticle(level, x, y, z, xa, ya, za);
    particle->setBaseTex(0x90);
    float color = level->mRandom->nextFloat() * 0.5f + 0.35f;
    particle->setColor(color, color * 0.0f, color);
    return particle;
}

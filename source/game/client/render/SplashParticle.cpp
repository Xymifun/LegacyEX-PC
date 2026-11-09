#include "game/client/render/SplashParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Mth.hpp"

SplashParticle::SplashParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : WaterDropParticle(level, x, y, z)
{
    gravity = 0.04f;
    setMiscTex(16);

    if (Mth::almostEquals(ya, 0.0, 2.2204460492503131e-16)
        && (!Mth::almostEquals(xa, 0.0, 2.2204460492503131e-16) || !Mth::almostEquals(za, 0.0, 2.2204460492503131e-16)))
    {
        zd = za;
        xd = xa;
        yd = ya + 0.1;
    }
}

uint64_t SplashParticle::GetType()
{
    return ParticleKind::SPLASH;
}

Particle* SplashParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                   double xa, double ya, double za, arrayWithLength<int> params)
{
    return new SplashParticle(level, x, y, z, xa, ya, za);
}

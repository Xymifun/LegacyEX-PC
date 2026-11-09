#include "game/client/render/EndRodParticle.hpp"
#include "game/core/EntityType.hpp"

EndRodParticle::EndRodParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : SimpleAnimatedParticle(level, x, y, z, 176, 8, -0.0005f)
{
    xd = xa;
    yd = ya;
    zd = za;
    size *= 0.75f;
    lifetime = random.nextInt(12) + 60;
    setFadeColor(0xf2dec9);
}

void EndRodParticle::move(double xa, double ya, double za)
{
    setBoundingBox(getBoundingBox()->move(xa, ya, za));
    setLocationFromBoundingbox();
}

uint64_t EndRodParticle::GetType()
{
    return ParticleKind::END_ROD;
}

Particle* EndRodParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                   double xa, double ya, double za, arrayWithLength<int> params)
{
    return new EndRodParticle(level, x, y, z, xa, ya, za);
}

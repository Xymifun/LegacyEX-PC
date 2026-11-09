#include "game/client/render/ChestRefillParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Mth.hpp"

ChestRefillParticle::ChestRefillParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : Particle(level, x, y, z, xa, ya, za)
{
    _init(level, x, y, z, xa, ya, za, 1.0f);
}

void ChestRefillParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks,
                                 float xa, float ya, float za, float xa2, float za2)
{
    float scale = ((float)age + partialTicks) / (float)lifetime * 32.0f;
    scale = Mth::clamp(scale, 0.0f, 1.0f);
    size = baseSize * scale;
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void ChestRefillParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime)
        remove();

    move(xd, yd, zd);
    xd *= 0.8999999761581421;
    yd *= 0.8999999761581421;
    zd *= 0.8999999761581421;
}

void ChestRefillParticle::_init(Level* level, double x, double y, double z, double xa, double ya, double za,
                                float scale)
{
    xd = xa;
    yd = ya;
    zd = za;
    size *= 0.75f;
    size *= scale;
    baseSize = size;
    lifetime = (int)(12.0 / (random.nextDouble() * 0.8 + 0.6));
    lifetime = (int)((float)lifetime * scale);
    hasPhysics = false;
    setMiscTex(65);
}

uint64_t ChestRefillParticle::GetType()
{
    const uint64_t type = ParticleKind::CHEST_REFILL;
    return type;
}

Particle* ChestRefillParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                        double xa, double ya, double za, arrayWithLength<int> params)
{
    ChestRefillParticle* particle = new ChestRefillParticle(level, x, y, z, xa, ya, za);
    particle->tick();
    const int colour = static_cast<int>(gApp.GetHTMLColor(eMinecraftColour_HTMLColor_e));
    particle->setColor(static_cast<float>(colour >> 4), static_cast<float>((colour >> 2) | 0xff),
                       static_cast<float>(colour | 0xff));
    return particle;
}

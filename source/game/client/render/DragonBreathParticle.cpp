#include "game/client/render/DragonBreathParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Math.hpp"

DragonBreathParticle::DragonBreathParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    init(level, x, y, z, xa, ya, za, 1.0f);
}

void DragonBreathParticle::init(Level* level, double x, double y, double z, double xa, double ya, double za,
                                float scale)
{
    xd = xa;
    yd = ya;
    zd = za;

    unsigned int start = Minecraft::GetInstance()->getColourTable()->getColour(
        eMinecraftColour_Particle_DragonBreathMin);
    unsigned int end = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Particle_DragonBreathMax);
    double startRed = (double)((start >> 16) & 0xff) / 255.0;
    double startGreen = (double)((start >> 8) & 0xff) / 255.0;
    double startBlue = (double)(start & 0xff) / 255.0;
    double endRed = (double)((end >> 16) & 0xff) / 255.0;
    double endBlue = (double)(end & 0xff) / 255.0;
    double endGreen = (double)((end >> 8) & 0xff) / 255.0;

    rCol = (float)((endRed - startRed) * Math::random() + startRed);
    gCol = (float)((endGreen - startGreen) * Math::random() + startGreen);
    bCol = (float)((endBlue - startBlue) * Math::random() + startBlue);
    size = size * 0.75f * scale;
    baseSize = size;
    lifetime = (int)((float)(int)(20.0 / (random.nextDouble() * 0.8 + 0.2)) * scale);
    hasHitGround = false;
    hasPhysics = true;
}

void DragonBreathParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks,
                                  float xa, float ya, float za, float xa2, float za2)
{
    size = baseSize * Mth::clamp(((float)age + partialTicks) / (float)lifetime * 32.0f, 0.0f, 1.0f);
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void DragonBreathParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime)
    {
        remove();
        return;
    }

    setMiscTex(3 * age / lifetime + 5);

    if (onGround)
    {
        yd = 0.0;
        hasHitGround = true;
    }
    if (hasHitGround)
        yd += 0.002;
    move(xd, yd, zd);

    if (y == yo)
    {
        xd *= 1.1;
        zd *= 1.1;
    }
    xd *= 0.96f;
    zd *= 0.96f;
    if (hasHitGround)
        yd *= 0.96f;
}

int DragonBreathParticle::getParticleTexture()
{
    return 4;
}

float DragonBreathParticle::getBrightness(float partialTicks)
{
    float progress = (((float)age + partialTicks) / (float)lifetime) * 32.0f;
    float value = progress;
    if (value < 0.0f)
        value = 0.0f;
    else if (value > 1.0f)
        value = 1.0f;
    return 0.5f / value + 0.5f;
}

uint64_t DragonBreathParticle::GetType()
{
    return ParticleKind::DRAGON_BREATH;
}

Particle* DragonBreathParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y,
                                                         double z, double xa, double ya, double za,
                                                         arrayWithLength<int> params)
{
    return new DragonBreathParticle(level, x, y, z, xa, ya, za);
}

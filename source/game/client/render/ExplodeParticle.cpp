#include "game/client/render/ExplodeParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/BufferBuilder.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/Math.hpp"

ExplodeParticle::ExplodeParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : Particle(level, x, y, z, xa, ya, za)
{
    xd = xa + (double)((float)(Math::random() * 2.0 - 1.0) * 0.05f);
    yd = ya + (double)((float)(Math::random() * 2.0 - 1.0) * 0.05f);
    zd = za + (double)((float)(Math::random() * 2.0 - 1.0) * 0.05f);

    const unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(
        eMinecraftColour_Particle_Explode);
    const float red = ((float)((colour >> 16) & 255)) / 255.0f;
    const float green = ((float)((colour >> 8) & 255)) / 255.0f;
    const float blue = ((float)(colour & 255)) / 255.0f;
    const float intensity = (float)(random.nextFloat() * 0.3f + 0.7f);
    rCol = red * intensity;
    gCol = green * intensity;
    bCol = blue * intensity;
    size = (float)(random.nextFloat() * random.nextFloat()) * 6.0f + 1.0f;
    lifetime = (int)(16.0f / (random.nextFloat() * 0.8f + 0.2f)) + 2;
}

uint64_t ExplodeParticle::GetType()
{
    return ParticleKind::EXPLODE;
}

void ExplodeParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks,
                             float xa, float ya, float za, float xa2, float za2)
{
    float px = (float)((x - xo) * partialTicks + xo - Particle::xOff);
    float py = (float)((y - yo) * partialTicks + yo - Particle::yOff);
    float pz = (float)((z - zo) * partialTicks + zo - Particle::zOff);
    float distSq = px * px + py * py + pz * pz;
    if (distSq >= 9.0f)
    {
        Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
    }
}

void ExplodeParticle::tick()
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
    xd *= 0.9;
    yd *= 0.9;
    zd *= 0.9;
    if (onGround)
    {
        xd *= 0.7;
        zd *= 0.7;
    }
}

Particle* ExplodeParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                    double xa, double ya, double za, arrayWithLength<int> params)
{
    return new ExplodeParticle(level, x, y, z, xa, ya, za);
}

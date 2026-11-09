#include "game/client/render/BubblePopParticle.hpp"

#include "game/client/render/TextureUVCoordinateSet.hpp"

BubblePopParticle::BubblePopParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    rCol = 1.0f;
    gCol = 1.0f;
    bCol = 1.0f;
    setMiscTex(256);
    lifetime = 4;
    gravity = 0.008f;
    xd = xa;
    yd = ya;
    zd = za;
}

void BubblePopParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    yd -= gravity;
    move(xd, yd, zd);

    if (age++ >= lifetime)
    {
        remove();
        return;
    }

    int frame = age * 5 / lifetime;
    if (frame <= 4)
    {
        float frameX = frame % 16;
        float frameY = frame / 16;
        uvs = TextureUVCoordinateSet(frameX * 0.0625f + 0.375f, frameY * 0.0625f - 0.625f,
                                     (frameX + 2.0f) * 0.0625f + 0.375f, (frameY + 2.0f) * 0.0625f - 0.625f, 16, 16,
                                     1.0f);
    }
}

uint64_t BubblePopParticle::GetType()
{
    return PARTICLE_TYPE_BUBBLE_POP;
}

Particle* BubblePopParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                      double xa, double ya, double za, arrayWithLength<int> params)
{
    return new BubblePopParticle(level, x, y, z, xa, ya, za);
}

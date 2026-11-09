#include "game/client/render/SimpleAnimatedParticle.hpp"

SimpleAnimatedParticle::SimpleAnimatedParticle(Level* level, double x, double y, double z, int texture, int frameCount,
                                               float verticalSpeed)
    : Particle(level, x, y, z),
      texture(texture),
      frameCount(frameCount),
      verticalSpeed(verticalSpeed),
      baseAirFriction(0.91f),
      fadeRed(0.0f),
      fadeGreen(0.0f),
      fadeBlue(0.0f),
      fading(false)
{
    setMiscTex(texture);
}

void SimpleAnimatedParticle::setColor(int color)
{
    float red = (float)((color >> 16) & 0xff) / 255.0f;
    float green = (float)((color >> 8) & 0xff) / 255.0f;
    float blue = (float)(color & 0xff) / 255.0f;
    Particle::setColor(red, green, blue);
}

void SimpleAnimatedParticle::setFadeColor(int color)
{
    fadeRed = (float)(int)((color >> 16) & 0xff) / 255.0f;
    fadeGreen = (float)(int)((color >> 8) & 0xff) / 255.0f;
    fadeBlue = (float)(int)(color & 0xff) / 255.0f;
    fading = true;
}

bool SimpleAnimatedParticle::isTransparent()
{
    return true;
}

void SimpleAnimatedParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime)
    {
        remove();
    }

    if (age > lifetime / 2)
    {
        setAlpha(1.0f - ((float)age - (float)(lifetime / 2)) / (float)lifetime);
        if (fading)
        {
            rCol += (fadeRed - rCol) * 0.2f;
            gCol += (fadeGreen - gCol) * 0.2f;
            bCol += (fadeBlue - bCol) * 0.2f;
        }
    }

    setMiscTex(texture + (frameCount - 1 - age * frameCount / lifetime));
    yd += (double)verticalSpeed;
    move(xd, yd, zd);
    xd *= baseAirFriction;
    yd *= baseAirFriction;
    zd *= baseAirFriction;
    if (onGround)
    {
        xd *= 0.699999988079071;
        zd *= 0.699999988079071;
    }
}

int SimpleAnimatedParticle::getLightColor(float partialTicks)
{
    return 0xf000f0;
}

float SimpleAnimatedParticle::getBrightness(float partialTicks)
{
    return 1.0f;
}

void SimpleAnimatedParticle::setBaseAirFriction(float friction)
{
    baseAirFriction = friction;
}

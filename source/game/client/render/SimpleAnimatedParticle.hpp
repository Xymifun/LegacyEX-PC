#ifndef GAME_CLIENT_RENDER_SIMPLEANIMATEDPARTICLE_HPP
#define GAME_CLIENT_RENDER_SIMPLEANIMATEDPARTICLE_HPP

#include "game/client/render/Particle.hpp"

class SimpleAnimatedParticle : public Particle
{
public:
    SimpleAnimatedParticle(Level* level, double x, double y, double z, int texture, int frameCount,
                           float verticalSpeed);
    virtual ~SimpleAnimatedParticle() {}
    void setColor(int color);
    void setFadeColor(int color);
    virtual bool isTransparent();
    virtual void tick();
    virtual int getLightColor(float partialTicks);
    virtual float getBrightness(float partialTicks);
    void setBaseAirFriction(float friction);

protected:
    int texture;
    int frameCount;
    float verticalSpeed;
    float baseAirFriction;
    float fadeRed;
    float fadeGreen;
    float fadeBlue;
    bool fading;
};

#endif

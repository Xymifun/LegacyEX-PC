#ifndef GAME_CLIENT_RENDER_BUBBLEPARTICLE_HPP
#define GAME_CLIENT_RENDER_BUBBLEPARTICLE_HPP

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class Level;

class BubbleParticle : public Particle
{
public:
    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

    BubbleParticle(Level* level, double x, double y, double z, double xa, double ya, double za,
                   arrayWithLength<int> params);
    virtual uint64_t GetType();
    virtual void tick();
    virtual int getLightColor(float partialTicks);
    virtual float getBrightness(float partialTicks);

private:
    int getRawBr();
};

#endif

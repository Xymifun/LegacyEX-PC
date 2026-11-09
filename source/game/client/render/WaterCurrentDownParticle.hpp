#ifndef GAME_CLIENT_RENDER_WATERCURRENTDOWNPARTICLE_HPP
#define GAME_CLIENT_RENDER_WATERCURRENTDOWNPARTICLE_HPP

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

const long long PARTICLE_TYPE_CURRENT_DOWN = 0x000000000008022ELL;

class WaterCurrentDownParticle : public Particle
{
public:
    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

    WaterCurrentDownParticle(Level* level, double x, double y, double z);
    virtual void tick();
    virtual float getBrightness(float partialTicks);
    virtual int getLightColor(float partialTicks);
    virtual uint64_t GetType();

private:
    int getRawBr();
    float rotation;
};

#endif

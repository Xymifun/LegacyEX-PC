#ifndef GAME_CLIENT_RENDER_WAKEPARTICLE_HPP
#define GAME_CLIENT_RENDER_WAKEPARTICLE_HPP

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class WakeParticle : public Particle
{
public:
    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

    WakeParticle(Level* level, double x, double y, double z, double xa, double ya, double za);
    virtual void tick();
    virtual uint64_t GetType();
};

#endif

#ifndef GAME_CLIENT_RENDER_SPITPARTICLE_HPP
#define GAME_CLIENT_RENDER_SPITPARTICLE_HPP

#include <stdint.h>

#include "game/client/render/ExplodeParticle.hpp"

class SpitParticle : public ExplodeParticle
{
public:
    SpitParticle(Level* level, double x, double y, double z, double xa, double ya, double za);
    virtual uint64_t GetType();
    virtual void tick();

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };
};

#endif

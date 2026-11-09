#ifndef GAME_CLIENT_RENDER_SUSPENDEDPARTICLE_HPP
#define GAME_CLIENT_RENDER_SUSPENDEDPARTICLE_HPP

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class SuspendedParticle : public Particle
{
public:
    SuspendedParticle(Level* level, double x, double y, double z, double xa, double ya, double za);
    virtual void tick();
    virtual uint64_t GetType();

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };
};

#endif

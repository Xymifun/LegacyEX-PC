#ifndef GAME_CLIENT_RENDER_BUBBLEPOPPARTICLE_HPP
#define GAME_CLIENT_RENDER_BUBBLEPOPPARTICLE_HPP

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class Level;

const long long PARTICLE_TYPE_BUBBLE_POP = 0x000000000008022DLL;

class BubblePopParticle : public Particle
{
public:
    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

    BubblePopParticle(Level* level, double x, double y, double z, double xa, double ya, double za);
    virtual void tick();
    virtual uint64_t GetType();
};

#endif

#ifndef GAME_CLIENT_RENDER_SUSPENDEDTOWNPARTICLE_HPP
#define GAME_CLIENT_RENDER_SUSPENDEDTOWNPARTICLE_HPP

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class SuspendedTownParticle : public Particle
{
public:
    SuspendedTownParticle(Level* level, double x, double y, double z, double xa, double ya, double za);
    virtual void tick();
    virtual void move(double xa, double ya, double za);
    virtual uint64_t GetType();

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

    class HappyVillagerProvider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };
};

#endif

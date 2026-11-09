#ifndef GAME_CLIENT_RENDER_WATERDROPPARTICLE_HPP
#define GAME_CLIENT_RENDER_WATERDROPPARTICLE_HPP

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class WaterDropParticle : public Particle
{
public:
    WaterDropParticle(Level* level, double x, double y, double z);
    virtual uint64_t GetType();
    virtual ~WaterDropParticle() {}
    virtual void tick();

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };
};

#endif

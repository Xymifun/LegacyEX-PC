#ifndef GAME_CLIENT_RENDER_SPLASHPARTICLE_HPP
#define GAME_CLIENT_RENDER_SPLASHPARTICLE_HPP

#include "game/client/render/ParticleProvider.hpp"
#include "game/client/render/WaterDropParticle.hpp"

class SplashParticle : public WaterDropParticle
{
public:
    SplashParticle(Level* level, double x, double y, double z, double xa, double ya, double za);
    virtual uint64_t GetType();

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };
};

#endif

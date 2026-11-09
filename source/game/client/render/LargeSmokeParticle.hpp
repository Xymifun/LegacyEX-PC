#ifndef GAME_CLIENT_RENDER_LARGESMOKEPARTICLE_HPP
#define GAME_CLIENT_RENDER_LARGESMOKEPARTICLE_HPP

#include "game/client/render/ParticleProvider.hpp"
#include "game/client/render/SmokeParticle.hpp"

class LargeSmokeParticle : public SmokeParticle
{
public:
    LargeSmokeParticle(Level* level, double x, double y, double z, double xa, double ya, double za);
    virtual uint64_t GetType();

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };
};

#endif

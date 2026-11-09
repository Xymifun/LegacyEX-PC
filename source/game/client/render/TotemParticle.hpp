#ifndef GAME_CLIENT_RENDER_TOTEMPARTICLE_HPP
#define GAME_CLIENT_RENDER_TOTEMPARTICLE_HPP

#include "game/client/render/ParticleProvider.hpp"
#include "game/client/render/SimpleAnimatedParticle.hpp"

class TotemParticle : public SimpleAnimatedParticle
{
public:
    TotemParticle(Level* level, double x, double y, double z, double xa, double ya, double za);
    virtual ~TotemParticle() {}
    virtual uint64_t GetType();

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };
};

#endif

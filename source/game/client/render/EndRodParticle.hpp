#ifndef GAME_CLIENT_RENDER_ENDRODPARTICLE_HPP
#define GAME_CLIENT_RENDER_ENDRODPARTICLE_HPP

#include "game/client/render/ParticleProvider.hpp"
#include "game/client/render/SimpleAnimatedParticle.hpp"

class EndRodParticle : public SimpleAnimatedParticle
{
public:
    EndRodParticle(Level* level, double x, double y, double z, double xa, double ya, double za);
    virtual uint64_t GetType();
    virtual void move(double xa, double ya, double za);

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };
};

#endif

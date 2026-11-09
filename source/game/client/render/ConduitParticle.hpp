#ifndef GAME_CLIENT_RENDER_CONDUITPARTICLE_HPP
#define GAME_CLIENT_RENDER_CONDUITPARTICLE_HPP

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

const long long PARTICLE_TYPE_CONDUIT = 0x000000000008022FLL;

class ConduitParticle : public Particle
{
public:
    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

    ConduitParticle(Level* level, double x, double y, double z, double xa, double ya, double za,
                    arrayWithLength<int> params);
    virtual void tick();
    virtual uint64_t GetType();

private:
    float m_conduitParticleValue;
    double m_startX;
    double m_startY;
    double m_startZ;
};

#endif

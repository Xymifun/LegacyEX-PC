#ifndef GAME_CLIENT_RENDER_HUGEEXPLOSIONSEEDPARTICLE_HPP
#define GAME_CLIENT_RENDER_HUGEEXPLOSIONSEEDPARTICLE_HPP

#include <stdint.h>

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class HugeExplosionSeedParticle : public Particle
{
public:
    HugeExplosionSeedParticle(Level* level, double x, double y, double z, double xa, double ya, double za);
    virtual uint64_t GetType();
    virtual void tick();
    virtual void render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                        float ya, float za, float xa2, float za2);
    virtual int getParticleTexture();

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };
};

#endif

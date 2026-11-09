#ifndef GAME_CLIENT_RENDER_BARRIERPARTICLE_HPP
#define GAME_CLIENT_RENDER_BARRIERPARTICLE_HPP

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class Item;

class BarrierParticle : public Particle
{
public:
    BarrierParticle(Level* level, double x, double y, double z, Item* item);
    virtual int getParticleTexture();
    virtual void render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                        float ya, float za, float xa2, float za2);
    virtual uint64_t GetType();

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };
};

#endif

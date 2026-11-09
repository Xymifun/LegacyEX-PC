#ifndef GAME_CLIENT_RENDER_CHESTREFILLPARTICLE_HPP
#define GAME_CLIENT_RENDER_CHESTREFILLPARTICLE_HPP

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class ChestRefillParticle : public Particle
{
public:
    ChestRefillParticle(Level* level, double x, double y, double z, double xa, double ya, double za);
    ChestRefillParticle(Level* level, double x, double y, double z, double xa, double ya, double za, float scale);
    virtual uint64_t GetType();
    virtual void tick();
    virtual void render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                        float ya, float za, float xa2, float za2);

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

private:
    void _init(Level* level, double x, double y, double z, double xa, double ya, double za, float scale);
    float baseSize;
};

#endif

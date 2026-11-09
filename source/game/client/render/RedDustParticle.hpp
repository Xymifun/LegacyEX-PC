#ifndef GAME_CLIENT_RENDER_REDDUSTPARTICLE_HPP
#define GAME_CLIENT_RENDER_REDDUSTPARTICLE_HPP

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class RedDustParticle : public Particle
{
public:
    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

    RedDustParticle(Level* level, double x, double y, double z, float red, float green, float blue);
    RedDustParticle(Level* level, double x, double y, double z, float alpha, float red, float green, float blue);
    void init(Level* level, double x, double y, double z, float alpha, float red, float green, float blue);
    virtual void render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                        float ya, float za, float xa2, float za2);
    virtual void tick();
    virtual uint64_t GetType();

private:
    float size;
};

#endif

#ifndef GAME_CLIENT_RENDER_DRIPPARTICLE_HPP
#define GAME_CLIENT_RENDER_DRIPPARTICLE_HPP

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class Material;

class DripParticle : public Particle
{
public:
    DripParticle(Level* level, double x, double y, double z, Material* material);
    virtual uint64_t GetType();
    virtual void tick();
    virtual int getLightColor(float partialTicks);
    virtual float getBrightness(float partialTicks);

    class LavaProvider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

    class WaterProvider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

private:
    Material* material;
    int delay;
};

#endif

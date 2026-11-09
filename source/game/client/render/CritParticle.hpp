#ifndef GAME_CLIENT_RENDER_CRITPARTICLE_HPP
#define GAME_CLIENT_RENDER_CRITPARTICLE_HPP

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class CritParticle : public Particle
{
public:
    CritParticle(Level* level, double x, double y, double z, double xa, double ya, double za);
    CritParticle(Level* level, double x, double y, double z, double xa, double ya, double za, float scale);
    virtual uint64_t GetType();
    virtual void render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                        float ya, float za, float xa2, float za2);
    virtual void tick();

    class DamageIndicatorProvider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

    class MagicProvider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

private:
    void _init(Level* level, double x, double y, double z, double xa, double ya, double za, float scale);
    float sizeMultiplier;
};

#endif

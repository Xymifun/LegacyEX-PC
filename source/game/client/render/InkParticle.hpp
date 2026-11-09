#ifndef GAME_CLIENT_RENDER_INKPARTICLE_HPP
#define GAME_CLIENT_RENDER_INKPARTICLE_HPP

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class BufferBuilder;

class InkParticle : public Particle
{
public:
    InkParticle(Level* level, double x, double y, double z, double xa, double ya, double za,
                arrayWithLength<int> params);
    virtual void render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                        float ya, float za, float xa2, float za2);
    virtual void tick();
    virtual uint64_t GetType();

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

private:
    float mStartSize;
    float mEndSize;
    float mScale;
};

#endif

#ifndef GAME_CLIENT_RENDER_DRAGONBREATHPARTICLE_HPP
#define GAME_CLIENT_RENDER_DRAGONBREATHPARTICLE_HPP

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class DragonBreathParticle : public Particle
{
public:
    DragonBreathParticle(Level* level, double x, double y, double z, double xa, double ya, double za);
    DragonBreathParticle(Level* level, double x, double y, double z, double xa, double ya, double za, float scale);
    void init(Level* level, double x, double y, double z, double xa, double ya, double za, float scale);
    virtual void render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                        float ya, float za, float xa2, float za2);
    virtual void tick();
    virtual int getParticleTexture();
    virtual float getBrightness(float partialTicks);
    virtual uint64_t GetType();

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

private:
    float baseSize;
    bool hasHitGround;
};

#endif

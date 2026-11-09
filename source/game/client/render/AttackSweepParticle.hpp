#ifndef GAME_CLIENT_RENDER_ATTACKSWEEPPARTICLE_HPP
#define GAME_CLIENT_RENDER_ATTACKSWEEPPARTICLE_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class BufferBuilder;
class Entity;
class Level;
class Textures;

class AttackSweepParticle : public Particle
{
public:
    AttackSweepParticle(Textures* textures, Level* level, double x, double y, double z, double xa, double ya,
                        double za);
    virtual void render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                        float ya, float za, float xa2, float za2);
    virtual int getLightColor(float partialTicks);
    virtual void tick();
    virtual int getParticleTexture();
    virtual uint64_t GetType();

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

private:
    Textures* textures;
    float quadSize;
};

#endif

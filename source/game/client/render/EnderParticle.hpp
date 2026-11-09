#ifndef GAME_CLIENT_RENDER_ENDERPARTICLE_HPP
#define GAME_CLIENT_RENDER_ENDERPARTICLE_HPP

#include <boost/enable_shared_from_this.hpp>
#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class BufferBuilder;

class EnderParticle : public Particle
{
public:
    EnderParticle(Level* level, double x, double y, double z, double xa, double ya, double za);
    virtual void render(BufferBuilder*, const boost::shared_ptr<Entity>&, float, float, float, float, float, float);
    virtual int getLightColor(float);
    virtual float getBrightness(float);
    virtual void tick();
    virtual void move(double, double, double);
    virtual uint64_t GetType();
    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE, Level*, double, double, double, double, double, double,
                                         arrayWithLength<int>);
    };

private:
    float scale;
    double originX;
    double originY;
    double originZ;
};

#endif

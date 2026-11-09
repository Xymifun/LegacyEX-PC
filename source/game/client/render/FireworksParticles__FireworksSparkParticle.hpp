#ifndef GAME_CLIENT_RENDER_FIREWORKSPARTICLES_FIREWORKSSPARKPARTICLE_HPP
#define GAME_CLIENT_RENDER_FIREWORKSPARTICLES_FIREWORKSSPARKPARTICLE_HPP

#include <stdint.h>

#include <boost/shared_ptr.hpp>

#include "game/client/render/SimpleAnimatedParticle.hpp"

class BufferBuilder;
class Entity;
class Level;
class ParticleEngine;

class FireworksParticles
{
public:
    class FireworksOverlayParticle;
    class FireworksSparkParticle;
    class FireworksStarter;
    class Provider;

    class FireworksSparkParticle : public SimpleAnimatedParticle
    {
    public:
        FireworksSparkParticle(Level* level, double x, double y, double z, double xa, double ya, double za,
                               ParticleEngine* particleEngine);
        void setTrail(bool trail);
        void setFlicker(bool flicker);
        virtual void render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks,
                            float xa, float ya, float za, float xa2, float za2);
        virtual void tick();
        virtual uint64_t GetType();

    private:
        bool trail;
        bool flicker;
        ParticleEngine* particleEngine;
    };
};

#endif

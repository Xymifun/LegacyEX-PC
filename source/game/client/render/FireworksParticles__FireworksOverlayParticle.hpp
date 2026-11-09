#ifndef GAME_CLIENT_RENDER_FIREWORKSPARTICLES_FIREWORKSOVERLAYPARTICLE_HPP
#define GAME_CLIENT_RENDER_FIREWORKSPARTICLES_FIREWORKSOVERLAYPARTICLE_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/FireworksParticles__FireworksSparkParticle.hpp"
#include "game/client/render/Particle.hpp"

class BufferBuilder;
class Entity;

class FireworksParticles::FireworksOverlayParticle : public Particle
{
public:
    FireworksOverlayParticle(Level* level, double x, double y, double z);
    virtual void render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                        float ya, float za, float xa2, float za2);
    virtual uint64_t GetType();
};

#endif

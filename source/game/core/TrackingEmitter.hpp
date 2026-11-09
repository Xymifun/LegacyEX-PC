#ifndef GAME_CORE_TRACKINGEMITTER_HPP
#define GAME_CORE_TRACKINGEMITTER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleType.hpp"

class BufferBuilder;
class Entity;
class Level;

class TrackingEmitter : public Particle
{
public:
    TrackingEmitter(Level* level, boost::shared_ptr<Entity> entity, const ParticleType* particleType, int lifetime);
    virtual uint64_t GetType();
    virtual void tick();
    virtual void render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                        float ya, float za, float xa2, float za2);
    virtual int getParticleTexture();

private:
    boost::shared_ptr<Entity> mEntity;
    const ParticleType* mParticleType;
};

#endif

#ifndef GAME_CLIENT_RENDER_MOBAPPEARANCEPARTICLE_HPP
#define GAME_CLIENT_RENDER_MOBAPPEARANCEPARTICLE_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class BufferBuilder;
class ElderGuardian;
class Entity;
class Level;

class MobAppearanceParticle : public Particle
{
public:
    MobAppearanceParticle(Level* level, double x, double y, double z);
    virtual uint64_t GetType();
    virtual void tick();
    bool isAffected();
    virtual void render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                        float ya, float za, float xa2, float za2);
    virtual int getParticleTexture();

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

private:
    boost::shared_ptr<ElderGuardian> guardian;
};

#endif

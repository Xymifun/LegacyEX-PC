#ifndef GAME_CLIENT_RENDER_BREAKINGITEMPARTICLE_HPP
#define GAME_CLIENT_RENDER_BREAKINGITEMPARTICLE_HPP

#include <stdint.h>
#include <boost/shared_ptr.hpp>

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class BufferBuilder;
class Entity;
class Item;
class Level;

class BreakingItemParticle : public Particle
{
public:
    BreakingItemParticle(Level* level, double x, double y, double z, Item* item, int meta);
    BreakingItemParticle(Level* level, double x, double y, double z, double xa, double ya, double za, Item* item,
                         int meta);
    virtual uint64_t GetType();
    virtual void render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                        float ya, float za, float xa2, float za2);
    virtual int getParticleTexture();

private:
    void _init(Item* item, int meta);
    Item* item;

public:
    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

    class SlimeProvider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

    class SnowballProvider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };
};

#endif

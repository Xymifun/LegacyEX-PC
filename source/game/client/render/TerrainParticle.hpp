#ifndef GAME_CLIENT_RENDER_TERRAINPARTICLE_HPP
#define GAME_CLIENT_RENDER_TERRAINPARTICLE_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class BlockPos;
class BlockState;
class BufferBuilder;
class Entity;
class Level;

const long long PARTICLE_TYPE_TERRAIN = 0x0000000000080210LL;

class TerrainParticle : public Particle
{
public:
    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

    TerrainParticle(Level* level, double x, double y, double z, double xa, double ya, double za,
                    const BlockState* state);

    virtual uint64_t GetType();
    virtual ~TerrainParticle();
    virtual void render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                        float ya, float za, float xa2, float za2);
    virtual int getParticleTexture();
    virtual bool isSemiTransparent();
    virtual int getLightColor(float partialTicks);

    void multiplyColor(const BlockPos* pos);

    TerrainParticle* init(const BlockPos& pos);
    TerrainParticle* init();

    const BlockState* blockState;
    BlockPos* pos;
};

#endif

#ifndef GAME_CLIENT_RENDER_TERRAINDUSTPARTICLE_HPP
#define GAME_CLIENT_RENDER_TERRAINDUSTPARTICLE_HPP

#include "game/client/render/TerrainParticle.hpp"

class BlockState;

class TerrainDustParticle : public TerrainParticle
{
public:
    TerrainDustParticle(Level* level, double x, double y, double z, double xa, double ya, double za,
                        const BlockState* state);
    virtual uint64_t GetType();

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };
};

#endif

#include "game/client/render/TerrainDustParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"

TerrainDustParticle::TerrainDustParticle(Level* level, double x, double y, double z, double xa, double ya, double za,
                                         const BlockState* state)
    : TerrainParticle(level, x, y, z, xa, ya, za, state)
{
    xd = xa;
    yd = ya;
    zd = za;
}

uint64_t TerrainDustParticle::GetType()
{
    return ParticleKind::TERRAIN_DUST;
}

Particle* TerrainDustParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                        double xa, double ya, double za, arrayWithLength<int> params)
{
    const BlockState* state = Block::stateByIdWithData(params.data[0]);
    if (state->getRenderShape() != BLOCK_RENDER_INVISIBLE)
    {
        return (new TerrainDustParticle(level, x, y, z, xa, ya, za, state))->init();
    }
    return NULL;
}

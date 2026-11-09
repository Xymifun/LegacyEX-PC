#include "game/client/render/TerrainParticle.hpp"

#include "game/core/BufferBuilder.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockColors.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/world/Level.hpp"

#include "game/core/Minecraft.hpp"

TerrainParticle::TerrainParticle(Level* level, double x, double y, double z, double xa, double ya, double za,
                                 const BlockState* state)
    : Particle(level, x, y, z, xa, ya, za)
{
    blockState = state;

    setSprite(state->getBlock()->getIconTexture(Direction::DOWN, state));

    gravity = state->getBlock()->mParticleGravity;
    rCol = gCol = bCol = 0.6f;
    size /= 2.0f;
    pos = NULL;

    float u = uo / 4.0f / 16.0f;
    float v = vo * 3.0f / 4.0f / 16.0f;

    uvs = TextureUVCoordinateSet(u, v, u + 0.015609375f, v + 0.015609375f, 1, 1);

    if (sprite != NULL)
    {
        uvs.u0 = sprite->getU(uo / 4.0f * 16.0f);
        uvs.u1 = sprite->getU((uo + 1.0f) / 4.0f * 16.0f);
        uvs.v0 = sprite->getV(vo / 4.0f * 16.0f);
        uvs.v1 = sprite->getV((vo + 1.0f) / 4.0f * 16.0f);
    }
    else if (state->getBlock() == Blocks::conduit)
    {
        uo = random.nextFloat();
        vo = random.nextFloat();

        float u0 = sprite->getU(uo * 0.5f * 3.0f + 5.0f);
        float u1 = sprite->getU((uo + 0.5f) * 3.0f + 5.0f);
        float v0 = sprite->getV(vo * 0.5f * 3.0f + 5.0f);
        float v1 = sprite->getV((vo + 0.5f) * 3.0f + 5.0f);

        uvs = TextureUVCoordinateSet(u0, v0, u1, v1, 1, 1);
    }
}

TerrainParticle::~TerrainParticle()
{
    delete pos;
    pos = NULL;
}

uint64_t TerrainParticle::GetType()
{
    return PARTICLE_TYPE_TERRAIN;
}

void TerrainParticle::multiplyColor(const BlockPos* pos)
{
    unsigned int colour = Minecraft::GetInstance()->getBlockColors()->getColor(blockState, level, pos, 0);

    rCol *= (float)(int)((colour >> 16) & 0xFF) / 255.0f;
    gCol *= (float)(int)((colour >> 8) & 0xFF) / 255.0f;
    bCol *= (float)(int)(colour & 0xFF) / 255.0f;
}

TerrainParticle* TerrainParticle::init(const BlockPos& pos)
{
    delete this->pos;
    this->pos = new BlockPos(pos);

    if (blockState->getBlock() != Blocks::grass)
    {
        multiplyColor(this->pos);
    }

    return this;
}

TerrainParticle* TerrainParticle::init()
{
    delete pos;
    pos = new BlockPos(x, y, z);

    if (blockState->getBlock() != Blocks::grass)
    {
        multiplyColor(pos);
    }

    return this;
}

int TerrainParticle::getParticleTexture()
{
    return 1;
}

void TerrainParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks,
                             float xa, float ya, float za, float xa2, float za2)
{
    float scale = 0.1f * size;

    float px = (float)(xo + (x - xo) * partialTicks - xOff);
    float py = (float)(yo + (y - yo) * partialTicks - yOff);
    float pz = (float)(zo + (z - zo) * partialTicks - zOff);

    int light = getLightColor(partialTicks);

    float distSq = px * px + py * py + pz * pz;
    if (distSq >= 1.0f)
    {
        buffer->color(rCol, gCol, bCol);
        buffer->tex2(light);

        buffer->vertexUV(px - xa * scale - xa2 * scale, py - ya * scale, pz - za * scale - za2 * scale, uvs.u0, uvs.v1);
        buffer->vertexUV(px - xa * scale + xa2 * scale, py + ya * scale, pz - za * scale + za2 * scale, uvs.u0, uvs.v0);
        buffer->vertexUV(px + xa * scale + xa2 * scale, py + ya * scale, pz + za * scale + za2 * scale, uvs.u1, uvs.v0);
        buffer->vertexUV(px + xa * scale - xa2 * scale, py - ya * scale, pz + za * scale - za2 * scale, uvs.u1, uvs.v1);
    }
}

int TerrainParticle::getLightColor(float partialTicks)
{
    int light = Particle::getLightColor(partialTicks);
    int levelLight = 0;

    if (level->hasChunkAt(*pos))
    {
        levelLight = level->getLightColor(*pos, 0, -1);
    }

    return light == 0 ? levelLight : light;
}

bool TerrainParticle::isSemiTransparent()
{
    return blockState->getBlock() == Blocks::stained_glass_pane || blockState->getBlock() == Blocks::stained_glass;
}

Particle* TerrainParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                    double xa, double ya, double za, arrayWithLength<int> params)
{
    return (new TerrainParticle(level, x, y, z, xa, ya, za, Block::stateByIdWithData(params.data[0])))->init();
}

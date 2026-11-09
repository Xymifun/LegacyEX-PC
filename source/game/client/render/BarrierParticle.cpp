#include "game/client/render/BarrierParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Direction.hpp"
#include "game/item/Item.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

BarrierParticle::BarrierParticle(Level* level, double x, double y, double z, Item* item)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    setSprite(Blocks::barrier->getTexture(Direction::UP));
    rCol = 1.0f;
    gCol = 1.0f;
    bCol = 1.0f;
    xd = 0.0;
    yd = 0.0;
    zd = 0.0;
    gravity = 0.0f;
    lifetime = 80;
}

int BarrierParticle::getParticleTexture()
{
    return 1;
}

void BarrierParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks,
                             float xa, float ya, float za, float xa2, float za2)
{
    float u0 = sprite->getU0(false);
    float u1 = sprite->getU1(false);
    float v0 = sprite->getV0(false);
    float v1 = sprite->getV1(false);
    float scale = 0.5f;
    float px = (float)(xo + (x - xo) * partialTicks - Particle::xOff);
    float py = (float)(yo + (y - yo) * partialTicks - Particle::yOff);
    float pz = (float)(zo + (z - zo) * partialTicks - Particle::zOff);
    int light = getLightColor(partialTicks);

    buffer->color(rCol, gCol, bCol);
    buffer->tex2(light);
    buffer->vertexUV(px - xa * scale - xa2 * scale, py - ya * scale, pz - za * scale - za2 * scale, u1, v1);
    buffer->vertexUV(px - xa * scale + xa2 * scale, py + ya * scale, pz - za * scale + za2 * scale, u1, v0);
    buffer->vertexUV(px + xa * scale + xa2 * scale, py + ya * scale, pz + za * scale + za2 * scale, u0, v0);
    buffer->vertexUV(px + xa * scale - xa2 * scale, py - ya * scale, pz + za * scale - za2 * scale, u0, v1);
}

uint64_t BarrierParticle::GetType()
{
    return ParticleKind::BARRIER;
}

Particle* BarrierParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                    double xa, double ya, double za, arrayWithLength<int> params)
{
    return new BarrierParticle(level, x, y, z, Item::byBlock(Blocks::barrier));
}

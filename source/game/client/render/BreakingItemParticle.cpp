#include "game/client/render/BreakingItemParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/client/render/TextureUVCoordinateSet.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

BreakingItemParticle::BreakingItemParticle(Level* level, double x, double y, double z, Item* item, int meta)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    _init(item, meta);
}

BreakingItemParticle::BreakingItemParticle(Level* level, double x, double y, double z, double xa, double ya, double za,
                                           Item* item, int meta)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    _init(item, meta);
    xd *= 0.1f;
    yd *= 0.1f;
    zd *= 0.1f;
    xd += xa;
    yd += ya;
    zd += za;
}

int BreakingItemParticle::getParticleTexture()
{
    int iconType = item->getIconType();
    return iconType != 0 ? 2 : 1;
}

void BreakingItemParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks,
                                  float xa, float ya, float za, float xa2, float za2)
{
    float scale = 0.1f * size;

    if (sprite != NULL)
    {
        uvs.u0 = sprite->getU(uo / 4.0f * 16.0f);
        uvs.u1 = sprite->getU((uo + 1.0f) / 4.0f * 16.0f);
        uvs.v0 = sprite->getV(vo / 4.0f * 16.0f);
        uvs.v1 = sprite->getV((vo + 1.0f) / 4.0f * 16.0f);
    }

    float px = (float)(xo + (x - xo) * partialTicks - xOff);
    float py = (float)(yo + (y - yo) * partialTicks - yOff);
    float pz = (float)(zo + (z - zo) * partialTicks - zOff);
    int light = getLightColor(partialTicks);

    buffer->color(rCol, gCol, bCol);
    buffer->tex2(light);
    buffer->vertexUV(px - xa * scale - xa2 * scale, py - ya * scale, pz - za * scale - za2 * scale, uvs.u0, uvs.v1);
    buffer->vertexUV(px - xa * scale + xa2 * scale, py + ya * scale, pz - za * scale + za2 * scale, uvs.u0, uvs.v0);
    buffer->vertexUV(px + xa * scale + xa2 * scale, py + ya * scale, pz + za * scale + za2 * scale, uvs.u1, uvs.v0);
    buffer->vertexUV(px + xa * scale - xa2 * scale, py - ya * scale, pz + za * scale - za2 * scale, uvs.u1, uvs.v1);
}

void BreakingItemParticle::_init(Item* item, int meta)
{
    this->item = item;
    setSprite(this->item->getIcon(meta));
    rCol = gCol = bCol = 1.0f;
    gravity = Blocks::snow->mParticleGravity;
    size /= 2.0f;

    float u = uo / 4.0f / 16.0f;
    float v = vo / 4.0f / 16.0f;
    uvs = TextureUVCoordinateSet(u, v, u + 0.015609375f, v + 0.015609375f, 1, 1);
}

uint64_t BreakingItemParticle::GetType()
{
    return ParticleKind::BREAKING_ITEM;
}

Particle* BreakingItemParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y,
                                                         double z, double xa, double ya, double za,
                                                         arrayWithLength<int> params)
{
    int meta = params.length > 1 ? params.data[1] : 0;
    return new BreakingItemParticle(level, x, y, z, xa, ya, za, Item::byId(params.data[0]), meta);
}

Particle* BreakingItemParticle::SlimeProvider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y,
                                                              double z, double xa, double ya, double za,
                                                              arrayWithLength<int> params)
{
    return new BreakingItemParticle(level, x, y, z, Items::slimeBall, 0);
}

Particle* BreakingItemParticle::SnowballProvider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y,
                                                                 double z, double xa, double ya, double za,
                                                                 arrayWithLength<int> params)
{
    return new BreakingItemParticle(level, x, y, z, Items::snowball, 0);
}

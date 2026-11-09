#include "game/client/render/InkParticle.hpp"
#include "game/world/block/Material.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/TextureUVCoordinateSet.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/util/Math.hpp"
#include "game/core/Mth.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

InkParticle::InkParticle(Level* level, double x, double y, double z, double xa, double ya, double za,
                         arrayWithLength<int> params)
    : Particle(level, x, y, z, xa, ya, za)
{
    mScale = 1.0f;
    float scale = 1.0f;
    if (params.length == 1)
        scale = (float)params.data[0] / 100.0f;

    xd *= 0.1;
    yd *= 0.1;
    zd *= 0.1;
    xd += xa;
    yd += ya;
    zd += za;
    rCol = gCol = bCol = (float)(Math::random() * 0.05);
    size *= 0.75f;
    size *= scale;
    mStartSize = 0.5f * size;
    mEndSize = size;
    lifetime = (int)(scale * 12.0f / ((float)Math::random() * 0.8f + 0.2f));
}

void InkParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                         float ya, float za, float xa2, float za2)
{
    float growth = Mth::clamp(((float)age + partialTicks) / (float)lifetime * 16.0f, 0.0f, 1.0f);
    size = (mEndSize - mStartSize) * growth + mStartSize;
    uvs = TextureUVCoordinateSet::fromOldSystem(8 - (age * 8) / lifetime);
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void InkParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
    move(xd, yd, zd);
    Material* material = level->getBlockState(BlockPos(x, y, z))->getMaterial();
    if (material == Material::AIR)
        yd -= 0.008;
    if (y == yo)
    {
        xd *= 1.1;
        zd *= 1.1;
    }
    xd *= 0.92;
    yd *= 0.92;
    zd *= 0.92;
    if (onGround)
    {
        xd *= 0.7;
        zd *= 0.7;
    }
    if (age++ >= lifetime)
        remove();
}

uint64_t InkParticle::GetType()
{
    return ParticleKind::INK;
}

Particle* InkParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                double xa, double ya, double za, arrayWithLength<int> params)
{
    return new InkParticle(level, x, y, z, xa, ya, za, params);
}

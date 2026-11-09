#include "game/client/render/WaterDropParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Mth.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Math.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"

WaterDropParticle::WaterDropParticle(Level* level, double x, double y, double z)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    xd *= 0.3;
    yd = random.nextDouble() * 0.2 + 0.1;
    zd *= 0.3;
    rCol = 1.0f;
    gCol = 1.0f;
    bCol = 1.0f;
    setMiscTex(random.nextInt(4) + 0x13);
    setSize(0.01f, 0.01f);
    hasPhysics = false;
    gravity = 0.06f;
    lifetime = (int)(8.0 / (random.nextDouble() * 0.8 + 0.2));
}

void WaterDropParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
    yd -= gravity;
    move(xd, yd, zd);
    xd *= 0.98;
    yd *= 0.98;
    zd *= 0.98;
    if (lifetime-- <= 0)
        remove();

    if (onGround)
    {
        if (Math::random() < 0.5)
            remove();
        xd *= 0.7;
        zd *= 0.7;
    }

    BlockPos position(x, y, z);
    const BlockState* state = level->getBlockState(position);
    Material* material = state->getMaterial();
    if (state->getBlock()->ParticlesSurviveWithin())
        return;
    if (!material->isLiquid() && !material->isSolid())
        return;

    double surface;
    if (state->getBlock()->isLiquidBlock())
        surface = 1.0f - LiquidBlock::getHeightFromData(state->getValue(LiquidBlock::LEVEL));
    else
        surface = state->getShape(level, position)->mMaxY;

    if (y < Mth::floor(y) + surface)
        remove();
}

uint64_t WaterDropParticle::GetType()
{
    return ParticleKind::WATER_DROP;
}

Particle* WaterDropParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                      double xa, double ya, double za, arrayWithLength<int> params)
{
    return new WaterDropParticle(level, x, y, z);
}

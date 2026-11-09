#include "game/client/render/WaterCurrentDownParticle.hpp"

#include "game/core/Mth.hpp"
#include "game/util/Math.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

WaterCurrentDownParticle::WaterCurrentDownParticle(Level* level, double x, double y, double z)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    setMiscTex(32);
    lifetime = (int)(Math::random() * 60.0) + 30;
    xd = 0.0;
    yd = -0.05;
    zd = 0.0;
    setSize(0.02f, 0.02f);
    size *= random.nextFloat() * 0.6f + 0.2f;
    gravity = 0.002f;
    rotation = 0.0f;
}

void WaterCurrentDownParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    xd += (double)((float)(0.6f * Mth::cos(rotation)));
    zd += (double)((float)(0.6f * Mth::sin(rotation)));
    xd *= 0.07;
    zd *= 0.07;
    move(xd, yd, zd);

    if (age++ >= lifetime || onGround)
        remove();

    BlockPos pos(x, y, z);
    const BlockState* state = level->getBlockState(pos);
    Block* block = level->getBlock(pos);
    Block* blockBelow = level->getBlock(pos.below());
    bool survives = block->ParticlesSurviveWithin()
                    || (blockBelow->ParticlesSurviveWithin() && state->getMaterial() == Material::AIR);

    Material* material = level->getBlockState(BlockPos(x, y, z))->getMaterial();
    if (material != Material::WATER && material != Material::BUBBLE_COLUMN && !survives)
    {
        remove();
    }

    rotation += 0.08f;
}

int WaterCurrentDownParticle::getRawBr()
{
    int brightness = 15;
    BlockPos pos(x, y, z);

    if (level->hasChunkAt(pos))
        brightness = level->getRawBrightness(pos);

    return Mth::floor(Mth::Lerp(7.5f, 15.0f, (float)brightness / 15.0f));
}

float WaterCurrentDownParticle::getBrightness(float partialTicks)
{
    return level->mDimension->getBrightnessRamp()[getRawBr()];
}

int WaterCurrentDownParticle::getLightColor(float partialTicks)
{
    BlockPos pos(x, y, z);
    uint32_t key = ((pos.x & 0xff) << 16) | ((pos.y & 0xff) << 8) | (pos.z & 0xff);

    if (key == cachedLightPos)
        return cachedLightColor;

    int brightness = getRawBr();
    cachedLightPos = key;
    cachedLightColor = (brightness << 20) | (brightness << 4);
    return cachedLightColor;
}

uint64_t WaterCurrentDownParticle::GetType()
{
    return PARTICLE_TYPE_CURRENT_DOWN;
}

Particle* WaterCurrentDownParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y,
                                                             double z, double xa, double ya, double za,
                                                             arrayWithLength<int> params)
{
    return new WaterCurrentDownParticle(level, x, y, z);
}

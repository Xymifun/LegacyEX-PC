#include "game/client/render/FallingDustParticle.hpp"
#include "game/world/block/Blocks.hpp"

#include <cmath>

#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockColors.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/FallingBlock.hpp"

FallingDustParticle::FallingDustParticle(Level* level, double x, double y, double z, float red, float green, float blue)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    mRotSpeed = (random.nextFloat() - 0.5f) * 0.1f;
    xd = 0.0;
    yd = 0.0;
    zd = 0.0;
    rCol = red;
    gCol = green;
    bCol = blue;
    Particle::size *= 0.75f;
    Particle::size *= 0.9f;
    m_size = Particle::size;
    lifetime = (int)std::floor(32.0f / (random.nextFloat() * 0.8f + 0.2f) * 0.9f);
    roll = random.nextFloat() * 6.2831855f;
}

void FallingDustParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks,
                                 float xa, float ya, float za, float xa2, float za2)
{
    Particle::size = m_size * Mth::clamp(((float)age + partialTicks) / (float)lifetime * 32.0f, 0.0f, 1.0f);
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void FallingDustParticle::tick()
{
    int previousAge = age;
    int particleLifetime = lifetime;
    age = previousAge + 1;
    xo = x;
    yo = y;
    zo = z;
    if (previousAge >= particleLifetime)
    {
        remove();
        particleLifetime = lifetime;
    }

    float spin = 3.1415927f * mRotSpeed;
    float previousRoll = roll;
    float nextRoll = roll + (spin + spin);
    if (onGround)
    {
        previousRoll = 0.0f;
        nextRoll = 0.0f;
    }
    roll = nextRoll;
    oRoll = previousRoll;
    setMiscTex(7 - (age << 3) / particleLifetime);
    move(xd, yd, zd);
    yd -= 0.003;
    yd = yd < -0.14 ? -0.14 : yd;
}

uint64_t FallingDustParticle::GetType()
{
    return PARTICLE_TYPE_FALLING_DUST;
}

Particle* FallingDustParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                        double xa, double ya, double za, arrayWithLength<int> params)
{
    const BlockState* state = Block::stateByIdWithData(params.data[0]);
    if (state->getBlock() != Blocks::air && state->getRenderShape() == BLOCK_RENDER_INVISIBLE)
        return NULL;

    unsigned int colour = Minecraft::GetInstance()->getBlockColors()->getColor(state, (LevelSource*)level,
                                                                               BlockPos(x, y, z));
    if (state->getBlock()->instanceof(EBLOCK_TYPE_FALLING))
        colour = static_cast<FallingBlock*>(state->getBlock())->getDustColor(state);
    float red = (float)(int)((colour >> 16) & 0xff) / 255.0f;
    float green = (float)(int)((colour >> 8) & 0xff) / 255.0f;
    float blue = (float)(int)(colour & 0xff) / 255.0f;
    return new FallingDustParticle(level, x, y, z, red, green, blue);
}

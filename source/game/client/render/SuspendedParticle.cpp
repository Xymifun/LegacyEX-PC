#include "game/client/render/SuspendedParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

SuspendedParticle::SuspendedParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : Particle(level, x, y - 0.125, z, xa, ya, za)
{
    unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Particle_Suspend);
    rCol = (float)((colour >> 16) & 0xff) / 255.0f;
    gCol = (float)((colour >> 8) & 0xff) / 255.0f;
    bCol = (float)(colour & 0xff) / 255.0f;
    setMiscTex(0);
    setSize(0.01f, 0.01f);
    size *= random.nextFloat() * 0.6f + 0.2f;
    xd = xa * 0.0;
    yd = ya * 0.0;
    zd = za * 0.0;
    lifetime = (int)(16.0f / (random.nextDouble() * 0.8 + 0.2));
}

void SuspendedParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
    move(xd, yd, zd);
    if (((LevelSource*)level)->getBlockState(BlockPos(x, y, z))->getMaterial() != Material::WATER)
        remove();
    if (lifetime-- <= 0)
        remove();
}

uint64_t SuspendedParticle::GetType()
{
    return ParticleKind::SUSPENDED;
}

Particle* SuspendedParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                      double xa, double ya, double za, arrayWithLength<int> params)
{
    return new SuspendedParticle(level, x, y, z, xa, ya, za);
}

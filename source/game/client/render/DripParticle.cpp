#include "game/client/render/DripParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"

DripParticle::DripParticle(Level* level, double x, double y, double z, Material* material)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    Material* water = Material::WATER;
    ColourTable* colourTable = Minecraft::GetInstance()->getColourTable();
    unsigned int colour;
    if (material == water)
        colour = colourTable->getColour(eMinecraftColour_Particle_DripWater);
    else
        colour = colourTable->getColour(eMinecraftColour_Particle_DripLavaStart);
    rCol = (float)((colour >> 16) & 255) / 255.0f;
    gCol = (float)((colour >> 8) & 255) / 255.0f;
    bCol = (float)(colour & 255) / 255.0f;
    setMiscTex(113);
    setSize(0.01f, 0.01f);
    gravity = 0.06f;
    this->material = material;
    delay = 40;
    lifetime = (int)(64.0 / (random.nextDouble() * 0.8 + 0.2));
    xd = 0.0;
    yd = 0.0;
    zd = 0.0;
}

int DripParticle::getLightColor(float partialTicks)
{
    if (material != Material::WATER)
        return 0xf000f0;
    return Particle::getLightColor(partialTicks);
}

float DripParticle::getBrightness(float partialTicks)
{
    if (material == Material::WATER)
        return Particle::getBrightness(partialTicks);
    return 1.0f;
}

void DripParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    Material* dripMaterial = material;
    Material* water = Material::WATER;
    ColourTable* colourTable = Minecraft::GetInstance()->getColourTable();
    if (dripMaterial == water)
    {
        unsigned int colour = colourTable->getColour(eMinecraftColour_Particle_DripWater);
        rCol = (float)((colour >> 16) & 255) / 255.0f;
        gCol = (float)((colour >> 8) & 255) / 255.0f;
        bCol = (float)(colour & 255) / 255.0f;
    }
    else
    {
        unsigned int start = colourTable->getColour(eMinecraftColour_Particle_DripLavaStart);
        unsigned int end = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Particle_DripLavaEnd);
        float startRed = (float)((start >> 16) & 255) / 255.0f;
        float startGreen = (float)((start >> 8) & 255) / 255.0f;
        float startBlue = (float)(start & 255) / 255.0f;
        float endRed = (float)((end >> 16) & 255) / 255.0f;
        float endGreen = (float)((end >> 8) & 255) / 255.0f;
        float endBlue = (float)(end & 255) / 255.0f;
        float age = (float)(40 - delay);
        rCol = startRed - (startRed - endRed) / 40.0f * age;
        gCol = startGreen - (startGreen - endGreen) / 40.0f * age;
        bCol = startBlue - (startBlue - endBlue) / 40.0f * age;
    }

    yd -= gravity;
    if (delay-- > 0)
    {
        xd *= 0.02;
        yd *= 0.02;
        zd *= 0.02;
        setMiscTex(113);
    }
    else
    {
        setMiscTex(112);
    }
    move(xd, yd, zd);
    xd *= 0.98f;
    yd *= 0.98f;
    zd *= 0.98f;

    if (lifetime-- < 1)
        remove();

    if (onGround)
    {
        if (material == Material::WATER)
        {
            remove();
            level->addParticle(PARTICLE_SPLASH, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
            xd *= 0.7f;
            zd *= 0.7f;
        }
        else
        {
            setMiscTex(114);
            xd *= 0.7f;
            zd *= 0.7f;
        }
    }

    BlockPos position(x, y, z);
    const BlockState* state = level->getBlockState(position);
    Material* stateMaterial = state->getBlock()->mMaterial;
    if (stateMaterial->isLiquid() || stateMaterial->isSolid())
    {
        double height = 0.0;
        if (state->getBlock()->isLiquidBlock())
        {
            int data = state->getValue(LiquidBlock::LEVEL);
            height = LiquidBlock::getHeightFromData(data);
        }
        double surface = Mth::floor(y) + 1.0 - height;
        if (y < surface)
            remove();
    }
}

uint64_t DripParticle::GetType()
{
    return ParticleKind::DRIP;
}

Particle* DripParticle::LavaProvider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                     double xa, double ya, double za, arrayWithLength<int> params)
{
    return new DripParticle(level, x, y, z, Material::LAVA);
}

Particle* DripParticle::WaterProvider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                      double xa, double ya, double za, arrayWithLength<int> params)
{
    return new DripParticle(level, x, y, z, Material::WATER);
}

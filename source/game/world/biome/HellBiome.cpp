#include "game/world/biome/HellBiome.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/biome/HellBiomeDecorator.hpp"

HellBiome::HellBiome(EBiomeIDs biomeId, BiomeProperties* properties) : Biome(biomeId, eBiomeType_Hell, properties)
{
    mMonsters.clear();
    mCreatures.clear();
    mChickens.clear();
    mWolves.clear();
    mWaterCreatures.clear();
    mAmbientCreatures.clear();

    mMonsters.push_back(new MobSpawnerData(100, EntityType::GHAST, 4, 4));
    mMonsters.push_back(new MobSpawnerData(200, EntityType::PIG_ZOMBIE, 4, 4));
    mMonsters.push_back(new MobSpawnerData(1, EntityType::ENDER_MAN, 4, 4));
    mMonsters.push_back(new MobSpawnerData(1, EntityType::WITHER_SKELETON, 4, 4));
    mSlimes.push_back(new MobSpawnerData(2, EntityType::MAGMA_CUBE, 4, 4));

    delete mDecorator;
    mDecorator = new HellBiomeDecorator();
}

Vec3* HellBiome::getFogColor(float celestialAngle, float partialTicks) const
{
    unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Nether_Fog_Colour);
    return Vec3::newTemp((double)(((colour >> 16) & 0xFF) * 0.00390625F),
                         (double)(((colour >> 8) & 0xFF) * 0.00390625F), (double)((colour & 0xFF) * 0.00390625F));
}

bool HellBiome::isFoggy()
{
    return true;
}

bool HellBiome::isNatural()
{
    return false;
}

unsigned int HellBiome::getSkyColor(float temperature)
{
    return 0;
}

Biome::EBiomeBaseClass HellBiome::getBaseClass()
{
    return eBiomeBaseClass_Hell;
}

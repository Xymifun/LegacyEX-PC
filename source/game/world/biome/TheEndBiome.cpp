#include "game/world/biome/TheEndBiome.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/biome/TheEndBiomeDecorator.hpp"
#include "game/world/block/Blocks.hpp"

TheEndBiome::TheEndBiome(EBiomeIDs biomeId, BiomeProperties* properties) : Biome(biomeId, eBiomeType_TheEnd, properties)
{
    mMonsters.clear();
    mCreatures.clear();
    mChickens.clear();
    mWolves.clear();
    mWaterCreatures.clear();
    mAmbientCreatures.clear();

    mMonsters.push_back(new MobSpawnerData(10, EntityType::ENDER_MAN, 4, 4));

    mTopBlock = BlockPair(BlockId::DIRT,
                          Blocks::dirt->convertBlockStateToLegacyData(Blocks::dirt->defaultBlockState()));
    mFillerBlock = BlockPair(BlockId::DIRT,
                             Blocks::dirt->convertBlockStateToLegacyData(Blocks::dirt->defaultBlockState()));

    mDecorator = new TheEndBiomeDecorator();
}

unsigned int TheEndBiome::getSkyColor(float temperature)
{
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Sky_Sky);
}

Vec3* TheEndBiome::getFogColor(float celestialAngle, float partialTicks) const
{
    int fogColour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_End_Fog_Colour);

    float brightness = Mth::cos(celestialAngle * 3.1415927F * 2.0F) * 2.0F + 0.5F;
    if (brightness < 0.0F)
        brightness = 0.0F;
    else if (brightness > 1.0F)
        brightness = 1.0F;

    float red = (float)(fogColour >> 16 & 0xFF) / 255.0F;
    float green = (float)(fogColour >> 8 & 0xFF) / 255.0F;
    float blue = (float)(fogColour & 0xFF) / 255.0F;
    red *= brightness * 0.0F + 0.15F;
    green *= brightness * 0.0F + 0.15F;
    blue *= brightness * 0.0F + 0.15F;

    return Vec3::newTemp(red, green, blue);
}

Biome::EBiomeBaseClass TheEndBiome::getBaseClass()
{
    return eBiomeBaseClass_TheEnd;
}

bool TheEndBiome::isNatural()
{
    return false;
}

#include "game/world/biome/VoidBiome.hpp"

#include "game/world/biome/VoidBiomeDecorator.hpp"

VoidBiome::VoidBiome(EBiomeIDs biomeId, BiomeProperties* properties) : Biome(biomeId, eBiomeType_Void, properties)
{
    mMonsters.clear();
    mSlimes.clear();
    mCreatures.clear();
    mChickens.clear();
    mWaterCreatures.clear();
    mAmbientCreatures.clear();

    delete mDecorator;
    mDecorator = new VoidBiomeDecorator();
}

Biome::EBiomeBaseClass VoidBiome::getBaseClass()
{
    return eBiomeBaseClass_Void;
}

bool VoidBiome::isAlwaysValidSpawn()
{
    return true;
}

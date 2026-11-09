#include "game/world/generation/LargeFeature.hpp"

#include "game/world/Level.hpp"

const std::wstring LargeFeature::STRONGHOLD(L"StrongHold");
const std::wstring LargeFeature::VILLAGE(L"Village");
const std::wstring LargeFeature::MONUMENT(L"Monument");
const std::wstring LargeFeature::MANSION(L"Mansion");
const std::wstring LargeFeature::END_CITY(L"EndCity");
const std::wstring LargeFeature::TEMPLE(L"Temple");
const std::wstring LargeFeature::MINESHAFT(L"Mineshaft");
const std::wstring LargeFeature::FORTRESS(L"Fortress");
const std::wstring LargeFeature::BURIED_TREASURE(L"BuriedTreasure");
const std::wstring LargeFeature::SHIPWRECK(L"Shipwreck");
const std::wstring LargeFeature::RUIN(L"Ruin");

LargeFeature::LargeFeature()
{
    range = 8;
    random = Random();
}

LargeFeature::~LargeFeature()
{
}

void LargeFeature::apply(Level* level, int chunkX, int chunkZ, ChunkPrimer* primer)
{
    this->level = level;

    const int featureRange = range;

    random.setSeed(level->getSeed());

    const long long xSeedFactor = random.nextLong();
    const long long zSeedFactor = random.nextLong();

    for (int x = chunkX - featureRange; x <= chunkX + featureRange; ++x)
    {
        for (int z = chunkZ - featureRange; z <= chunkZ + featureRange; ++z)
        {
            long long xPart = xSeedFactor * x;
            long long zPart = zSeedFactor * z;
            random.setSeed(xPart ^ zPart ^ level->getSeed());
            addFeature(level, x, z, chunkX, chunkZ, primer);
        }
    }
}

void LargeFeature::addFeature(Level* level, int chunkX, int chunkZ, int originChunkX, int originChunkZ,
                              ChunkPrimer* primer)
{
}

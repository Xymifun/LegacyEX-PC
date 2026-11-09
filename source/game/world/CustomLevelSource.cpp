#include "game/world/CustomLevelSource.hpp"

#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/Level.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/LargeFeature.hpp"
#include "game/world/generation/StrongholdFeature.hpp"
#include "game/world/generation/StructureFeature.hpp"
#include "game/entity/mob/MobCategory.hpp"

CustomLevelSource::~CustomLevelSource()
{
}

void CustomLevelSource::prepareHeights(int, int, ChunkPrimer*)
{
}

LevelChunk* CustomLevelSource::createChunk(int, int)
{
    return NULL;
}

void CustomLevelSource::lightChunk(LevelChunk*)
{
}

void CustomLevelSource::postProcess(int, int)
{
}

bool CustomLevelSource::postProcessLoadedChunk(LevelChunk*, int, int)
{
    return false;
}

std::vector<Biome::MobSpawnerData*>* CustomLevelSource::getMobsAt(MobCategory*, const BlockPos&)
{
    return NULL;
}

bool CustomLevelSource::isPosInFeature(Level*, const std::wstring& feature, const BlockPos& pos)
{
    if (!mGenerateStructures)
        return false;
    if (LargeFeature::STRONGHOLD == feature)
    {
        if (mStronghold != NULL)
            return mStronghold->isInsideFeature(pos);
    }
    if (LargeFeature::SHIPWRECK == feature)
    {
        if (mShipwreck != NULL)
            return mShipwreck->isInsideFeature(pos);
    }
    if (LargeFeature::MANSION == feature)
    {
        if (mMansion != NULL)
            return mMansion->isInsideFeature(pos);
    }
    if (LargeFeature::MONUMENT == feature)
    {
        if (mMonument != NULL)
            return mMonument->isInsideFeature(pos);
    }
    if (LargeFeature::VILLAGE == feature)
    {
        if (mVillage != NULL)
            return mVillage->isInsideFeature(pos);
    }
    if (LargeFeature::MINESHAFT == feature)
    {
        if (mMineshaft != NULL)
            return mMineshaft->isInsideFeature(pos);
    }
    if (LargeFeature::TEMPLE == feature)
    {
        if (mTemple != NULL)
            return mTemple->isInsideFeature(pos);
    }
    if (LargeFeature::RUIN == feature)
    {
        if (mOceanRuin != NULL)
            return mOceanRuin->isInsideFeature(pos);
    }
    return false;
}

BlockPos* CustomLevelSource::findNearestMapFeature(Level*, const std::wstring&, const BlockPos&, bool)
{
    return NULL;
}

void CustomLevelSource::recreateLogicStructuresForChunk(LevelChunk*, int, int)
{
}

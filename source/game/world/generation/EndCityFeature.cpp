#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/world/generation/EndCityFeature.hpp"

#include <algorithm>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/Rotation.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/world/Level.hpp"
#include "game/world/TheEndDimension.hpp"
#include "game/world/TheEndLevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/core/EndCityPieces.hpp"


EndCityFeature::EndCityFeature(TheEndLevelSource* source) : StructureFeature(), mSource(source)
{
}

std::wstring EndCityFeature::getFeatureName()
{
    return L"EndCity";
}

bool EndCityFeature::isFeatureChunk(int chunkX, int chunkZ, bool)
{
    ChunkPos* area = TheEndDimension::closestAreaToPoint(chunkX, chunkZ);
    if (area->x == TheEndDimension::AREAS[1].x && area->z == TheEndDimension::AREAS[1].z)
    {
        return chunkX == area->x && chunkZ == area->z;
    }
    if (area->x == TheEndDimension::AREAS[0].x && area->z == TheEndDimension::AREAS[0].z)
    {
        return false;
    }
    Random& chunkRandom = *level->getRandomFor(area->x, area->z, 10387313);
    return chunkX == area->x && chunkZ == area->z && chunkRandom.nextDouble() < 0.33;
}

int EndCityFeature::getYPositionForFeature(int chunkX, int chunkZ, TheEndLevelSource* source)
{
    int seed = chunkX + chunkZ * 10387313;
    Random random(seed);
    Rotation* rotation = Rotation::VALUES[random.nextInt(4)];
    arrayWithLength<unsigned short> elements(65536, true);
    ChunkPrimer primer(true, elements);
    source->prepareHeights(chunkX, chunkZ, &primer);
    int xOffset = 5;
    int zOffset = 5;
    if (rotation == Rotation::CLOCKWISE_90)
    {
        xOffset = -5;
    }
    else if (rotation == Rotation::CLOCKWISE_180)
    {
        xOffset = -5;
        zOffset = -5;
    }
    else if (rotation == Rotation::COUNTERCLOCKWISE_90)
    {
        zOffset = -5;
    }
    int height0 = primer.getHighestNonAirPos(7, 7);
    int height1 = primer.getHighestNonAirPos(7, zOffset + 7);
    int height2 = primer.getHighestNonAirPos(xOffset + 7, 7);
    int height3 = primer.getHighestNonAirPos(xOffset + 7, zOffset + 7);
    return std::min(std::min(height0, height1), std::min(height2, height3));
}

StructureStart* EndCityFeature::createStructureStart(int chunkX, int chunkZ)
{
    return new EndCityStart(level, mSource, random, chunkX, chunkZ);
}

BlockPos* EndCityFeature::getNearestGeneratedFeature(Level* levelIn, const BlockPos& pos, bool skipExistingChunks)
{
    level = levelIn;
    return StructureFeature::findNearestFeaturePositionBySpacing(levelIn, this, pos, 10, 4, 10387313, true, 100,
                                                                 skipExistingChunks);
}

EndCityFeature::EndCityStart::EndCityStart(Level* levelIn, TheEndLevelSource* source, Random& random, int chunkX,
                                           int chunkZ)
    : StructureStart(chunkX, chunkZ)
{
    gApp.AddTerrainFeaturePosition((_eTerrainFeatureType)8, (chunkX * 16 + 8) / 16, (chunkZ * 16 + 8) / 16);
    create(levelIn, source, random, chunkX, chunkZ);
}

void EndCityFeature::EndCityStart::create(Level* levelIn, TheEndLevelSource* source, Random& random, int chunkX,
                                          int chunkZ)
{
    Rotation* rotation = Rotation::getRandom(random);
    int y = EndCityFeature::getYPositionForFeature(chunkX, chunkZ, source);
    BlockPos position(chunkX * 16 + 8, y, chunkZ * 16 + 8);
    EndCityPieces::startHouseTower(levelIn->getLevelStorage()->getStructureManager(), position, rotation, mPieces,
                                   random);
    calculateBoundingBox();
    mValid = true;
}

bool EndCityFeature::EndCityStart::isValid()
{
    return mValid;
}

EStructureStart EndCityFeature::EndCityStart::GetType()
{
    return eStructureStart_EndCity;
}

#include "game/world/generation/StructureFeature.hpp"

#include <typeinfo>

#include "game/core/BoundingBox.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/data/ByteArrayTag.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/SavedData.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelType.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"
#include "game/world/generation/StructureFeatureSavedData.hpp"
#include "game/world/generation/StructureStart.hpp"

BoundingBox* StructureStart::getBoundingBox()
{
    return mBoundingBox;
}

bool StructureStart::isDirty()
{
    return mDirty;
}

unsigned int StructureStart::getChunkX()
{
    return (unsigned int)mChunkX;
}

unsigned int StructureStart::getChunkZ()
{
    return (unsigned int)mChunkZ;
}

std::vector<StructurePiece*>* StructureStart::getPieces()
{
    return &mPieces;
}

void StructureStart::clearDirty()
{
    mDirty = false;
}

StructureFeature::StructureFeature()
{
    mSavedData = boost::shared_ptr<StructureFeatureSavedData>();
}

StructureFeature::~StructureFeature()
{
    for (boost::unordered_map<long long, StructureStart*>::iterator it = mStructureMap.begin();
         it != mStructureMap.end(); ++it)
    {
        delete it->second;
    }
}

void StructureFeature::addFeature(Level* level, int chunkX, int chunkZ, int originChunkX, int originChunkZ,
                                  ChunkPrimer* primer)
{
    restoreSavedData(level);

    long long key = ChunkPos::asLong(chunkX, chunkZ);
    if (mStructureMap.find(key) != mStructureMap.end())
    {
        return;
    }

    random.nextInt();
    if (isFeatureChunk(chunkX, chunkZ, level->getLevelData()->getGeneratorType() == LevelType::FLAT))
    {
        StructureStart* start = createStructureStart(chunkX, chunkZ);
        mStructureMap[ChunkPos::asLong(chunkX, chunkZ)] = start;
        if (start->isValid())
        {
            saveFeature(chunkX, chunkZ, start);
        }
    }
}

bool StructureFeature::postProcess(Level* level, Random& randomSource, ChunkPos* chunkPos)
{
    restoreSavedData(level);
    int minX = chunkPos->x * 16;
    int minZ = chunkPos->z * 16;
    bool processed = false;
    for (boost::unordered_map<long long, StructureStart*>::iterator it = mStructureMap.begin();
         it != mStructureMap.end(); ++it)
    {
        StructureStart* start = it->second;
        if (start->isValid() && start->isValidForPostProcess(chunkPos)
            && start->getBoundingBox()->intersects(minX, minZ, minX + 15, minZ + 15))
        {
            BoundingBox box(minX, minZ, minX + 15, minZ + 15);
            start->postProcess(level, randomSource, &box);
            start->notifyPostProcessAt(chunkPos);
            processed = true;
            if (start->isDirty())
            {
                saveFeature(start->getChunkX(), start->getChunkZ(), start);
            }
        }
    }
    return processed;
}

bool StructureFeature::isIntersection(int x, int z)
{
    restoreSavedData(level);
    for (boost::unordered_map<long long, StructureStart*>::iterator it = mStructureMap.begin();
         it != mStructureMap.end(); ++it)
    {
        StructureStart* start = it->second;
        if (start->isValid() && start->getBoundingBox()->intersects(x, z, x, z))
        {
            std::vector<StructurePiece*>::iterator it2 = start->getPieces()->begin();
            while (it2 != start->getPieces()->end())
            {
                StructurePiece* next = *it2++;
                if (next->getBoundingBox()->intersects(x, z, x, z))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

StructureStart* StructureFeature::getStructureAt(const BlockPos& pos)
{
    for (boost::unordered_map<long long, StructureStart*>::iterator it = mStructureMap.begin();
         it != mStructureMap.end(); ++it)
    {
        StructureStart* start = it->second;
        if (start->isValid())
        {
            if (start->getBoundingBox()->isInside(pos))
            {
                std::vector<StructurePiece*>* pieces = start->getPieces();
                for (std::vector<StructurePiece*>::iterator piece = pieces->begin(); piece != pieces->end(); piece++)
                {
                    if ((*piece)->getBoundingBox()->isInside(pos))
                    {
                        return start;
                    }
                }
            }
        }
    }
    return NULL;
}

bool StructureFeature::isInsideFeature(const BlockPos& pos)
{
    if (level == NULL)
    {
        return false;
    }
    restoreSavedData(level);
    return getStructureAt(pos) != NULL;
}

bool StructureFeature::isInsideBoundingFeature(Level* level, const BlockPos& pos)
{
    restoreSavedData(level);
    for (boost::unordered_map<long long, StructureStart*>::iterator it = mStructureMap.begin();
         it != mStructureMap.end(); ++it)
    {
        StructureStart* start = it->second;
        if (start->isValid())
        {
            if (start->getBoundingBox()->isInside(pos))
            {
                return true;
            }
        }
    }
    return false;
}

std::vector<BlockPos>* StructureFeature::getGuesstimatedFeaturePositions()
{
    return NULL;
}

void StructureFeature::restoreSavedData(Level* levelToRestore)
{
    if (mSavedData || levelToRestore == NULL)
    {
        return;
    }

    mSavedData = boost::dynamic_pointer_cast<StructureFeatureSavedData>(
        levelToRestore->getSavedData(typeid(StructureFeatureSavedData), getFeatureName()));
    if (!mSavedData)
    {
        mSavedData = boost::shared_ptr<StructureFeatureSavedData>(new StructureFeatureSavedData(getFeatureName()));
        levelToRestore->setSavedData(getFeatureName(), mSavedData);
    }
    else
    {
        CompoundTag* tags = mSavedData->getFullTag();
        boost::unordered_map<std::wstring, Tag*>* entries = tags->getAllTagsMap();
        for (boost::unordered_map<std::wstring, Tag*>::iterator it = entries->begin(); it != entries->end(); it++)
        {
            Tag* tag = it->second;
            if (tag->getId() == Tag::TAG_BYTE_ARRAY)
            {
                StructureStart* start = StructureFeatureIO::loadStaticStart(static_cast<ByteArrayTag*>(tag),
                                                                            levelToRestore);
                if (start != NULL)
                {
                    mStructureMap[ChunkPos::asLong(start->getChunkX(), start->getChunkZ())] = start;
                }
            }
        }
    }
}
void StructureFeature::saveFeature(int chunkX, int chunkZ, StructureStart* start)
{
    mSavedData->putFeatureTag(start->createTag(chunkX, chunkZ), chunkX, chunkZ);
    mSavedData->setDirty();
    start->clearDirty();
}

BlockPos* StructureFeature::findNearestFeaturePositionBySpacing(Level* levelToSearch, StructureFeature* feature,
                                                                const BlockPos& pos, int spacing, int separation,
                                                                int salt, bool skipKnownStructures, int maxDistance,
                                                                bool skipExistingChunks)
{
    int centerX = pos.x >> 4;
    int centerZ = pos.z >> 4;
    int halfSize = levelToSearch->getLevelData()->getXZSize() / 2;
    for (int radius = 0; radius <= maxDistance; ++radius)
    {
        for (int xOffset = -radius; xOffset <= radius; ++xOffset)
        {
            bool xEdge = xOffset == -radius || xOffset == radius;
            for (int zOffset = -radius; zOffset <= radius; ++zOffset)
            {
                bool zEdge = zOffset == -radius || zOffset == radius;
                if (!xEdge && !zEdge)
                    continue;

                int chunkX = centerX + xOffset;
                int chunkZ = centerZ + zOffset;
                bool flatWorld = levelToSearch->getLevelData()->getGeneratorType() == LevelType::FLAT;
                if (!feature->isFeatureChunk(chunkX, chunkZ, flatWorld))
                {
                    if (radius == 0)
                        break;
                    continue;
                }
                if (chunkX < -halfSize || chunkX > halfSize || chunkZ < -halfSize || chunkZ > halfSize)
                    continue;
                if (skipExistingChunks && levelToSearch->isChunkGeneratedAt(chunkX, chunkZ))
                    continue;
                return new BlockPos(chunkX * 16 + 8, 64, chunkZ * 16 + 8);
            }
        }
    }
    return NULL;
}

bool StructureFeature::canPlaceFeatureInChunk(bool defaultValue, EFeatureTypes type, int chunkX, int chunkZ)
{
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options != NULL && options->hasFeature(type) && chunkX > -27 && chunkZ < 27)
    {
        bool result = options->isFeatureChunk(chunkX, chunkZ, type, NULL);
        return result;
    }
    return defaultValue;
}

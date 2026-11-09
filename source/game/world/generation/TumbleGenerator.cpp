#include "game/world/generation/TumbleGenerator.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockId.hpp"

#include <string>
#include <utility>
#include <boost/unordered_set.hpp>

#include "game/core/GenerationWriteTask.hpp"
#include "game/core/System.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockTransaction.hpp"
#include "game/world/block/Iterator.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/LayerGenerator.hpp"
#include "game/core/PIXNamedEvents.hpp"

TumbleGenerator::TumbleGenerator(Random random, std::vector<LayerGenerator*>& layers)
    : MapGenerator(random),
      mLayers(layers)
{
}

TumbleGenerator::~TumbleGenerator()
{
    std::vector<LayerGenerator*>::iterator end = mLayers.end();
    for (std::vector<LayerGenerator*>::iterator layer = mLayers.begin(); layer != end; ++layer)
    {
        delete *layer;
    }
}

AABB* TumbleGenerator::GetSpawnArea(unsigned int layer) const
{
    AABB* area = AABB::newTemp(mLayers[layer]->getArea());
    area->mMinY = static_cast<double>(mLayers[layer]->getCoreWorldMinY());
    area->mMaxY = area->mMinY + 5.0;
    return area;
}

void TumbleGenerator::AddAdditionalBlock(BlockPos position)
{
    mAdditionalBlocks.push_back(position);
}

void TumbleGenerator::StartGeneration(Level* level)
{
    System::processTimeInMilliSecs();
    int totalOffset = 0;
    std::vector<std::pair<LayerGenerator*, BlockTransaction*> > transactions;
    std::wstring(L"mapgen_perf.pix3");

    std::vector<LayerGenerator*>::iterator end = mLayers.end();
    for (std::vector<LayerGenerator*>::iterator layer = mLayers.begin(); layer != end; ++layer)
    {
        LayerGenerator* generator = *layer;
        BlockTransaction* transaction = generator->Generate(mRandom, level);

        AABB* area = AABB::newPermanent(generator->getArea());
        area->mMaxY += static_cast<float>(totalOffset);
        area->mMinY += static_cast<float>(totalOffset);
        generator->updateArea(area);

        unsigned int layerCount = static_cast<unsigned int>(mLayers.size());
        const int minimumLayerHeights[] = {4, 4, 4, 4, 3, 3, 2, 2, 1};
        if (layerCount > 9)
        {
            layerCount = 9;
        }

        Vec3i minimum = transaction->getMin();
        totalOffset += minimum.y - minimumLayerHeights[layerCount];
        transactions.push_back(std::make_pair(generator, transaction));
        mBlockCounts.push_back(0);
    }

    const int centreOffset = totalOffset / 2;
    unsigned int layerIndex = 0;
    for (std::vector<std::pair<LayerGenerator*, BlockTransaction*> >::iterator transaction = transactions.begin(),
                                                                               transactionsEnd = transactions.end();
         transaction != transactionsEnd; ++transaction)
    {
        LayerGenerator* generator = transaction->first;
        BlockTransaction* blockTransaction = transaction->second;
        AABB* area = AABB::newPermanent(generator->getArea());
        area->mMaxY -= static_cast<float>(centreOffset);
        area->mMinY -= static_cast<float>(centreOffset);
        generator->updateArea(area);

        Vec3i originPosition(static_cast<int>(area->mMinX), static_cast<int>(area->mMinY),
                             static_cast<int>(area->mMinZ));
        mGenerationWriteTasks.push_back(
            new GenerationWriteTask(level, blockTransaction, BlockPos(originPosition), &mBlockCounts[layerIndex]));

        ++layerIndex;
    }

    System::processTimeInMilliSecs();
}

AABB* TumbleGenerator::GetSpawnArea() const
{
    return GetSpawnArea(0);
}

void TumbleGenerator::CleanUp(Level* level)
{
    std::wstring(L"cleanup_perf.pix3");
    boost::unordered_set<LevelChunk*> dirtyChunks;

    std::vector<LayerGenerator*>::iterator end = mLayers.end();
    for (std::vector<LayerGenerator*>::iterator layer = mLayers.begin(); layer != end; ++layer)
    {
        AABB* area = (*layer)->getArea();
        if (area != NULL)
        {
            PIXBeginNamedEvent(0.0f, "TumbleGenerator::CleanUpLayer::WipeData");
            BlockPos minimum(area->mMinX, area->mMinY, area->mMinZ);
            BlockPos maximum(area->mMaxX, area->mMaxY, area->mMaxZ);
            Iterator<BlockPos*>* positions = BlockPos::between(minimum, maximum);
            BlockPos* position = positions->next();
            while (positions->hasNext())
            {
                level->setBlockAndData(position->x, position->y, position->z, BlockId::AIR, 0,
                                       Block::UPDATE_CLIENTS | Block::UPDATE_LIGHT_IF_CHANGED);
                dirtyChunks.insert(level->getChunkAt(position->x, position->z));
                positions->next();
            }
            PIXEndNamedEvent();
        }
    }

    mBlockCounts.clear();

    for (unsigned int index = 0; index < mAdditionalBlocks.size(); ++index)
    {
        BlockPos& additional = mAdditionalBlocks[index];
        level->setBlockAndData(additional.x, additional.y, additional.z, BlockId::AIR, 0,
                               Block::UPDATE_CLIENTS | Block::UPDATE_LIGHT_IF_CHANGED);
        dirtyChunks.insert(level->getChunkAt(additional.x, additional.z));
    }
    mAdditionalBlocks.clear();

    PIXBeginNamedEvent(0.0f, "TumbleGenerator::CleanUpLayer::Relight");
    boost::unordered_set<LevelChunk*>::iterator chunk = dirtyChunks.begin();
    while (chunk != dirtyChunks.end())
    {
        LevelChunk* levelChunk = *chunk;
        levelChunk->recalcHeightmapOnly();
        levelChunk->lightLava();
        ++chunk;
    }
    PIXEndNamedEvent();
}

AABB* TumbleGenerator::GetLayerBB(unsigned int layer)
{
    if (layer >= mLayers.size() || layer > 8)
    {
        return NULL;
    }
    return mLayers[layer]->getArea();
}

int TumbleGenerator::GetNumberOfLayers()
{
    return static_cast<int>(mLayers.size());
}

void TumbleGenerator::GetLayerExtents(unsigned int layer, float* minimum, float* maximum)
{
    if (layer >= mLayers.size() || layer > 8)
    {
        return;
    }
    LayerGenerator*& generator = mLayers[layer];
    *minimum = static_cast<float>(generator->getCoreWorldMinY());
    *maximum = static_cast<float>(generator->getCoreWorldMaxY());
}

unsigned int TumbleGenerator::GetBlockCountOnLayer(unsigned int layer)
{
    if (layer >= mBlockCounts.size())
    {
        return 0;
    }
    return mBlockCounts[layer];
}

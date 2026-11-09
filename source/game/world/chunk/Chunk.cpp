#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/world/chunk/Chunk.hpp"

#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/BlockEntityRenderDispatcher.hpp"
#include "game/client/render/BlockRenderer.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Culler.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Region.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/minigame/TargetAreaRuleDefinition.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/GlideRingGenerator.hpp"
#include "game/core/PIXNamedEvents.hpp"

#include <algorithm>
#include <climits>

extern LevelRenderer* gLevelRenderer;

static Tesselator* t;
static int updates = 0;

Chunk::Chunk(Level* level, ChunkBlockEntityMap& blockEntities, OSFastMutex& mutex, int x, int z, int dimension,
             ClipChunk* clipChunk)
    : mLevel(level),
      mX(0),
      mZ(0),
      mDimension(0),
      mDirty(0),
      mEmpty(0),
      mRenderable(0),
      mMinX(0),
      mMinZ(0),
      mMinDimension(0),
      mMaxX(0),
      mMaxZ(0),
      mMaxDimension(0),
      mBoundingBox(NULL),
      mOwnsBoundingBox(false),
      mClipChunk(clipChunk),
      mUnknown(0),
      mBlockEntities(&blockEntities),
      mMutex(&mutex),
      mNonStack(false)
{
    mClipChunk->mVisible = false;
    mClipChunk->mChunk = this;
    setPos(x, z, dimension);
    mNonStack = true;
}

Chunk::Chunk() : mOwnsBoundingBox(false)
{
}

Chunk::~Chunk()
{
    if (mOwnsBoundingBox)
        delete mBoundingBox;
}

void Chunk::reset()
{
    if (mNonStack)
    {
        EnterCriticalSection(&gLevelRenderer->mChunkMutex);
        unsigned char references = gLevelRenderer->decGlobalChunkRefCount(mX, mZ, mDimension, mLevel);
        mNonStack = false;
        if (references == 0)
        {
            int list = LevelRenderer::getGlobalIndexForChunk(mX, mZ, mDimension, mLevel) * 3;
            if (list >= 0)
            {
                list += gLevelRenderer->mDisplayList;
                for (int i = 0; i < 3; ++i)
                    gRenderer->CBuffClear(list + i);
                gLevelRenderer->setGlobalChunkFlags(mX, mZ, mDimension, mLevel, 0);
            }
        }
        LeaveCriticalSection(&gLevelRenderer->mChunkMutex);
    }
    mClipChunk->mVisible = false;
}

void Chunk::setPos(int x, int z, int dimension)
{
    if (!mNonStack || x != mX || z != mZ || dimension != mDimension)
    {
        reset();
        mX = x;
        mZ = z;
        mDimension = dimension;
        mMaxX = x + 8;
        mMaxZ = z + 8;
        mMaxDimension = dimension + 8;
        mClipChunk->mX = mMaxX;
        mClipChunk->mY = mMaxZ;
        mClipChunk->mZ = mMaxDimension;
        mClipChunk->mGlobalIndex = LevelRenderer::getGlobalIndexForChunk(x, z, dimension, mLevel);
        mMinX = x;
        mMinZ = z;
        mMinDimension = dimension;
        mDirty = 0;
        mEmpty = 0;
        mRenderable = 0;
        if (mBoundingBox == NULL)
            mBoundingBox = AABB::newPermanent(-6.0, -6.0, -6.0, 22.0, 22.0, 22.0);
        mClipChunk->mMinX = (float)(mBoundingBox->mMinX + x);
        mClipChunk->mMinY = (float)(mBoundingBox->mMinY + z);
        mClipChunk->mMinZ = (float)(mBoundingBox->mMinZ + dimension);
        mNonStack = true;
        mClipChunk->mMaxX = (float)(mBoundingBox->mMaxX + x);
        mClipChunk->mMaxY = (float)(mBoundingBox->mMaxY + z);
        mClipChunk->mMaxZ = (float)(mBoundingBox->mMaxZ + dimension);
        EnterCriticalSection(&gLevelRenderer->mChunkMutex);
        if (gLevelRenderer->incGlobalChunkRefCount(x, z, dimension, mLevel) == 1)
        {
            gLevelRenderer->setGlobalChunkFlag(x, z, dimension, mLevel, 2, 0);
            PIXSetMarkerDeprecated(0.0f, "Non-stack event pushed");
        }
        LeaveCriticalSection(&gLevelRenderer->mChunkMutex);
    }
}

void Chunk::translateToPos()
{
    GlStateManager::translatef((float)mMinX, (float)mMinZ, (float)mMinDimension);
}

unsigned int Chunk::elementToBlockId(unsigned short element)
{
    return (element >> 4) & 0xffff;
}

bool Chunk::isEmpty()
{
    return mEmpty != 0;
}

void Chunk::setDirty()
{
    mDirty = 1;
}

void Chunk::makeCopyForRebuild(Chunk* source)
{
    mLevel = source->mLevel;
    mX = source->mX;
    mZ = source->mZ;
    mDimension = source->mDimension;
    mDirty = source->mDirty;
    mEmpty = source->mEmpty;
    mRenderable = source->mRenderable;
    mMinX = source->mMinX;
    mMinZ = source->mMinZ;
    mMinDimension = source->mMinDimension;
    mMaxX = source->mMaxX;
    mMaxZ = source->mMaxZ;
    mMaxDimension = source->mMaxDimension;
    mBoundingBox = source->mBoundingBox;
    mClipChunk = source->mClipChunk;
    mUnknown = source->mUnknown;
    mBlockEntities = source->mBlockEntities;
    mMutex = source->mMutex;
}

void Chunk::rebuild(unsigned int* count)
{
    PIXBeginNamedEvent(0.0f, "Rebuilding chunk %d, %d, %d", mX, mZ, mDimension);
    PIXBeginNamedEvent(0.0f, "Rebuild section A");
    t = Tesselator::getInstance();
    BufferBuilder* builder = t->getBuilder();
    updates++;
    int x0 = mX;
    int y0 = mZ;
    int z0 = mDimension;
    int x1 = x0 + 16;
    int y1 = y0 + 16;
    int z1 = z0 + 16;
    LevelChunk::touchedSky = false;
    std::vector<boost::shared_ptr<BlockEntity> > renderableBlockEntities;
    int lists = LevelRenderer::getGlobalIndexForChunk(mX, mZ, mDimension, mLevel) * 3;
    if (lists >= 0)
    {
        lists += gLevelRenderer->mDisplayList;
        PIXEndNamedEvent();
        PIXBeginNamedEvent(0.0f, "Rebuild section B");

        static unsigned short cachedElements[65536];
        static unsigned short cachedExtraElements[65536];
        static const BlockState* cachedStates[18 * 16 * 16];
        arrayWithLength<unsigned short> elements(cachedElements, 65536);
        arrayWithLength<unsigned short> extraElements(cachedExtraElements, 65536);

        BlockPos chunkPos(mX, mZ, mDimension);
        int lowerSlice = std::max(0, y0 / 4 - 1);
        int upperSlice = std::min(64, y1 / 4 + 1);
        mLevel->getChunkAt(chunkPos)->getElementRange(elements, extraElements, lowerSlice, upperSlice);
        Region* region = new Region(mLevel, BlockPos(x0 - 1, y0 - 1, z0 - 1), BlockPos(x1 + 1, y1 + 1, z1 + 1), 1);
        region->setCachedBlocksAndData(cachedElements, mX >> 4, mZ >> 4, mDimension >> 4, cachedStates);
        PIXBeginNamedEvent(0.0f, "Creating BlockRenderer");
        BlockRenderer* blockRenderer = new BlockRenderer(region, mX, mZ, mDimension, count);
        PIXEndNamedEvent();

        bool empty = true;
        int yStart = std::max(0, y0 - 1);
        int yEnd = std::min(256, y1 + 1);
        for (int xz = 0; xz < 16 * 16; xz++)
        {
            for (int y = yStart; y < yEnd; y++)
            {
                unsigned int index = ((y & 0x80) << 8) | (y & 0x7f) | (xz << 7);
                unsigned short element = cachedElements[index];
                unsigned int id = elementToBlockId(element);
                if (id == BlockId::STONE || id == BlockId::GRASS || id == BlockId::DIRT || id == BlockId::BEDROCK
                    || id == BlockId::SAND || id == BlockId::GRAVEL || id == BlockId::GOLD_ORE
                    || id == BlockId::IRON_ORE || id == BlockId::COAL_ORE || id == BlockId::LAPIS_ORE
                    || id == BlockId::DIAMOND_ORE || id == BlockId::REDSTONE_ORE || id == BlockId::QUARTZ_ORE)
                    cachedElements[index] = element | 0x8000;
                if (empty && y >= y0 && y < y1 && element != 0)
                    empty = false;
            }
        }

        if (!empty)
        {
            int yLimit = y1 == 256 ? 255 : y1;
            for (int y = y0; y < yLimit; y++)
            {
                for (int z = 1; z < 15; z++)
                {
                    for (int x = 1; x < 15; x++)
                    {
                        unsigned int index = ((y & 0x80) << 8) | (y & 0x7f) | (x << 11) | (z << 7);
                        unsigned int above = (((y + 1) & 0x80) << 8) | ((y + 1) & 0x7f) | (x << 11) | (z << 7);
                        unsigned int below = (((y - 1) & 0x80) << 8) | ((y - 1) & 0x7f) | (x << 11) | (z << 7);
                        if ((cachedElements[index - 0x800] & 0x8000) && (cachedElements[index + 0x800] & 0x8000)
                            && (cachedElements[index - 0x80] & 0x8000) && (cachedElements[index + 0x80] & 0x8000)
                            && (y < 1 || (cachedElements[below] & 0x8000)) && (cachedElements[above] & 0x8000))
                            cachedElements[index] = 0xffff;
                    }
                }
            }
        }

        std::vector<BlockPos> ringPositions;
        std::vector<GlideRingGenerator::eGlideRingSize> ringSizes;
        if (Minecraft::InMiniGame(eMiniGameId_Glide, false))
        {
            LevelRuleset* ruleset = gApp.getGameRuleDefinitions();
            std::vector<TargetAreaRuleDefinition*> rings;
            AABB* chunkArea = AABB::newTemp(x0, y0, z0, x1, y1, z1);
            if (ruleset != NULL)
                ruleset->getRingsWithinArea(&rings, chunkArea);
            for (std::vector<TargetAreaRuleDefinition*>::iterator it = rings.begin(); it != rings.end(); ++it)
            {
                TargetAreaRuleDefinition* ring = *it;
                std::vector<BlockPos> positions;
                GlideRingGenerator::BuildRing(positions, ring->getArea(),
                                              static_cast<GlideRingGenerator::eGlideRingSize>(ring->getSize()));
                for (std::vector<BlockPos>::iterator pos = positions.begin(); pos != positions.end(); ++pos)
                {
                    if (chunkArea->containsIncludingLowerBound(Vec3::newTemp(pos->x, pos->y, pos->z)))
                    {
                        ringPositions.push_back(*pos);
                        ringSizes.push_back(static_cast<GlideRingGenerator::eGlideRingSize>(ring->getSize()));
                    }
                }
            }
            for (std::vector<BlockPos>::iterator pos = ringPositions.begin(); pos != ringPositions.end(); ++pos)
            {
                int y = pos->y;
                int upperHalf = 0;
                if (y >= 128)
                {
                    upperHalf = 0x8000;
                    y -= 128;
                }
                cachedElements[upperHalf + (((pos->x - x0) << 11) | ((pos->z - z0) << 7) | y)] = 0xffff;
            }
        }
        PIXEndNamedEvent();

        if (empty && ringPositions.size() == 0)
        {
            for (int pass = 0; pass < 3; pass++)
            {
                gLevelRenderer->setGlobalChunkFlag(mX, mZ, mDimension, mLevel, 4, pass);
                gRenderer->CBuffClear(lists + pass);
            }
            int key = LevelRenderer::getGlobalIndexForChunk(mX, mZ, mDimension, mLevel);
            EnterCriticalSection(mMutex);
            ChunkBlockEntityMap::iterator existing = mBlockEntities->find(key);
            if (existing != mBlockEntities->end())
            {
                for (std::vector<boost::shared_ptr<BlockEntity> >::iterator it = existing->second.begin();
                     it != existing->second.end(); ++it)
                    (*it)->setRenderRemoveStage(1);
            }
            LeaveCriticalSection(mMutex);
            delete region;
            delete blockRenderer;
            PIXEndNamedEvent();
        }
        else
        {
            PIXBeginNamedEvent(0.0f, "Rebuild section C");
            BufferBuilder::Bounds bounds;
            bounds.mMinX = bounds.mMinY = bounds.mMinZ = -6.0f;
            bounds.mMaxX = bounds.mMaxY = bounds.mMaxZ = 22.0f;
            MutableBlockPos pos(0, 0, 0);
            for (int pass = 0; pass < 3; pass++)
            {
                bool renderNextLayer = false;
                bool started = false;
                bool rendered = false;
                if (ringPositions.size() != 0)
                {
                    if (pass == 0)
                    {
                        renderNextLayer = true;
                    }
                    else if (pass == 1)
                    {
                        started = true;
                        MemSect(0x1f);
                        GlStateManager::newList(lists + pass, 0);
                        MemSect(0);
                        GlStateManager::pushMatrix();
                        GlStateManager::depthMask(true);
                        builder->useCompactVertices(true);
                        translateToPos();
                        builder->begin();
                        builder->offset((float)-mX, (float)-mZ, (float)-mDimension);
                        for (unsigned int ring = 0; ring < ringPositions.size(); ring++)
                        {
                            PIXBeginNamedEvent(0.0f, "Tesselate in world");
                            blockRenderer->setLightOverride(INT_MAX);
                            blockRenderer->tesselateInWorldFixedTexture(
                                Blocks::stained_glass->defaultBlockState(), ringPositions.at(ring),
                                GlideRingGenerator::getIcon(ringSizes.at(ring)));
                            blockRenderer->clearLightOverride();
                            PIXEndNamedEvent();
                        }
                        rendered = true;
                    }
                }
                for (int z = z0; z < z1; z++)
                {
                    for (int x = x0; x < x1; x++)
                    {
                        for (int y = y0; y < y1; y++)
                        {
                            unsigned int index = ((y & 0x80) << 8) | (y & 0x7f) | ((x - x0) << 11) | ((z - z0) << 7);
                            unsigned short element = cachedElements[index];
                            int blockId = elementToBlockId(element & 0x7fff);
                            unsigned short extraElement = cachedExtraElements[index];
                            if (extraElement != 0)
                                renderNextLayer = true;
                            bool useExtra = false;
                            if (pass == 2 && extraElement != 0)
                            {
                                blockId = elementToBlockId(extraElement);
                                useExtra = true;
                                element = extraElement;
                            }
                            if (element == 0xffff || blockId == 0)
                                continue;

                            pos.set(x, y, z);
                            Block* block = Block::byId(blockId);
                            if (pass == 0 && block->mBlockEntity)
                            {
                                boost::shared_ptr<BlockEntity> blockEntity = region->getBlockEntity(pos);
                                if (BlockEntityRenderDispatcher::instance->hasRenderer(blockEntity))
                                    renderableBlockEntities.push_back(blockEntity);
                            }
                            int renderLayer = block->getRenderLayer();
                            if (renderLayer > pass)
                            {
                                renderNextLayer = true;
                            }
                            else if (renderLayer == pass)
                            {
                                if (!started)
                                {
                                    started = true;
                                    MemSect(0x1f);
                                    GlStateManager::newList(lists + pass, 0);
                                    MemSect(0);
                                    GlStateManager::pushMatrix();
                                    GlStateManager::depthMask(true);
                                    builder->useCompactVertices(true);
                                    translateToPos();
                                    builder->begin();
                                    builder->offset((float)-mX, (float)-mZ, (float)-mDimension);
                                }
                                PIXBeginNamedEvent(0.0f, "Tesselate in world");
                                if (useExtra)
                                    rendered |= blockRenderer->tesselateInWorld(Block::getStateByElementId(element),
                                                                                pos, NULL,
                                                                                boost::shared_ptr<BlockEntity>());
                                else
                                    rendered |= blockRenderer->tesselateInWorld(region->getBlockState(pos), pos, NULL,
                                                                                boost::shared_ptr<BlockEntity>());
                                PIXEndNamedEvent();
                            }
                        }
                    }
                }

                if (started)
                {
                    builder->end();
                    bounds.addBounds(builder->mBounds);
                    GlStateManager::popMatrix();
                    GlStateManager::endList(0);
                    builder->useCompactVertices(false);
                    builder->offset(0.0f, 0.0f, 0.0f);
                }
                if (started && rendered)
                {
                    gLevelRenderer->clearGlobalChunkFlag(mX, mZ, mDimension, mLevel, 4, pass);
                }
                else
                {
                    gLevelRenderer->setGlobalChunkFlag(mX, mZ, mDimension, mLevel, 4, pass);
                    gRenderer->CBuffClear(lists + pass);
                }
                if (pass == 0 && !renderNextLayer)
                {
                    gLevelRenderer->setGlobalChunkFlag(mX, mZ, mDimension, mLevel, 0x18, 0);
                    gRenderer->CBuffClear(lists + 1);
                    gRenderer->CBuffClear(lists + 2);
                    break;
                }
                if (pass == 1 && !renderNextLayer)
                {
                    gLevelRenderer->setGlobalChunkFlag(mX, mZ, mDimension, mLevel, 0x10, 0);
                    gRenderer->CBuffClear(lists + 2);
                    break;
                }
            }
            if (mBoundingBox != NULL)
                mBoundingBox->set(bounds.mMinX, bounds.mMinY, bounds.mMinZ, bounds.mMaxX, bounds.mMaxY, bounds.mMaxZ);
            delete blockRenderer;
            delete region;
            PIXEndNamedEvent();

            PIXBeginNamedEvent(0.0f, "Rebuild section D");
            int key = LevelRenderer::getGlobalIndexForChunk(mX, mZ, mDimension, mLevel);
            EnterCriticalSection(mMutex);
            if (renderableBlockEntities.size() != 0)
            {
                ChunkBlockEntityMap::iterator existing = mBlockEntities->find(key);
                if (existing != mBlockEntities->end())
                {
                    for (std::vector<boost::shared_ptr<BlockEntity> >::iterator it = existing->second.begin();
                         it != existing->second.end(); ++it)
                        (*it)->setRenderRemoveStage(1);
                    for (unsigned int i = 0; i < renderableBlockEntities.size(); i++)
                    {
                        std::vector<boost::shared_ptr<BlockEntity> >::iterator previous = std::find(
                            existing->second.begin(), existing->second.end(), renderableBlockEntities[i]);
                        if (previous == existing->second.end())
                            (*mBlockEntities)[key].push_back(renderableBlockEntities[i]);
                        else
                            (*previous)->setRenderRemoveStage(0);
                    }
                }
                else
                {
                    for (unsigned int i = 0; i < renderableBlockEntities.size(); i++)
                    {
                        renderableBlockEntities[i]->shouldRemoveForRender();
                        (*mBlockEntities)[key].push_back(renderableBlockEntities[i]);
                    }
                }
            }
            else
            {
                ChunkBlockEntityMap::iterator existing = mBlockEntities->find(key);
                if (existing != mBlockEntities->end())
                {
                    for (std::vector<boost::shared_ptr<BlockEntity> >::iterator it = existing->second.begin();
                         it != existing->second.end(); ++it)
                        (*it)->setRenderRemoveStage(1);
                }
            }
            LeaveCriticalSection(mMutex);
            PIXEndNamedEvent();
            gLevelRenderer->setGlobalChunkFlag(mX, mZ, mDimension, mLevel, 1, 0);
            PIXEndNamedEvent();
        }
    }
}

float Chunk::distanceToSqr(boost::shared_ptr<Entity> entity) const
{
    float x = (float)(entity->mX - (double)mMaxX);
    float y = (float)(entity->mY - (double)mMaxZ);
    float z = (float)(entity->mZ - (double)mMaxDimension);
    return x * x + y * y + z * z;
}

void Chunk::_delete()
{
    reset();
    mLevel = NULL;
}

void Chunk::cull(Culler* culler)
{
    mClipChunk->mVisible = culler->isVisible(mBoundingBox);
}

void Chunk::clearDirty()
{
    gLevelRenderer->clearGlobalChunkFlag(mX, mZ, mDimension, mLevel, 2, 0);
}

#include "game/world/LevelEvent.hpp"
#include "game/world/block/Material.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/client/render/Particle.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "game/client/render/BlockEntityRenderDispatcher.hpp"
#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/BlockRenderer.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/Font.hpp"
#include "game/client/render/MobSkinMemTextureProcessor.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Frustum.hpp"
#include "game/core/FrustumCuller.hpp"
#include "game/core/FrustumData.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/InfoBarManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Options.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/DyeItem.hpp"
#include "game/item/ThrownPotion.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/System.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/projectile/DragonFireball.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/block/PistonMovingBlockEntity.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/client/render/ParticleEngine.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/Gui.hpp"
#include "game/item/RecordItem.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/CauldronBlock.hpp"
#include "game/world/block/DragonEggBlock.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/TheEndDimension.hpp"
#include "platform/wiiu/CProfile.hpp"

#ifdef MINECRAFT_PC
#include "platform/pc/PcSettings.hpp"
#include "platform/pc/PcModViews.hpp"

static int ChunkGridWidth(int activePlayers)
{
    const float distance = static_cast<float>(PcSettingsRenderDistance());
    return (int)sqrtf(4.0f * distance * distance / (float)activePlayers);
}

static unsigned int CommandBufferBudget()
{
    const unsigned int distance = (unsigned int)PcSettingsRenderDistance();
    const unsigned long long budget = 115343360ull * distance * distance / 100u;
    if (budget < 115343360ull)
        return 115343360u;
    return budget > 0x40000000ull ? 0x40000000u : (unsigned int)budget;
}

static const unsigned int gChunkDimensionSizes[3] = {640, 80, 128};
#else
static const unsigned int gChunkDimensionSizes[2] = {80, 20};
#endif

static ResourceLocation sMoonPhasesLocation(TN_TERRAIN_MOON_PHASES);
static ResourceLocation sSunLocation(TN_TERRAIN_SUN);
static ResourceLocation sCloudsLocation(TN_ENVIRONMENT_CLOUDS);
static ResourceLocation sEndSkyLocation(TN_MISC_TUNNEL);

float gCloudDriftDelta = 0.0f;
float gCloudPrevTime = -1.0f;
float gCloudDriftAccum = 0.0f;
bool gCloudDriftLatched = false;

#ifdef MINECRAFT_PC
int gChunkDimensionBases[4] = {0, 0x640000, 0x659000, 0x699000};
#else
int gChunkDimensionBases[4] = {0, 0x19000, 0x1a900, 0x1c640};
#endif

LevelRenderer* gLevelRenderer;

int getChunkIndex(int dimension, int x, int y, int z)
{
#ifdef MINECRAFT_PC
    if ((unsigned int)dimension < 3)
#else
    if ((unsigned int)dimension < 2)
#endif
    {
        unsigned int size = gChunkDimensionSizes[dimension];
        int half = (int)size / 2;
        int offsetZ = half + z;
        int offsetX = half + x;

        if (offsetX >= 0 && offsetX < (int)size && (unsigned int)y < 16 && offsetZ >= 0 && offsetZ < (int)size)
        {
            return gChunkDimensionBases[dimension] + (offsetX * size + offsetZ) * 16 + y;
        }

        return -1;
    }

    if (dimension != 2)
    {
        return -1;
    }

    unsigned int innerX = (x + 9) - TheEndDimension::AREAS[0].x;
    unsigned int innerZ = (z + 9) - TheEndDimension::AREAS[0].z;

    if (innerX < 18 && (unsigned int)y < 16 && innerZ < 18)
    {
        return gChunkDimensionBases[2] + innerX * 0x120 + innerZ * 16 + y;
    }

    unsigned int outerX = (x + 6) - TheEndDimension::AREAS[1].x;
    unsigned int outerZ = (z + 6) - TheEndDimension::AREAS[1].z;

    if (outerX < 12 && (unsigned int)y < 16 && outerZ < 12)
    {
        return gChunkDimensionBases[2] + outerX * 0xc0 + outerZ * 16 + y + 0x1440;
    }

    return -1;
}

LevelRenderer::DestroyedBlockManager::RecentBlock::RecentBlock(const BlockPos& blockPos, Level* sourceLevel)
    : pos(blockPos),
      level(sourceLevel),
      ticks(20),
      remove(false)
{
}

LevelRenderer::DestroyedBlockManager::RecentBlock::~RecentBlock()
{
    for (std::vector<AABB*>::iterator box = boxes.begin(); box != boxes.end(); ++box)
    {
        delete *box;
    }
}

LevelRenderer::DestroyedBlockManager::DestroyedBlockManager()
{
    InitializeCriticalSection(&lock);
}

void LevelRenderer::DestroyedBlockManager::tick()
{
    EnterCriticalSection(&lock);

    for (unsigned int index = 0; index < blocks.size();)
    {
        if (--blocks[index]->ticks == 0)
        {
            delete blocks[index];
            blocks[index] = blocks[blocks.size() - 1];
            blocks.pop_back();
        }
        else
        {
            ++index;
        }
    }

    LeaveCriticalSection(&lock);
}

void LevelRenderer::DestroyedBlockManager::updatedChunkAt(Level* sourceLevel, const BlockPos& chunkPos, int radius)
{
    EnterCriticalSection(&lock);

    for (unsigned int index = 0; index < blocks.size(); ++index)
    {
        RecentBlock* recent = blocks[index];

        if (recent->level == sourceLevel && recent->pos.x >= chunkPos.x && recent->pos.x < chunkPos.x + 16
            && recent->pos.y >= chunkPos.y && recent->pos.y < chunkPos.y + 16 && recent->pos.z >= chunkPos.z
            && recent->pos.z < chunkPos.z + 16)
        {
            recent->remove = true;
        }
    }

    if (radius < 2)
    {
        for (unsigned int index = 0; index < blocks.size();)
        {
            if (blocks[index]->remove)
            {
                delete blocks[index];
                blocks[index] = blocks[blocks.size() - 1];
                blocks.pop_back();
            }
            else
            {
                ++index;
            }
        }
    }

    LeaveCriticalSection(&lock);
}

void LevelRenderer::DestroyedBlockManager::destroyingBlockAt(Level* sourceLevel, const BlockPos& blockPos)
{
    EnterCriticalSection(&lock);

    RecentBlock* recent = new RecentBlock(blockPos, sourceLevel);
    AABB* bounds = AABB::newTemp((double)blockPos.x, (double)blockPos.y, (double)blockPos.z, (double)(blockPos.x + 1),
                                 (double)(blockPos.y + 1), (double)(blockPos.z + 1));
    const BlockState* state = Minecraft::GetInstance()->mLevel->getBlockState(blockPos);
    Block* block = state->getBlock();

    if (block != 0)
    {
        boost::shared_ptr<Entity> none;
        block->addCollisionAABBs(state, sourceLevel, blockPos, bounds, &recent->boxes, none, false);
    }

    for (unsigned int index = 0; index < recent->boxes.size(); ++index)
    {
        AABB* box = recent->boxes[index];
        recent->boxes[index] = AABB::newPermanent(box->mMinX, box->mMinY, box->mMinZ, box->mMaxX, box->mMaxY,
                                                  box->mMaxZ);
    }

    blocks.push_back(recent);

    LeaveCriticalSection(&lock);
}

void LevelRenderer::DestroyedBlockManager::addAABBs(Level* sourceLevel, const AABB* area, std::vector<AABB*>* result)
{
    EnterCriticalSection(&lock);

    for (unsigned int index = 0; index < blocks.size(); ++index)
    {
        if (blocks[index]->level != sourceLevel)
        {
            continue;
        }

        for (unsigned int box = 0; box < blocks[index]->boxes.size(); ++box)
        {
            if (blocks[index]->boxes[box]->intersects(area->mMinX, area->mMinY, area->mMinZ, area->mMaxX, area->mMaxY,
                                                      area->mMaxZ))
            {
                AABB* hit = blocks[index]->boxes[box];
                result->push_back(
                    AABB::newTemp(hit->mMinX, hit->mMinY, hit->mMinZ, hit->mMaxX, hit->mMaxY, hit->mMaxZ));
            }
        }
    }

    LeaveCriticalSection(&lock);
}

LevelRenderer::LevelRenderer(Minecraft* minecraft, Textures* textures)
    : mBlockEntities(11),
      mRenderableBlockEntities(11),
      mDestroyingBlocks(11)
{
    memset(mDestroyStages, 0, sizeof(mDestroyStages));

    mEntityRenderDispatcher = Minecraft::GetInstance()->getEntityRenderDispatcher();

    for (int player = 0; player < 4; ++player)
    {
        mLevels[player] = 0;
        mBlockRenderers[player] = 0;
    }

    mChunkGridWidth = 0;
    mChunkGridHeight = 0;
    mChunkGridDepth = 0;
    mDisplayList = 0;
    mRenderTick = 0;
    mStarList = 0;
    mSkyList = 0;
    mDarkList = 0;
    mMinChunkX = 0;
    mMinChunkY = 0;
    mMinChunkZ = 0;
    mMaxChunkX = 0;
    mMaxChunkY = 0;
    mMaxChunkZ = 0;

    for (int player = 0; player < 4; ++player)
    {
        mLastSortX[player] = -9999.0;
        mLastSortY[player] = -9999.0;
        mLastSortZ[player] = -9999.0;
    }

    mLastViewDistance = -1;
    mSkippedFrames = 2;
    mCountEntitiesTotal = 0;
    mCountEntitiesRendered = 0;
    mCountEntitiesHidden = 0;
    mUnknown0x128 = 0;
    mUnknown0x180 = 0;
    int auxiliaryDisplayList = MemoryTracker::genLists(1);

    for (int counter = 0; counter < 5; ++counter)
    {
        mRenderCounters[counter] = 0;
    }

    for (int player = 0; player < 4; ++player)
    {
        mClipChunks[player].data = 0;
        mClipChunks[player].length = 0;
        mActivePlayers[player] = 0;
    }

    mAuxiliaryDisplayList = auxiliaryDisplayList;
    mCloudOffset = 0.0f;

    InitializeCriticalSection(&mChunkMutex);
    InitializeCriticalSection(&mBlockEntityMutex);
    InitializeCriticalSection(&mLevelMutex);
    InitializeCriticalSection(&mRenderableBlockEntityMutex);

    mTextures = textures;
    mHasDirtyChunks = false;
    mLastRebuildTime = 0;
    mMinecraft = minecraft;

    mDisplayList = MemoryTracker::genLists(getGlobalChunkCount() * 3);
    mGlobalChunkFlags = new unsigned char[getGlobalChunkCount()];
    memset(mGlobalChunkFlags, 0, getGlobalChunkCount());

    mStarList = MemoryTracker::genLists(4);

    GlStateManager::pushMatrix();
    GlStateManager::newList(mStarList, 0);
    renderStars();
    GlStateManager::endList(0);
    createCloudMesh();
    GlStateManager::popMatrix();

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    mSkyList = mStarList + 1;
    GlStateManager::newList(mSkyList, 0);
    GlStateManager::depthMask(false);

    for (int skyX = -384; skyX <= 384; skyX += 64)
    {
        for (int skyZ = -384; skyZ <= 384; skyZ += 64)
        {
            builder->begin();
            builder->vertex((float)skyX, 16.0f, (float)skyZ);
            builder->vertex((float)(skyX + 64), 16.0f, (float)skyZ);
            builder->vertex((float)(skyX + 64), 16.0f, (float)(skyZ + 64));
            builder->vertex((float)skyX, 16.0f, (float)(skyZ + 64));
            builder->end();
        }
    }

    GlStateManager::endList(0);

    mDarkList = mStarList + 2;
    GlStateManager::newList(mDarkList, 0);
    builder->begin();

    for (int darkX = -384; darkX <= 384; darkX += 64)
    {
        for (int darkZ = -384; darkZ <= 384; darkZ += 64)
        {
            builder->vertex((float)(darkX + 64), -16.0f, (float)darkZ);
            builder->vertex((float)darkX, -16.0f, (float)darkZ);
            builder->vertex((float)darkX, -16.0f, (float)(darkZ + 64));
            builder->vertex((float)(darkX + 64), -16.0f, (float)(darkZ + 64));
        }
    }

    builder->end();
    GlStateManager::endList(0);

    mHaloRingList = mStarList + 3;
    GlStateManager::newList(mHaloRingList, 0);
    builder->begin(1);
    builder->color(0xffffff);

    float ringU = 0.0f;

    for (unsigned int segment = 0; segment < 51; ++segment)
    {
        float distance = fabsf((float)segment - 25.0f);
        float over = distance < 19.0f ? 0.0f : distance - 19.0f;
        float halfHeight = (over * over) / 36.0f * 10.0f + 1.0f;
        float angle = (float)segment * 0.12566371f;

        builder->vertexUV(100.0f * cosf(angle) - 99.9f, 100.0f * sinf(angle), -halfHeight, ringU, 0.0f);
        builder->vertexUV(100.0f * cosf(angle) - 99.9f, 100.0f * sinf(angle), halfHeight, ringU, 1.0f);

        ringU -= 0.25;
    }

    builder->end();
    GlStateManager::endList(0);

    gLevelRenderer = this;
    mDestroyedBlocks = new DestroyedBlockManager();
}

LevelRenderer::~LevelRenderer()
{
}

unsigned int LevelRenderer::getGlobalChunkCount()
{
    return gChunkDimensionBases[3];
}

void LevelRenderer::renderStars()
{
    Random random(10842LL);
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    builder->begin();

    for (int star = 0; star < 1500; ++star)
    {
        double x = (double)(random.nextFloat() * 2.0f - 1.0f);
        double y = (double)(random.nextFloat() * 2.0f - 1.0f);
        double z = (double)(random.nextFloat() * 2.0f - 1.0f);
        double size = (double)(random.nextFloat() * 0.1f + 0.15f);
        double lengthSqr = x * x + y * y + z * z;

        if (lengthSqr < 1.0 && lengthSqr > 0.01)
        {
            double scale = 1.0 / sqrt(lengthSqr);

            x = x * scale;
            y = y * scale;
            z = z * scale;

            double centreX = x * 160.0;
            double centreY = y * 160.0;
            double centreZ = z * 160.0;
            double yaw = atan2(x, z);
            double sinYaw = sin(yaw);
            double cosYaw = cos(yaw);
            double pitch = atan2(sqrt(x * x + z * z), y);
            double sinPitch = sin(pitch);
            double cosPitch = cos(pitch);
            double roll = random.nextDouble() * 3.1415927f * 2.0;
            double sinRoll = sin(roll);
            double cosRoll = cos(roll);

            for (int corner = 0; corner < 4; ++corner)
            {
                double flat = 0.0;
                double cornerU = (double)((corner & 2) - 1) * size;
                double cornerV = (double)((corner + 1 & 2) - 1) * size;
                double rolledU = cornerU * cosRoll - cornerV * sinRoll;
                double rolledV = cornerV * cosRoll + cornerU * sinRoll;
                double pitchedY = rolledU * sinPitch + flat * cosPitch;
                double pitchedZ = flat * sinPitch - rolledU * cosPitch;
                double offsetX = pitchedZ * sinYaw - rolledV * cosYaw;
                double offsetZ = rolledV * sinYaw + pitchedZ * cosYaw;

                builder->vertex((float)(centreX + offsetX), (float)(centreY + pitchedY), (float)(centreZ + offsetZ));
            }
        }
    }

    builder->end();
}

void LevelRenderer::createCloudMesh()
{
    mCloudLists = MemoryTracker::genLists(7);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    for (int face = 0; face < 7; ++face)
    {
        GlStateManager::newList(mCloudLists + face, 0);

        if (face == 0 || face == 6)
        {
            builder->begin();

            for (int z = 0; z < 8; ++z)
            {
                for (int x = 0; x < 8; ++x)
                {
                    float u = ((float)x + 0.5f) * 0.00390625f;
                    float v = ((float)z + 0.5f) * 0.00390625f;

                    builder->color(0.7f, 0.7f, 0.7f, 0.8f);
                    builder->normal(0.0f, -1.0f, 0.0f);
                    builder->vertexUV((float)x, 0.0f, (float)z, u, v);
                    builder->vertexUV((float)x + 1.0f, 0.0f, (float)z, u, v);
                    builder->vertexUV((float)x + 1.0f, 0.0f, (float)z + 1.0f, u, v);
                    builder->vertexUV((float)x, 0.0f, (float)z + 1.0f, u, v);
                }
            }

            builder->end();
        }

        if (face == 1 || face == 6)
        {
            builder->begin();

            for (int z = 0; z < 8; ++z)
            {
                for (int x = 0; x < 8; ++x)
                {
                    float u = ((float)x + 0.5f) * 0.00390625f;
                    float v = ((float)z + 0.5f) * 0.00390625f;

                    builder->color(1.0f, 1.0f, 1.0f, 0.8f);
                    builder->normal(0.0f, 1.0f, 0.0f);
                    builder->vertexUV((float)x, 4.0f, (float)z + 1.0f, u, v);
                    builder->vertexUV((float)x + 1.0f, 4.0f, (float)z + 1.0f, u, v);
                    builder->vertexUV((float)x + 1.0f, 4.0f, (float)z, u, v);
                    builder->vertexUV((float)x, 4.0f, (float)z, u, v);
                }
            }

            builder->end();
        }

        if (face == 2 || face == 6)
        {
            builder->begin();

            for (int z = 0; z < 8; ++z)
            {
                for (int x = 0; x < 8; ++x)
                {
                    float u = ((float)x + 0.5f) * 0.00390625f;
                    float v = ((float)z + 0.5f) * 0.00390625f;

                    builder->color(0.9f, 0.9f, 0.9f, 0.8f);
                    builder->normal(-1.0f, 0.0f, 0.0f);
                    builder->vertexUV((float)x, 0.0f, (float)z + 1.0f, u, v);
                    builder->vertexUV((float)x, 4.0f, (float)z + 1.0f, u, v);
                    builder->vertexUV((float)x, 4.0f, (float)z, u, v);
                    builder->vertexUV((float)x, 0.0f, (float)z, u, v);
                }
            }

            builder->end();
        }

        if (face == 3 || face == 6)
        {
            builder->begin();

            for (int z = 0; z < 8; ++z)
            {
                for (int x = 0; x < 8; ++x)
                {
                    float u = ((float)x + 0.5f) * 0.00390625f;
                    float v = ((float)z + 0.5f) * 0.00390625f;

                    builder->color(0.9f, 0.9f, 0.9f, 0.8f);
                    builder->normal(1.0f, 0.0f, 0.0f);
                    builder->vertexUV((float)x + 1.0f, 0.0f, (float)z, u, v);
                    builder->vertexUV((float)x + 1.0f, 4.0f, (float)z, u, v);
                    builder->vertexUV((float)x + 1.0f, 4.0f, (float)z + 1.0f, u, v);
                    builder->vertexUV((float)x + 1.0f, 0.0f, (float)z + 1.0f, u, v);
                }
            }

            builder->end();
        }

        if (face == 4 || face == 6)
        {
            builder->begin();

            for (int z = 0; z < 8; ++z)
            {
                for (int x = 0; x < 8; ++x)
                {
                    float u = ((float)x + 0.5f) * 0.00390625f;
                    float v = ((float)z + 0.5f) * 0.00390625f;

                    builder->color(0.8f, 0.8f, 0.8f, 0.8f);
                    builder->normal(-1.0f, 0.0f, 0.0f);
                    builder->vertexUV((float)x, 4.0f, (float)z, u, v);
                    builder->vertexUV((float)x + 1.0f, 4.0f, (float)z, u, v);
                    builder->vertexUV((float)x + 1.0f, 0.0f, (float)z, u, v);
                    builder->vertexUV((float)x, 0.0f, (float)z, u, v);
                }
            }

            builder->end();
        }

        if (face == 5 || face == 6)
        {
            builder->begin();

            for (int z = 0; z < 8; ++z)
            {
                for (int x = 0; x < 8; ++x)
                {
                    float u = ((float)x + 0.5f) * 0.00390625f;
                    float v = ((float)z + 0.5f) * 0.00390625f;

                    builder->color(0.8f, 0.8f, 0.8f, 0.8f);
                    builder->normal(1.0f, 0.0f, 0.0f);
                    builder->vertexUV((float)x, 0.0f, (float)z + 1.0f, u, v);
                    builder->vertexUV((float)x + 1.0f, 0.0f, (float)z + 1.0f, u, v);
                    builder->vertexUV((float)x + 1.0f, 4.0f, (float)z + 1.0f, u, v);
                    builder->vertexUV((float)x, 4.0f, (float)z + 1.0f, u, v);
                }
            }

            builder->end();
        }

        GlStateManager::endList(0);
    }
}

int LevelRenderer::activePlayers()
{
    int count = 0;

    for (int i = 0; i < 4; ++i)
    {
        if (mLevels[i] != 0)
        {
            ++count;
        }
    }

    return count;
}

void LevelRenderer::nonStackDirtyChunksAdded()
{
    mDirtyChunkStack.Push((int*)1);
}

void LevelRenderer::resortChunks(int x, int y, int z, int dimension)
{
    if (dimension == -1)
    {
        dimension = mMinecraft->mPlayer->mUserIndex;
    }

    EnterCriticalSection(&mChunkMutex);

    mMinChunkX = 0x7fffffff;
    mMinChunkY = 0x7fffffff;
    mMinChunkZ = 0x7fffffff;
    mMaxChunkX = (int)0x80000000;
    mMaxChunkY = (int)0x80000000;
    mMaxChunkZ = (int)0x80000000;

    int spanX = mChunkGridWidth * 16;

    for (int gridX = 0; gridX < mChunkGridWidth; ++gridX)
    {
        int worldX = gridX * 16;
        int wrapX = (worldX + spanX / 2) - (x - 8);

        if (wrapX < 0)
        {
            wrapX -= spanX - 1;
        }

        wrapX = worldX - (wrapX / spanX) * spanX;

        if (wrapX < mMinChunkX)
        {
            mMinChunkX = wrapX;
        }

        if (wrapX > mMaxChunkX)
        {
            mMaxChunkX = wrapX;
        }

        for (int gridZ = 0; gridZ < mChunkGridDepth; ++gridZ)
        {
            int worldZ = gridZ * 16;
            int wrapZ = (worldZ + spanX / 2) - (z - 8);

            if (wrapZ < 0)
            {
                wrapZ -= spanX - 1;
            }

            wrapZ = worldZ - (wrapZ / spanX) * spanX;

            if (wrapZ < mMinChunkZ)
            {
                mMinChunkZ = wrapZ;
            }

            if (wrapZ > mMaxChunkZ)
            {
                mMaxChunkZ = wrapZ;
            }

            for (int gridY = 0; gridY < mChunkGridHeight; ++gridY)
            {
                int worldY = gridY * 16;

                if (worldY < mMinChunkY)
                {
                    mMinChunkY = worldY;
                }

                if (worldY > mMaxChunkY)
                {
                    mMaxChunkY = worldY;
                }

                mClipChunks[dimension]
                    .data[(gridZ * mChunkGridHeight + gridY) * mChunkGridWidth + gridX]
                    .mChunk->setPos(wrapX, worldY, wrapZ);
            }
        }
    }

    nonStackDirtyChunksAdded();

    LeaveCriticalSection(&mChunkMutex);
}

void LevelRenderer::setLevel(int dimension, MultiPlayerLevel* level)
{
    EnterCriticalSection(&mLevelMutex);

    Level* oldLevel = mLevels[dimension];

    if (oldLevel != 0)
    {
        int oldReferences = 0;

        for (int i = 0; i < 4; ++i)
        {
            if (mLevels[i] == oldLevel)
            {
                ++oldReferences;
            }
        }

        if (oldReferences == 1)
        {
            oldLevel->removeListener(this);
        }
    }

    mLevels[dimension] = level;
    mLastSortX[dimension] = -9999.0;
    mLastSortY[dimension] = -9999.0;
    mLastSortZ[dimension] = -9999.0;

    BlockRenderer* oldRenderer = mBlockRenderers[dimension];

    if (oldRenderer != 0)
    {
        delete oldRenderer;
    }

    mBlockRenderers[dimension] = new BlockRenderer(level);

    if (level != 0)
    {
        int newReferences = 0;

        for (int i = 0; i < 4; ++i)
        {
            if (mLevels[i] == level)
            {
                ++newReferences;
            }
        }

        if (newReferences == 1)
        {
            level->addListener(this);
        }

        allChanged(dimension);
        LeaveCriticalSection(&mLevelMutex);
    }
    else
    {
        if (mClipChunks[dimension].data != 0)
        {
            for (unsigned int index = 0; index < mClipChunks[dimension].length; ++index)
            {
                mClipChunks[dimension][index].mChunk->_delete();
                delete mClipChunks[dimension][index].mChunk;
            }

            delete mClipChunks[dimension].data;
            mClipChunks[dimension].length = 0;
            mClipChunks[dimension].data = 0;
        }

        if (dimension == gProfile->GetPrimaryPad())
        {
            mBlockEntities.clear();
        }

        LeaveCriticalSection(&mLevelMutex);
    }
}

void LevelRenderer::allChanged(int dimension)
{
    if (mLevels[dimension] == 0)
    {
        return;
    }

    Minecraft::GetInstance()->mGameRenderer->DisableUpdateThread(4);

    static_cast<LeafBlock*>(Blocks::leaves)->setFancy(mMinecraft->mOptions->mFancyGraphics);
    static_cast<LeafBlock*>(Blocks::leaves2)->setFancy(mMinecraft->mOptions->mFancyGraphics);

    mLastViewDistance = mMinecraft->mOptions->mViewDistance;

#ifdef MINECRAFT_PC
    int viewDistance = ChunkGridWidth(activePlayers());
#else
    int viewDistance = (int)sqrtf(400.0f / (float)activePlayers());
#endif

    mMinecraft->mOptions->mViewDistance = viewDistance;
    mActivePlayers[dimension] = activePlayers();
    mChunkGridWidth = viewDistance;
    mChunkGridHeight = 16;
    mChunkGridDepth = viewDistance;

    if (mClipChunks[dimension].data != 0)
    {
        for (unsigned int index = 0; index < mClipChunks[dimension].length; ++index)
        {
            mClipChunks[dimension][index].mChunk->_delete();
            delete mClipChunks[dimension][index].mChunk;
        }

        delete mClipChunks[dimension].data;
    }

    mClipChunks[dimension] = arrayWithLength<ClipChunk>(mChunkGridWidth * mChunkGridHeight * mChunkGridDepth, true);

    for (int index = 0; index < mChunkGridWidth * mChunkGridHeight * mChunkGridDepth; ++index)
    {
        mClipChunks[dimension][index].mChunk = 0;
    }

    mMinChunkX = 0;
    mMinChunkY = 0;
    mMinChunkZ = 0;
    mMaxChunkX = mChunkGridWidth;
    mMaxChunkY = mChunkGridHeight;
    mMaxChunkZ = mChunkGridDepth;

    int globalIndex = 0;

    for (int gridX = 0; gridX < mChunkGridWidth; ++gridX)
    {
        for (int gridY = 0; gridY < mChunkGridHeight; ++gridY)
        {
            for (int gridZ = 0; gridZ < mChunkGridDepth; ++gridZ)
            {
                ClipChunk* clipChunk =
                    &mClipChunks[dimension].data[(gridZ * mChunkGridHeight + gridY) * mChunkGridWidth + gridX];

                clipChunk->mChunk = new Chunk(mLevels[dimension], mBlockEntities, mBlockEntityMutex, gridX * 16,
                                              gridY * 16, gridZ * 16, clipChunk);
                clipChunk->mVisible = true;
                clipChunk->mChunk->mUnknown = globalIndex;
                ++globalIndex;
            }
        }
    }

    nonStackDirtyChunksAdded();

    EnterCriticalSection(&mRenderableBlockEntityMutex);
    mRenderableBlockEntities.clear();
    LeaveCriticalSection(&mRenderableBlockEntityMutex);

    if (mLevels[dimension] != 0)
    {
        boost::shared_ptr<Entity> camera = mMinecraft->getCameraEntity();

        if (camera)
        {
            resortChunks(Mth::floor(camera->mX), Mth::floor(camera->mY), Mth::floor(camera->mZ), dimension);
        }
    }

    mSkippedFrames = 2;

    Minecraft::GetInstance()->mGameRenderer->EnableUpdateThread(4);
}

void LevelRenderer::tick()
{
    ++mRenderTick;

    if (mRenderTick % 20 != 0)
    {
        return;
    }

    boost::unordered_map<int, BlockDestructionProgress*>::iterator progress = mDestroyingBlocks.begin();

    while (progress != mDestroyingBlocks.end())
    {
        if (mRenderTick - progress->second->getUpdatedRenderTick() > 400)
        {
            delete progress->second;
            progress = mDestroyingBlocks.erase(progress);
        }
        else
        {
            ++progress;
        }
    }
}

void LevelRenderer::clear()
{
    MemoryTracker::releaseLists(mDisplayList);
}

void LevelRenderer::allChanged()
{
    int userIndex = mMinecraft->mPlayer->mUserIndex;
    allChanged(userIndex);
}

bool LevelRenderer::sortEntDisFunc(std::pair<int, double> first, std::pair<int, double> second)
{
    return first.second < second.second;
}

int LevelRenderer::getDimensionIndexFromId(int id)
{
    return (3 - id) % 3;
}

bool LevelRenderer::isGlobalIndexInSameDimension(int index, Level* level)
{
    int dimensionIndex = getDimensionIndexFromId(level->mDimension->getType()->getId());

    bool sameDimension;

    if (index < gChunkDimensionBases[1])
    {
        sameDimension = dimensionIndex == 0;
    }
    else if (index < gChunkDimensionBases[2])
    {
        sameDimension = dimensionIndex == 1;
    }
    else
    {
        int otherDimensionIndex = 2;

        if (index >= gChunkDimensionBases[3])
        {
            otherDimensionIndex = -1;
        }

        sameDimension = dimensionIndex == otherDimensionIndex;
    }

    return sameDimension;
}

bool LevelRenderer::shouldEntityGlow(boost::shared_ptr<Entity> entity, boost::shared_ptr<Entity> camera, Culler* culler)
{
    bool cameraIsSleeping = false;

    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, camera->GetType()))
    {
        cameraIsSleeping = boost::static_pointer_cast<LivingEntity>(camera)->isSleeping();
    }

    if (entity != camera || mMinecraft->mOptions->mThirdPersonView != 0 || cameraIsSleeping)
    {
        if (entity->isGlowing())
        {
            return true;
        }
    }

    return false;
}

void LevelRenderer::setupDestroyState()
{
    GlStateManager::blendFuncSeparate(8, 2, 1, 0);
    GlStateManager::blendFunc(8, 2);
    GlStateManager::enableBlend();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 0.5f);
    GlStateManager::polygonOffset(-3.0f, -3.0f);
    GlStateManager::enablePolygonOffset();
    GlStateManager::alphaFunc(4, 0.01f);
    GlStateManager::enableAlphaTest();
    GlStateManager::pushMatrix();
}

void LevelRenderer::restoreDestroyState()
{
    GlStateManager::disableAlphaTest();
    GlStateManager::polygonOffset(0.0f, 0.0f);
    GlStateManager::disablePolygonOffset();
    GlStateManager::enableAlphaTest();
    GlStateManager::depthMask(true);
    GlStateManager::popMatrix();
}

void LevelRenderer::renderEntities(boost::shared_ptr<Entity> camera, Culler* culler, float partialTick)
{
    int userIndex = mMinecraft->mPlayer->mUserIndex;

    PIXBeginNamedEvent(0.0f, "Prepare");

    BlockEntityRenderDispatcher::instance->prepare(mLevels[userIndex], mTextures, mMinecraft->mFont,
                                                   mMinecraft->getCameraEntity(), mMinecraft->mHitResult, partialTick);

    mMinecraft->getBlockRenderer()->prepare(mLevels[userIndex]);

    mEntityRenderDispatcher->prepare(mLevels[userIndex], mMinecraft->mFont, mMinecraft->getCameraEntity(),
                                     mMinecraft->mSpectateEntity, mMinecraft->mOptions, partialTick);

    PIXEndNamedEvent();

    if (mSkippedFrames > 0)
    {
        --mSkippedFrames;
        return;
    }

    mCountEntitiesTotal = 0;
    mCountEntitiesRendered = 0;
    mCountEntitiesHidden = 0;

    double camX = (camera->mX - camera->mXo) * partialTick + camera->mXo;
    double camY = (camera->mY - camera->mYo) * partialTick + camera->mYo;
    double camZ = (camera->mZ - camera->mZo) * partialTick + camera->mZo;

    boost::shared_ptr<Entity> positionDispatcherCamera = mMinecraft->getCameraEntity();
    double dispatcherX = (positionDispatcherCamera->mX - positionDispatcherCamera->mXOld) * partialTick
                         + positionDispatcherCamera->mXOld;
    double dispatcherY = (positionDispatcherCamera->mY - positionDispatcherCamera->mYOld) * partialTick
                         + positionDispatcherCamera->mYOld;
    double dispatcherZ = (positionDispatcherCamera->mZ - positionDispatcherCamera->mZOld) * partialTick
                         + positionDispatcherCamera->mZOld;
    BlockEntityRenderDispatcher::xOff = dispatcherX;
    BlockEntityRenderDispatcher::yOff = dispatcherY;
    BlockEntityRenderDispatcher::zOff = dispatcherZ;
    mEntityRenderDispatcher->setPosition(dispatcherX, dispatcherY, dispatcherZ);

    mMinecraft->mGameRenderer->turnOnLightLayer(partialTick);

    std::vector<boost::shared_ptr<Entity> > allEntities = mLevels[userIndex]->getAllEntities();
    mCountEntitiesTotal = (int)allEntities.size();

    PIXBeginNamedEvent(0.0f, "Sort entities by distance for rendering");

    std::vector<std::pair<int, double> > sortedIndices;
    sortedIndices.reserve(allEntities.size());
    for (unsigned int entityIndex = 0; entityIndex < allEntities.size(); ++entityIndex)
    {
        boost::shared_ptr<Entity>& sortEntity = allEntities[entityIndex];
        double dx = camX - sortEntity->mX;
        double dy = camY - sortEntity->mY;
        double dz = camZ - sortEntity->mZ;
        sortedIndices.push_back(std::pair<int, double>((int)entityIndex, pow(dx, 2) + pow(dy, 2) + pow(dz, 2)));
    }
    std::sort(sortedIndices.begin(), sortedIndices.end(), &LevelRenderer::sortEntDisFunc);

    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Render %d global entities", (int)mLevels[userIndex]->mGlobalEntities.size());
    std::vector<boost::shared_ptr<Entity> >::iterator globalEnd = mLevels[userIndex]->mGlobalEntities.end();
    for (std::vector<boost::shared_ptr<Entity> >::iterator globalIt = mLevels[userIndex]->mGlobalEntities.begin();
         globalIt != globalEnd; ++globalIt)
    {
        boost::shared_ptr<Entity> globalEntity = *globalIt;
        ++mCountEntitiesRendered;
        if (globalEntity->shouldRender(camZ, camY, camZ))
        {
            mEntityRenderDispatcher->render(globalEntity, partialTick, false);
        }
    }
    PIXEndNamedEvent();

    InfoBarManager::StartTimingCategory(InfoBarManager::eTimingCagegory_Entities);
    PIXBeginNamedEvent(0.0f, "Render %d entities", (int)allEntities.size());

    std::vector<boost::shared_ptr<Entity> > glowingEntities;
    std::vector<boost::shared_ptr<Entity> > secondPassEntities;

    for (unsigned int sortIndex = 0; sortIndex < sortedIndices.size(); ++sortIndex)
    {
        boost::shared_ptr<Entity>& entity = allEntities[sortedIndices[sortIndex].first];
        double entityDistanceSqr = sortedIndices[sortIndex].second;

        PIXBeginNamedEvent(0.0f, "shouldRender");
        bool renderable = mEntityRenderDispatcher->shouldRender(entity, culler, entityDistanceSqr);
        if (!renderable)
        {
            if (entity->hasIndirectPassenger(mMinecraft->mPlayer))
            {
                renderable = true;
            }
            else if (entity->hasPassenger(EntityType::PLAYER))
            {
                renderable = true;
            }
            else if (eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType()))
            {
                boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(entity);
                if (mob->isLeashed() && mob->getLeashHolder() != NULL)
                {
                    renderable = culler->isVisible(mob->getLeashHolder()->mBoundingBox);
                }
            }
        }
        PIXEndNamedEvent();

        if (!renderable)
        {
            continue;
        }

        if (entity == mMinecraft->getCameraEntity())
        {
            bool thirdPerson = Minecraft::GetInstance()->mPlayer->GetThirdPersonView(false) != 0;
            bool sleeping = eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType())
                            && boost::static_pointer_cast<LivingEntity>(entity)->isSleeping();
            if (!thirdPerson && !sleeping)
            {
                continue;
            }
        }

        if (entity->mY >= 0.0 && entity->mY < 256.0)
        {
            BlockPos entityBlockPos(Mth::floor(entity->mX), 0, Mth::floor(entity->mZ));
            if (!mLevels[userIndex]->hasChunkAt(entityBlockPos))
            {
                continue;
            }
        }

        ++mCountEntitiesRendered;
        mEntityRenderDispatcher->render(entity, partialTick, false);

        if (shouldEntityGlow(entity, positionDispatcherCamera, culler))
        {
            glowingEntities.push_back(entity);
        }

        if (mEntityRenderDispatcher->hasSecondPass(entity))
        {
            secondPassEntities.push_back(entity);
        }
    }

    for (unsigned int secondPassIndex = 0; secondPassIndex < secondPassEntities.size(); ++secondPassIndex)
    {
        mEntityRenderDispatcher->renderSecondPass(secondPassEntities[secondPassIndex], partialTick);
    }

    PIXEndNamedEvent();
    InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_Entities);
    Lighting::turnOn();

    EnterCriticalSection(&mBlockEntityMutex);
    PIXBeginNamedEvent(0.0f, "Render %d block entities", (int)mBlockEntities.size());
    InfoBarManager::StartTimingCategory(InfoBarManager::eTimingCagegory_BlockEntities);

    std::vector<boost::shared_ptr<BlockEntity> > deferredBlockEntities;

    for (ChunkBlockEntityMap::iterator bucketIt = mBlockEntities.begin(); bucketIt != mBlockEntities.end(); ++bucketIt)
    {
        if (!isGlobalIndexInSameDimension(bucketIt->first, mLevels[userIndex]))
        {
            continue;
        }

        std::vector<boost::shared_ptr<BlockEntity> >& bucket = bucketIt->second;
        for (unsigned int bucketIndex = 0; bucketIndex < bucket.size(); ++bucketIndex)
        {
            boost::shared_ptr<BlockEntity>& blockEntity = bucket[bucketIndex];

            if (eTYPE_DERIVED_FROM(BlockEntityType::PISTON_MOVING, blockEntity->GetType()))
            {
                boost::shared_ptr<PistonMovingBlockEntity> pistonEntity =
                    boost::static_pointer_cast<PistonMovingBlockEntity>(blockEntity);
                if (pistonEntity->getBlockState()->getBlock() == Blocks::slime)
                {
                    deferredBlockEntities.push_back(blockEntity);
                    continue;
                }
            }

            BlockEntityRenderDispatcher::instance->render(blockEntity, partialTick);
        }
    }

    for (unsigned int deferredIndex = 0; deferredIndex < deferredBlockEntities.size(); ++deferredIndex)
    {
        BlockEntityRenderDispatcher::instance->render(deferredBlockEntities[deferredIndex], partialTick);
    }

    InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_BlockEntities);
    deferredBlockEntities.clear();
    PIXEndNamedEvent();

    EnterCriticalSection(&mRenderableBlockEntityMutex);
    PIXBeginNamedEvent(0.0f, "Render %d global block entities", (int)mRenderableBlockEntities.size());

    for (boost::unordered_set<boost::shared_ptr<BlockEntity> >::iterator renderableIt =
             mRenderableBlockEntities.begin();
         renderableIt != mRenderableBlockEntities.end(); ++renderableIt)
    {
        BlockEntityRenderDispatcher::instance->render(*renderableIt, partialTick, -1, true);
    }

    PIXEndNamedEvent();
    LeaveCriticalSection(&mRenderableBlockEntityMutex);

    PIXBeginNamedEvent(0.0f, "Clear out block entities");

    for (ChunkBlockEntityMap::iterator bucketIt = mBlockEntities.begin(); bucketIt != mBlockEntities.end();)
    {
        std::vector<boost::shared_ptr<BlockEntity> >& bucket = bucketIt->second;
        for (std::vector<boost::shared_ptr<BlockEntity> >::iterator blockEntityIt = bucket.begin();
             blockEntityIt != bucket.end();)
        {
            if ((*blockEntityIt)->shouldRemoveForRender())
            {
                (*blockEntityIt)->setRenderRemoveStage(0);
                blockEntityIt = bucket.erase(blockEntityIt);
            }
            else
            {
                ++blockEntityIt;
            }
        }

        if (bucket.empty())
        {
            bucketIt = mBlockEntities.erase(bucketIt);
        }
        else
        {
            ++bucketIt;
        }
    }

    setupDestroyState();

    for (boost::unordered_map<int, BlockDestructionProgress*>::iterator progressIt = mDestroyingBlocks.begin();
         progressIt != mDestroyingBlocks.end(); ++progressIt)
    {
        BlockDestructionProgress* progress = progressIt->second;
        BlockPos destroyPos = progress->getPos();
        LevelChunk* destroyChunk = mLevels[userIndex]->getChunkAt(destroyPos);
        boost::shared_ptr<BlockEntity> destroyBlockEntity = destroyChunk->getBlockEntity(destroyPos,
                                                                                         LevelChunk::NO_CREATE);

        if (!destroyBlockEntity)
        {
            continue;
        }

        if (eTYPE_DERIVED_FROM(BlockEntityType::CHEST, destroyBlockEntity->GetType()))
        {
            boost::shared_ptr<ChestBlockEntity> chestEntity = boost::static_pointer_cast<ChestBlockEntity>(
                destroyBlockEntity);
            if (chestEntity->mWestNeighbour.lock() != NULL)
            {
                destroyPos = destroyPos.relative(Direction::WEST);
                destroyBlockEntity = mLevels[userIndex]->getBlockEntity(destroyPos);
            }
            else if (chestEntity->mNorthNeighbour.lock() != NULL)
            {
                destroyPos = destroyPos.relative(Direction::NORTH);
                destroyBlockEntity = mLevels[userIndex]->getBlockEntity(destroyPos);
            }
        }

        const BlockState* destroyState = mLevels[userIndex]->getBlockState(destroyPos);
        if (destroyBlockEntity && destroyState->hasCustomBreakingProgress())
        {
            BlockEntityRenderDispatcher::instance->render(destroyBlockEntity, partialTick, progress->getProgress(),
                                                          false);
        }
    }

    restoreDestroyState();

    PIXEndNamedEvent();
    LeaveCriticalSection(&mBlockEntityMutex);

    mMinecraft->mGameRenderer->turnOffLightLayer(partialTick);
}

unsigned int LevelRenderer::getBucketMask(double x, double y, double z, ClipChunk* chunk)
{
    unsigned int mask = 1;

    if ((double)chunk->mMaxX >= x)
    {
        mask = 0x21;
    }

    if ((double)chunk->mMinX <= x)
    {
        mask |= 8;
    }

    if ((double)chunk->mMinZ <= z)
    {
        mask |= 0x10;
    }

    if ((double)chunk->mMaxZ >= z)
    {
        mask |= 4;
    }

    if ((double)chunk->mMaxY >= y)
    {
        mask |= 0x40;
    }

    if ((double)chunk->mMinY <= y)
    {
        mask |= 2;
    }

    return mask;
}

void LevelRenderer::setupCameraInsideAreas(boost::shared_ptr<Entity> camera)
{
    if (!Minecraft::InMiniGame(eMiniGameId_Glide, true))
    {
        return;
    }

    mCameraInsideAreas.clear();

    std::vector<AABB*> areas;
    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();

    if (ruleset != 0)
    {
        ruleset->getActiveViewAreas(&areas);

        Vec3* position = Vec3::newTemp(camera->mX * 0.0625, camera->mY * 0.0625, camera->mZ * 0.0625);

        for (unsigned int index = 0; index < areas.size(); ++index)
        {
            AABB* area = areas[index];

            if (area->insideXZ(position))
            {
                mCameraInsideAreas.push_back(areas[index]);
            }
        }
    }
}

unsigned int LevelRenderer::clippedByCameraInsideAreas(ClipChunk* chunk)
{
    unsigned int clipped = 0;

    if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
    {
        clipped = 1;

        unsigned int areaCount = mCameraInsideAreas.size();

        if (areaCount != 0)
        {
            Vec3* position = Vec3::newTemp((double)(chunk->mX / 16), (double)(chunk->mY / 16),
                                           (double)(chunk->mZ / 16));

            unsigned int index = 0;

            do
            {
                if (mCameraInsideAreas[index]->insideXZ(position))
                {
                    clipped = 0;
                }

                ++index;
            } while (index < areaCount);
        }
        else
        {
            clipped = 0;
        }
    }

    return clipped;
}

int LevelRenderer::renderChunks(int first, int count, int layer, double partialTick)
{
    int userIndex = mMinecraft->mPlayer->mUserIndex;

    mMinecraft->mGameRenderer->turnOnLightLayer(partialTick);

    boost::shared_ptr<Entity> camera = mMinecraft->getCameraEntity();

    double camX = (camera->mX - camera->mXOld) * partialTick + camera->mXOld;
    double camY = (camera->mY - camera->mYOld) * partialTick + camera->mYOld;
    double camZ = (camera->mZ - camera->mZOld) * partialTick + camera->mZOld;

    GlStateManager::pushMatrix();
    GlStateManager::translatef(-(float)camX, -(float)camY, -(float)camZ);

    if (layer == 2)
    {
        unsigned int waterColor = Biome::OCEAN->getWaterColor();
        float alpha = (float)(int)(waterColor >> 24) / 255.0f;
        gRenderer->StateSetVertexAlpha(alpha);
    }

    bool running = true;
    unsigned int mask = (unsigned int)(4 << (layer & 0x3f)) & 0xff;

    ClipChunk* chunk = mClipChunks[userIndex].data;
    int visibleCount = 0;

    setupCameraInsideAreas(camera);

    {
        std::vector<int> visibleChunks;
        unsigned int chunkCount = mClipChunks[userIndex].length;
        visibleChunks.reserve(chunkCount);

        for (unsigned int index = 0; index < chunkCount; ++index, ++chunk)
        {
            if (chunk->mVisible && chunk->mGlobalIndex != -1 && (mask & mGlobalChunkFlags[chunk->mGlobalIndex]) != mask
                && clippedByCameraInsideAreas(chunk) == 0)
            {
                int globalIndex = chunk->mGlobalIndex;
                int bucket = getBucketMask(camX, camY, camZ, chunk);
#ifdef MINECRAFT_PC
                if (PcModViewsInView())
                    bucket = 0x7f;
#endif
                visibleChunks.push_back(globalIndex | bucket << 24);
                ++visibleCount;
            }
        }

        unsigned int visibleSize = visibleChunks.size();

#ifdef MINECRAFT_PC
        unsigned int index = 0;
        for (; index < visibleSize && running; ++index)
        {
            unsigned int packed = visibleChunks[index];
            if (gRenderer->CBuffCall((packed & 0xffffff) * 3 + layer + mDisplayList, running, packed >> 24))
            {
                running = false;
            }
        }
        std::vector<int> ids;
        ids.reserve(visibleSize - index);
        for (; index < visibleSize; ++index)
            ids.push_back((int)(visibleChunks[index] & 0xffffff) * 3 + layer + mDisplayList);
        if (!ids.empty())
            gRenderer->CBuffCallRun(&ids[0], (int)ids.size());
#else
        for (unsigned int index = 0; index < visibleSize; ++index)
        {
            unsigned int packed = visibleChunks[index];
            if (gRenderer->CBuffCall((packed & 0xffffff) * 3 + layer + mDisplayList, running, packed >> 24))
            {
                running = false;
            }
        }
#endif
    }

    GlStateManager::popMatrix();
    mMinecraft->mGameRenderer->turnOffLightLayer(partialTick);

    return visibleCount;
}

void LevelRenderer::render(boost::shared_ptr<Entity> camera, int layer, double partialTick, bool sort)
{
    int dimension = mMinecraft->mPlayer->mUserIndex;

#ifdef MINECRAFT_PC
    if (mActivePlayers[dimension] != activePlayers() || mMinecraft->mOptions->mViewDistance != mLastViewDistance
        || ChunkGridWidth(activePlayers()) != mChunkGridWidth)
#else
    if (mActivePlayers[dimension] != activePlayers() || mMinecraft->mOptions->mViewDistance != mLastViewDistance)
#endif
    {
        allChanged();
    }

    if (layer == 0)
    {
        for (int counter = 0; counter < 5; ++counter)
        {
            mRenderCounters[counter] = 0;
        }
    }

    double deltaX = camera->mX - mLastSortX[dimension];
    double deltaY = camera->mY - mLastSortY[dimension];
    double deltaZ = camera->mZ - mLastSortZ[dimension];

    if (deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ > 16.0)
    {
        mLastSortX[dimension] = camera->mX;
        mLastSortY[dimension] = camera->mY;
        mLastSortZ[dimension] = camera->mZ;

        resortChunks(Mth::floor(camera->mX), Mth::floor(camera->mY), Mth::floor(camera->mZ), -1);
    }

    Lighting::turnOff();

    renderChunks(0, mClipChunks[dimension].length, layer, partialTick);
}

void LevelRenderer::renderSky(float partialTick)
{
    if (mMinecraft->mLevel->mDimension->getType() == DimensionType::THE_END)
    {
        GlStateManager::disableFog();
        GlStateManager::disableAlphaTest();
        GlStateManager::enableBlend();
        GlStateManager::blendFunc(4, 5);
        Lighting::turnOff();
        GlStateManager::depthMask(false);

        mTextures->bindTexture(&sEndSkyLocation);

        BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
        builder->setMipmapEnable(false);

        for (int face = 0; face < 6; ++face)
        {
            GlStateManager::pushMatrix();

            if (face == 1)
            {
                GlStateManager::rotatef(90.0f, 1.0f, 0.0f, 0.0f);
            }

            if (face == 2)
            {
                GlStateManager::rotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            }

            if (face == 3)
            {
                GlStateManager::rotatef(180.0f, 1.0f, 0.0f, 0.0f);
            }

            if (face == 4)
            {
                GlStateManager::rotatef(90.0f, 0.0f, 0.0f, 1.0f);
            }

            if (face == 5)
            {
                GlStateManager::rotatef(-90.0f, 0.0f, 0.0f, 1.0f);
            }

            builder->begin();
            builder->color(0x282828);
            builder->vertexUV(-100.0f, -100.0f, -100.0f, 0.0f, 0.0f);
            builder->vertexUV(-100.0f, -100.0f, 100.0f, 0.0f, 16.0f);
            builder->vertexUV(100.0f, -100.0f, 100.0f, 16.0f, 16.0f);
            builder->vertexUV(100.0f, -100.0f, -100.0f, 16.0f, 0.0f);
            builder->end();

            GlStateManager::popMatrix();
        }

        builder->setMipmapEnable(true);
        GlStateManager::depthMask(true);
        GlStateManager::enableTexture();
        GlStateManager::enableAlphaTest();
        return;
    }

    bool biomeNotNatural = mMinecraft->mPlayer
                           && !mMinecraft->mLevel->getBiome(BlockPos(mMinecraft->mPlayer))->isNatural();

    if (!mMinecraft->mLevel->mDimension->isNaturalDimension() || biomeNotNatural)
    {
        return;
    }

    GlStateManager::disableTexture();

    int userIndex = mMinecraft->mPlayer->mUserIndex;

    if (!mLevels[userIndex]->getGameRules()->getBoolean(GameRules::RULE_DAYLIGHT))
    {
        partialTick = 0.0f;
    }

    Vec3* skyColor = mLevels[userIndex]->getSkyColor(mMinecraft->getCameraEntity(), partialTick);
    float red = (float)skyColor->x;
    float green = (float)skyColor->y;
    float blue = (float)skyColor->z;

    if (mMinecraft->mOptions->mAnaglyph3d)
    {
        float red30 = red * 30.0f;
        float newBlue = (blue * 70.0f + red30) / 100.0f;
        float newRed = (blue * 11.0f + (green * 59.0f + red30)) / 100.0f;
        float newGreen = (green * 70.0f + red30) / 100.0f;
        red = newRed;
        green = newGreen;
        blue = newBlue;
    }

    GlStateManager::color3f(red, green, blue);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    GlStateManager::depthMask(false);
    GlStateManager::enableFog();
    GlStateManager::color3f(red, green, blue);
    GlStateManager::callList(mSkyList);
    GlStateManager::disableFog();
    GlStateManager::disableAlphaTest();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    Lighting::turnOff();

    float* sunriseColors = mLevels[userIndex]->mDimension->getSunriseColor((float)mLevels[userIndex]->getTimeOfDay(partialTick), partialTick);

    if (sunriseColors != 0)
    {
        GlStateManager::disableTexture();
        GlStateManager::shadeModel(0);
        GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
        GlStateManager::pushMatrix();
        GlStateManager::rotatef(90.0f, 1.0f, 0.0f, 0.0f);

        float faceAngle = (Mth::sin(mLevels[userIndex]->getSunAngle(partialTick)) >= 0.0f) ? 0.0f : 180.0f;
        GlStateManager::rotatef(faceAngle, 0.0f, 0.0f, 1.0f);
        GlStateManager::rotatef(90.0f, 0.0f, 0.0f, 1.0f);

        float sunriseR = sunriseColors[0];
        float sunriseG = sunriseColors[1];
        float sunriseB = sunriseColors[2];

        if (mMinecraft->mOptions->mAnaglyph3d)
        {
            float r30 = sunriseR * 30.0f;
            float newB = (sunriseB * 70.0f + r30) / 100.0f;
            float newG = (sunriseG * 70.0f + r30) / 100.0f;
            float newR = (sunriseB * 11.0f + (sunriseG * 59.0f + r30)) / 100.0f;
            sunriseR = newR;
            sunriseG = newG;
            sunriseB = newB;
        }

        builder->begin(2);
        builder->color(sunriseR, sunriseG, sunriseB, sunriseColors[3]);
        builder->vertex(0.0f, 100.0f, 0.0f);
        builder->color(sunriseColors[0], sunriseColors[1], sunriseColors[2], 0.0f);

        for (int segment = 0; segment <= 16; ++segment)
        {
            float angle = (float)segment * 3.14159274f * 2.0f / 16.0f;
            float sinA = Mth::sin(angle);
            float cosA = Mth::cos(angle);
            builder->vertex(sinA * 120.0f, cosA * 120.0f, -(cosA * 40.0f * sunriseColors[3]));
        }

        builder->end();
        GlStateManager::popMatrix();
        GlStateManager::shadeModel(0);
    }

    float sunAngleDegrees = (float)(mLevels[userIndex]->getSunAngle(partialTick) * 57.2957763671875);
    float moonAngleDegrees = fmodf(sunAngleDegrees + 180.0f, 360.0f);

    GlStateManager::enableTexture();
    GlStateManager::blendFunc(4, 1);
    GlStateManager::pushMatrix();

    float rainLevel = mLevels[userIndex]->getRainLevel(partialTick);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f - rainLevel);
    GlStateManager::translatef(0.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(-90.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(mLevels[userIndex]->getTimeOfDay(partialTick) * 360.0f, 1.0f, 0.0f, 0.0f);

    if (sunAngleDegrees <= 125.0f || 235.0f <= sunAngleDegrees)
    {
        MemSect(0x1f);
        mTextures->bindTexture(&sSunLocation);
        MemSect(0);

        builder->begin();
        builder->vertexUV(-30.0f, 100.0f, -30.0f, 1.0f, 1.0f);
        builder->vertexUV(30.0f, 100.0f, -30.0f, 0.0f, 1.0f);
        builder->vertexUV(30.0f, 100.0f, 30.0f, 0.0f, 0.0f);
        builder->vertexUV(-30.0f, 100.0f, 30.0f, 1.0f, 0.0f);
        builder->end();
    }

    if (moonAngleDegrees <= 125.0f || 235.0f <= moonAngleDegrees)
    {
        mTextures->bindTexture(&sMoonPhasesLocation);

        int moonPhase = mLevels[userIndex]->getMoonPhase();
        int column = moonPhase & 3;
        int row = (moonPhase >> 2) % 2;
        float u0 = (float)column * 0.25f;
        float u1 = (float)(column + 1) * 0.25f;
        float v0 = (float)row * 0.5f;
        float v1 = (float)(row + 1) * 0.5f;

        builder->begin();
        builder->vertexUV(-20.0f, -100.0f, 20.0f, u1, v1);
        builder->vertexUV(20.0f, -100.0f, 20.0f, u0, v1);
        builder->vertexUV(20.0f, -100.0f, -20.0f, u0, v0);
        builder->vertexUV(-20.0f, -100.0f, -20.0f, u1, v0);
        builder->end();
    }

    GlStateManager::disableTexture();

    float starBrightness = mLevels[userIndex]->getStarBrightness(partialTick) * (1.0f - rainLevel);

    if (starBrightness > 0.0f)
    {
        GlStateManager::color4f(starBrightness, starBrightness, starBrightness, starBrightness);
        GlStateManager::callList(mStarList);
    }

    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::disableBlend();
    GlStateManager::enableAlphaTest();
    GlStateManager::enableFog();
    GlStateManager::popMatrix();
    GlStateManager::disableTexture();
    GlStateManager::color3f(0.0f, 0.0f, 0.0f);

    if (!mLevels[userIndex]->mDimension->hasGround())
    {
        GlStateManager::color3f(red, green, blue);
        GlStateManager::enableTexture();
        GlStateManager::depthMask(true);
    }
    else
    {
        GlStateManager::color3f(red * 0.2f + 0.04f, green * 0.2f + 0.04f, blue * 0.6f + 0.1f);
        GlStateManager::enableTexture();
        GlStateManager::depthMask(true);
    }
}

void LevelRenderer::renderHaloRing(float partialTick)
{
    if (!mMinecraft->mLevel->mDimension->isNaturalDimension())
    {
        return;
    }

    GlStateManager::disableAlphaTest();
    GlStateManager::disableTexture();
    GlStateManager::depthMask(false);
    GlStateManager::enableFog();

    int dimension = mMinecraft->mPlayer->mUserIndex;
    Vec3* skyColor = mLevels[dimension]->getSkyColor(mMinecraft->getCameraEntity(), partialTick);
    float brightness = ((float)skyColor->x + (float)skyColor->x + (float)skyColor->z + (float)skyColor->y
                        + (float)skyColor->y + (float)skyColor->y)
                           / 6.0f * 0.4f
                       + 0.6f;

    GlStateManager::color3f(brightness, brightness, brightness);
    GlStateManager::fogMode(1);

    float fogStart = GlStateManager::getFogStart();
    float fogEnd = GlStateManager::getFogEnd();

    GlStateManager::fogStart(100.0f);
    GlStateManager::fogEnd(20.0f);

    Lighting::turnOn();
    GlStateManager::depthMask(false);

    mTextures->bindTexture(L"misc/haloRing");

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    bool mipmap = builder->setMipmapEnable(true);

    GlStateManager::pushMatrix();
    GlStateManager::rotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(90.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::callList(mHaloRingList);
    GlStateManager::popMatrix();

    builder->setMipmapEnable(mipmap);

    GlStateManager::depthMask(true);
    GlStateManager::enableTexture();
    GlStateManager::enableAlphaTest();
    GlStateManager::disableFog();
    GlStateManager::fogStart(fogStart);
    GlStateManager::fogEnd(fogEnd);
}

void LevelRenderer::renderAdvancedClouds(float partialTick, int pass, double x, double y, double z)
{
    GlStateManager::multiTexCoord2f(0.0f, 0.0f);
    gRenderer->StateSetEnableViewportClipPlanes(true);

    int savedTick = mRenderTick;
    int userIndex = mMinecraft->mPlayer->mUserIndex;
    Level* level = mLevels[userIndex];

    if (GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(0xb), -1))
    {
        savedTick = mDebugRenderTick;
    }

    float currentTime = (float)mRenderTick + partialTick;

    if (gCloudDriftDelta != 0.0f)
    {
        gCloudDriftAccum = gCloudDriftAccum + gCloudDriftDelta;
    }

    double driftAccum = (double)gCloudDriftAccum;
    double cellX;
    double cellZ;

    if (!gCloudDriftLatched)
    {
        if (gCloudPrevTime != -1.0f)
        {
            gCloudDriftAccum = (float)(driftAccum + ((double)currentTime - (double)gCloudPrevTime));
            driftAccum = (double)gCloudDriftAccum;
            gCloudPrevTime = -1.0f;
        }

        cellZ = z / 12.0 + 0.33000001311302185;
        cellX = (((double)currentTime - driftAccum) * 0.029999999329447746 + x) / 12.0;
    }
    else
    {
        if (gCloudPrevTime == -1.0f)
        {
            gCloudPrevTime = currentTime;
        }

        cellZ = z / 12.0 + 0.33000001311302185;
        cellX = (((double)gCloudPrevTime - driftAccum) * 0.029999999329447746 + x) / 12.0;
    }

    float cloudRelativeY = level->mDimension->getCloudHeight() - (float)y + 0.33f;

    int sectorX = Mth::floor(cellX * (1.0 / 2048.0));
    int sectorZ = Mth::floor(cellZ * (1.0 / 2048.0));
    cellX -= (double)(sectorX << 11);
    cellZ -= (double)(sectorZ << 11);

    bool cameraInCloudBand = (-5.0 < cloudRelativeY) && (cloudRelativeY <= 5.0);

    if (cameraInCloudBand)
    {
        GlStateManager::disableCull();
    }
    else
    {
        GlStateManager::enableCull();
    }

    MemSect(0x1f);
    mTextures->bindTexture(&sCloudsLocation);
    MemSect(0);

    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);

    Vec3* cloudColor = level->getCloudColor(partialTick);
    float red = (float)cloudColor->x;
    float green = (float)cloudColor->y;
    float blue = (float)cloudColor->z;
    float alpha = 1.0f;

    if (mMinecraft->mOptions->mAnaglyph3d)
    {
        float red30 = red * 30.0f;
        float newBlue = (blue * 70.0f + red30) / 100.0f;
        float newRed = (blue * 11.0f + (green * 59.0f + red30)) / 100.0f;
        float newGreen = (green * 70.0f + red30) / 100.0f;
        red = newRed;
        green = newGreen;
        blue = newBlue;
    }

    float wrapAnchorX = (float)Mth::floor(cellX) * (1.0f / 256.0f);
    float wrapAnchorZ = (float)Mth::floor(cellZ) * (1.0f / 256.0f);

    while (wrapAnchorX < 1.0f)
    {
        wrapAnchorX = wrapAnchorX + 1.0f;
    }

    while (wrapAnchorZ < 1.0f)
    {
        wrapAnchorZ = wrapAnchorZ + 1.0f;
    }

    float fracX = (float)(cellX - (double)Mth::floor(cellX));
    float fracZ = (float)(cellZ - (double)Mth::floor(cellZ));

    int gridRadius = 3;

    if (activePlayers() > 2)
    {
        gridRadius = 2;
    }

    GlStateManager::scalef(12.0f, 1.0f, 12.0f);
    FrustumData* frustum = Frustum::getFrustum();

    int gridMin = 1 - gridRadius;

    for (int blendPass = 0; blendPass < 2; ++blendPass)
    {
        if (blendPass == 0)
        {
            GlStateManager::blendFunc(0, 1);
        }
        else
        {
            GlStateManager::blendFunc(4, 5);
        }

        if (gridMin <= gridRadius)
        {
            for (int cellRow = gridMin; cellRow <= gridRadius; ++cellRow)
            {
                float rowWorld = (float)(cellRow * 8);

                for (int cellCol = gridMin; cellCol <= gridRadius; ++cellCol)
                {
                    float colWorld = (float)(cellCol * 8);
                    float boxX = rowWorld - fracX;
                    float boxZ = colWorld - fracZ;

                    if (frustum->cubeInFrustum(boxX, cloudRelativeY, boxZ, boxX + 8.0f, cloudRelativeY + 4.0f,
                                               boxZ + 8.0f))
                    {
                        GlStateManager::matrixMode(2);
                        GlStateManager::loadIdentity();
                        GlStateManager::translatef(rowWorld * (1.0f / 256.0f) + wrapAnchorX,
                                                   colWorld * (1.0f / 256.0f) + wrapAnchorZ, 0.0f);
                        GlStateManager::matrixMode(0);
                        GlStateManager::pushMatrix();
                        GlStateManager::translatef(boxX, cloudRelativeY, boxZ);
                        GlStateManager::color4f(red, green, blue, alpha);

                        if (!cameraInCloudBand)
                        {
                            GlStateManager::callList(mCloudLists + 6);
                        }
                        else
                        {
                            GlStateManager::callList(mCloudLists + 0);
                            GlStateManager::callList(mCloudLists + 1);

                            if (cellRow >= 0)
                            {
                                if (cellRow < 2)
                                {
                                    GlStateManager::translatef(0.0009765625f, 0.0f, 0.0f);
                                    GlStateManager::callList(mCloudLists + 2);
                                    GlStateManager::translatef(-0.0009765625f, 0.0f, 0.0f);
                                    GlStateManager::callList(mCloudLists + 3);
                                }
                                else
                                {
                                    GlStateManager::callList(mCloudLists + 2);
                                }
                            }
                            else
                            {
                                GlStateManager::callList(mCloudLists + 3);
                            }

                            if (cellCol >= 0)
                            {
                                if (cellCol < 2)
                                {
                                    GlStateManager::translatef(0.0f, 0.0f, 0.0009765625f);
                                    GlStateManager::callList(mCloudLists + 4);
                                    GlStateManager::translatef(0.0f, 0.0f, -0.0009765625f);
                                    GlStateManager::callList(mCloudLists + 5);
                                }
                                else
                                {
                                    GlStateManager::callList(mCloudLists + 4);
                                }
                            }
                            else
                            {
                                GlStateManager::callList(mCloudLists + 5);
                            }
                        }

                        GlStateManager::popMatrix();
                        GlStateManager::matrixMode(2);
                        GlStateManager::loadIdentity();
                        GlStateManager::matrixMode(0);
                    }
                }
            }
        }
    }

    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::disableBlend();
    GlStateManager::enableCull();

    if (gApp.DebugSettingsOn() && !GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(0xb), -1))
    {
        mDebugRenderTick = savedTick;
    }

    gRenderer->StateSetEnableViewportClipPlanes(false);
}

void LevelRenderer::renderClouds(float partialTick, int pass, double x, double y, double z)
{
    int savedTick = mRenderTick;
    int userIndex = mMinecraft->mPlayer->mUserIndex;

    if (!GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_Clouds))
    {
        return;
    }

    bool biomeNotNatural = mMinecraft->mPlayer
                           && !mMinecraft->mLevel->getBiome(BlockPos(mMinecraft->mPlayer))->isNatural();

    if (!mMinecraft->mLevel->mDimension->isNaturalDimension() || biomeNotNatural)
    {
        return;
    }

    if (mMinecraft->mOptions->mFancyGraphics)
    {
        renderAdvancedClouds(partialTick, pass, x, y, z);
        return;
    }

    if (GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(0xb), -1))
    {
        savedTick = mDebugRenderTick;
    }

    GlStateManager::disableCull();

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    mTextures->bindTexture(&sCloudsLocation);
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);

    Level* level = mLevels[userIndex];
    Vec3* cloudColor = level->getCloudColor(partialTick);
    float red = (float)cloudColor->x;
    float green = (float)cloudColor->y;
    float blue = (float)cloudColor->z;

    if (mMinecraft->mOptions->mAnaglyph3d)
    {
        float red30 = red * 30.0f;
        float newBlue = (blue * 70.0f + red30) / 100.0f;
        float newRed = (blue * 11.0f + (green * 59.0f + red30)) / 100.0f;
        float newGreen = (green * 70.0f + red30) / 100.0f;
        red = newRed;
        green = newGreen;
        blue = newBlue;
    }

    x = (double)((float)mRenderTick + partialTick) * 0.029999999329447746 + x;

    int gridX = Mth::floor(x * 0.00048828125);
    int gridZ = Mth::floor(z * 0.00048828125);
    float localX = (float)((x - (double)(gridX << 11)) * 0.00048828125);
    float localZ = (float)((z - (double)(gridZ << 11)) * 0.00048828125);
    float cloudY = level->mDimension->getCloudHeight() - (float)y + 0.33f;

    builder->begin();
    builder->color(red, green, blue, 0.8f);

    for (int cellX = -256; cellX < 256; cellX += 32)
    {
        float x0 = (float)cellX;
        float x1 = (float)(cellX + 32);
        float u0 = x0 * 0.00048828125f + localX;
        float u1 = x1 * 0.00048828125f + localX;

        for (int cellZ = -256; cellZ < 256; cellZ += 32)
        {
            float z0 = (float)cellZ;
            float z1 = (float)(cellZ + 32);
            float v0 = z0 * 0.00048828125f + localZ;
            float v1 = z1 * 0.00048828125f + localZ;

            builder->vertexUV(x0, cloudY, z1, u0, v1);
            builder->vertexUV(x1, cloudY, z1, u1, v1);
            builder->vertexUV(x1, cloudY, z0, u1, v0);
            builder->vertexUV(x0, cloudY, z0, u0, v0);
        }
    }

    builder->end();

    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::disableBlend();
    GlStateManager::enableCull();

    if (gApp.DebugSettingsOn() && !GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(0xb), -1))
    {
        mDebugRenderTick = savedTick;
    }
}

bool LevelRenderer::isInCloud(double x, double y, double z, float partialTick)
{
    return false;
}

void LevelRenderer::setGlobalChunkFlag(int index, unsigned char mask, unsigned char shift)
{
    unsigned char bits = (unsigned char)(mask << shift);

    if (index == -1)
    {
        return;
    }

    mGlobalChunkFlags[index] |= bits;
}

int LevelRenderer::getGlobalIndexForChunk(int x, int y, int z, Level* level)
{
    return getChunkIndex(getDimensionIndexFromId(level->mDimension->getType()->getId()), x >> 4, y >> 4, z >> 4);
}

bool LevelRenderer::getGlobalChunkFlag(int x, int y, int z, Level* level, unsigned char mask, unsigned char shift)
{
    unsigned char bits = (unsigned char)(mask << shift);
    int index = getGlobalIndexForChunk(x, y, z, level);

    if (index == -1)
    {
        return false;
    }

    return (bits & mGlobalChunkFlags[index]) == bits;
}

unsigned int LevelRenderer::updateDirtyChunks(unsigned int* count)
{
    std::vector<Chunk*> emptyChunks;

    ClipChunk* nearest = 0;
    int nearestDistance = 0x7fffffff;
    int nearCount = 0;

    unsigned int commandBufferSize = (unsigned int)gRenderer->CBuffSize(-1);
#ifdef MINECRAFT_PC
    const unsigned int commandBufferBudget = CommandBufferBudget();
#endif
    float commandBufferBytes = (float)commandBufferSize;

    InfoBarManager::SetCommandBufferSize(commandBufferBytes / 115343360.0f);
    PIXAddNamedCounter((float)(int)(commandBufferBytes / 1048576.0f), "Command buffer allocations");

    EnterCriticalSection(&mChunkMutex);

    int* dirtyChunk;

    while ((dirtyChunk = mDirtyChunkStack.Pop()) != 0)
    {
        if ((int)(intptr_t)dirtyChunk > 0)
        {
            if ((int)(intptr_t)dirtyChunk > 1)
            {
                setGlobalChunkFlag((int)(intptr_t)dirtyChunk - 2, 2, 0);
            }

            mHasDirtyChunks = true;
        }
    }

    if (mHasDirtyChunks)
    {
        mLastRebuildTime = System::processTimeInMilliSecs();

        PIXBeginNamedEvent(0.0f, "Finding nearest chunk\n");

        for (int player = 0; player < 4; ++player)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = mMinecraft->mLocalPlayers[player];

            if (!localPlayer)
            {
                continue;
            }

            if (mClipChunks[player].data == 0)
            {
                continue;
            }

            if (mLevels[player] == 0)
            {
                continue;
            }

            if ((int)mClipChunks[player].length != mChunkGridWidth * mChunkGridDepth * 16)
            {
                continue;
            }

            int cameraX = (int)localPlayer->mX;
            int cameraY = (int)localPlayer->mY;
            int cameraZ = (int)localPlayer->mZ;

            for (int gridZ = 0; gridZ < mChunkGridDepth; ++gridZ)
            {
                for (int gridY = 0; gridY < 16; ++gridY)
                {
                    for (int gridX = 0; gridX < mChunkGridWidth; ++gridX)
                    {
                        ClipChunk* clipChunk =
                            &mClipChunks[player].data[(gridZ * mChunkGridHeight + gridY) * mChunkGridWidth + gridX];

                        int dx = clipChunk->mX - cameraX;
                        int dy = clipChunk->mY - cameraY;
                        int dz = clipChunk->mZ - cameraZ;
                        int globalIndex = clipChunk->mGlobalIndex;
                        int distance = dx * dx + dy * dy + dz * dz;
                        int squishedDistance = dx * dx + dy * dy * 4 + dz * dz;

                        if (globalIndex == -1)
                        {
                            continue;
                        }

                        unsigned char flags = mGlobalChunkFlags[globalIndex];
                        if ((flags & 2) == 0)
                        {
                            continue;
                        }

#ifdef MINECRAFT_PC
                        if (commandBufferSize >= commandBufferBudget && (flags & 1) == 0 && distance >= 400)
#else
                        if (commandBufferSize >= 115343360u && (flags & 1) == 0 && distance >= 400)
#endif
                        {
                            continue;
                        }

                        if (squishedDistance < nearestDistance)
                        {
                            Chunk* chunk = clipChunk->mChunk;
                            LevelChunk* levelChunk = mLevels[player]->getChunkAt(
                                BlockPos(chunk->mX, 0, chunk->mDimension));

                            if (!levelChunk->isRenderChunkEmpty(gridY * 16))
                            {
                                nearestDistance = squishedDistance;
                                nearest = clipChunk;
                            }
                            else
                            {
                                chunk->clearDirty();
                                mGlobalChunkFlags[globalIndex] |= 0x1c;
                                emptyChunks.push_back(chunk);
                            }
                        }

                        if (distance < 400)
                        {
                            ++nearCount;
                        }
                    }
                }
            }
        }

        PIXEndNamedEvent();
    }

    if (nearest == 0)
    {
        long long elapsed = System::processTimeInMilliSecs() - mLastRebuildTime;

        mHasDirtyChunks = elapsed > 250;

        LeaveCriticalSection(&mChunkMutex);

#ifdef MINECRAFT_PC
        if (!mHasDirtyChunks)
            Sleep(1);
#endif
        return 0;
    }

    Chunk* chunk = nearest->mChunk;

    PIXBeginNamedEvent(0.0f, "Rebuilding near chunk %d %d %d", chunk->mX, chunk->mZ, chunk->mDimension);

    if (nearCount > 0)
    {
        gRenderer->CBuffDeferredModeStart();
    }

    chunk->clearDirty();

    static Chunk rebuildChunk;

    rebuildChunk.makeCopyForRebuild(chunk);

    LeaveCriticalSection(&mChunkMutex);

    rebuildChunk.rebuild(count);

    PIXEndNamedEvent();

    for (int remaining = (int)emptyChunks.size(); remaining > 0; --remaining)
    {
        Chunk* emptyChunk = emptyChunks[remaining - 1];

        mDestroyedBlocks->updatedChunkAt(emptyChunk->mLevel,
                                         BlockPos(emptyChunk->mX, emptyChunk->mZ, emptyChunk->mDimension), remaining);
    }

    if (nearCount > 1)
    {
        mDestroyedBlocks->updatedChunkAt(chunk->mLevel, BlockPos(chunk->mX, chunk->mZ, chunk->mDimension), nearCount);

        return 1;
    }

    if (nearCount > 0 && getGlobalChunkFlag(chunk->mX, chunk->mZ, chunk->mDimension, chunk->mLevel, 2, 0))
    {
        mDestroyedBlocks->updatedChunkAt(chunk->mLevel, BlockPos(chunk->mX, chunk->mZ, chunk->mDimension), 2);

        return 1;
    }

    mDestroyedBlocks->updatedChunkAt(chunk->mLevel, BlockPos(chunk->mX, chunk->mZ, chunk->mDimension), nearCount);

    return 0;
}

void LevelRenderer::renderHit(boost::shared_ptr<Player> player, const HitResult* hit, int pass,
                              not_null_ptr<ItemInstance> item, float partialTick)
{
    GlStateManager::enableBlend();
    GlStateManager::enableAlphaTest();
    GlStateManager::blendFunc(4, 1);

    float phase = sinf((float)Minecraft::currentTimeMillis() / 100.0f);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, (phase * 0.2f + 0.4f) * 0.5f);

    if (pass != 0)
    {
        if (!item->isEmpty())
        {
            GlStateManager::blendFunc(4, 5);

            float phase1 = sinf((float)Minecraft::currentTimeMillis() / 100.0f);
            float rgb = phase1 * 0.2f + 0.8f;

            float phase2 = sinf((float)Minecraft::currentTimeMillis() / 200.0f);
            float alpha = phase2 * 0.2f + 0.5f;

            GlStateManager::color4f(rgb, rgb, rgb, alpha);
            mTextures->bindTexture(&BLOCKS_LOCATION);
        }
    }

    GlStateManager::disableBlend();
    GlStateManager::disableAlphaTest();
}

void LevelRenderer::renderDestroyAnimation(Tesselator* tesselator, boost::shared_ptr<Player> player, float partialTick)
{
    double camX = (player->mX - player->mXOld) * partialTick + player->mXOld;
    double camY = (player->mY - player->mYOld) * partialTick + player->mYOld;
    double camZ = (player->mZ - player->mZOld) * partialTick + player->mZOld;
    int userIndex = mMinecraft->mPlayer->mUserIndex;
    if (mDestroyingBlocks.empty())
        return;

    BufferBuilder* builder = tesselator->getBuilder();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(8, 2);
    mTextures->bindTexture(&BLOCKS_LOCATION);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 0.5f);
    GlStateManager::pushMatrix();
    GlStateManager::disableAlphaTest();
    GlStateManager::polygonOffset(-3.0f, -3.0f);
    GlStateManager::enablePolygonOffset();
    GlStateManager::enableAlphaTest();
    builder->begin();
    builder->offset(-(float)camX, -(float)camY, -(float)camZ);
    builder->noColor();

    for (boost::unordered_map<int, BlockDestructionProgress*>::iterator entry = mDestroyingBlocks.begin();
         entry != mDestroyingBlocks.end(); ++entry)
    {
        BlockDestructionProgress* progress = entry->second;
        double dx = progress->getPos().x - camX;
        double dy = progress->getPos().y - camY;
        double dz = progress->getPos().z - camZ;
        Block* block = mLevels[userIndex]->getBlockState(progress->getPos())->getBlock();
        if (block == Blocks::chest || block == Blocks::ender_chest || block == Blocks::trapped_chest
            || block == Blocks::standing_sign || block == Blocks::skull || block == Blocks::wall_sign)
        {
            continue;
        }

        if (dx * dx + dy * dy + dz * dz < 1024.0)
        {
            int rendererIndex = mMinecraft->mPlayer->mUserIndex;
            const BlockState* state = mMinecraft->mLevel->getBlockState(progress->getPos());
            state->getBlock();
            mBlockRenderers[rendererIndex]->tesselateInWorldFixedTexture(state, progress->getPos(),
                                                                         mDestroyStages[progress->getProgress()]);
        }
    }

    builder->end();
    builder->offset(0.0f, 0.0f, 0.0f);
    GlStateManager::disableAlphaTest();
    GlStateManager::polygonOffset(0.0f, 0.0f);
    GlStateManager::disablePolygonOffset();
    GlStateManager::enableAlphaTest();
    GlStateManager::depthMask(true);
    GlStateManager::popMatrix();
    GlStateManager::disableBlend();
}

void LevelRenderer::renderLineBox(const AABB* box, int color)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    builder->begin(5);
    builder->color(color);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->end();

    builder->begin(5);
    builder->color(color);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->end();

    builder->begin(4);
    builder->color(color);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->end();
}

void LevelRenderer::renderHitOutline(boost::shared_ptr<Player> player, const HitResult* hit, int pass,
                                     float partialTick)
{
    if (pass != 0 || hit->mType != HitResult::BLOCK)
    {
        return;
    }

    int userIndex = mMinecraft->mPlayer->mUserIndex;

    if (GameSettings::GetGameSettings(userIndex, eGameSetting_DisplayHUD))
    {
        GlStateManager::enableBlend();
        GlStateManager::blendFunc(4, 5);
        GlStateManager::color4f(0.0f, 0.0f, 0.0f, 0.4f);
        GlStateManager::lineWidth(2.0f);
        GlStateManager::disableTexture();
        GlStateManager::depthMask(false);

        const BlockState* state = mMinecraft->mLevel->getBlockState(hit->getBlockPos());
        Block* block = state->getBlock();

        double camX = (player->mX - player->mXOld) * partialTick + player->mXOld;
        double camY = (player->mY - player->mYOld) * partialTick + player->mYOld;
        double camZ = (player->mZ - player->mZOld) * partialTick + player->mZOld;

        AABB* box = block->getOutlineAABB(state, mLevels[userIndex], hit->getBlockPos())
                        ->grow(0.0020000000949949026, 0.0020000000949949026, 0.0020000000949949026)
                        ->move(-camX, -camY, -camZ);

        renderLineBox(box, 0xffffff);

        GlStateManager::depthMask(true);
        GlStateManager::enableTexture();
        GlStateManager::disableBlend();
    }
}

void LevelRenderer::renderColoredLineBox(const AABB* box, int color, bool splitLargeFaces)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    builder->begin(5);
    builder->color(color);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->end();

    builder->begin(5);
    builder->color(color);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->end();

    builder->begin(4);
    builder->color(color);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->end();

    if (!splitLargeFaces)
    {
        return;
    }

    if (fabs(box->mMaxX) - fabs(box->mMinX) > 1.5)
    {
        double midX = (box->mMaxX - box->mMinX) * 0.5 + box->mMinX;

        builder->begin(5);
        builder->color(color);
        builder->vertex((float)midX, (float)box->mMinY, (float)box->mMinZ);
        builder->vertex((float)midX, (float)box->mMinY, (float)box->mMaxZ);
        builder->vertex((float)midX, (float)box->mMaxY, (float)box->mMaxZ);
        builder->vertex((float)midX, (float)box->mMaxY, (float)box->mMinZ);
        builder->vertex((float)midX, (float)box->mMinY, (float)box->mMinZ);
        builder->end();
    }

    if (fabs(box->mMaxZ) - fabs(box->mMinZ) > 1.5)
    {
        double midZ = (box->mMaxZ - box->mMinZ) * 0.5 + box->mMinZ;

        builder->begin(5);
        builder->color(color);
        builder->vertex((float)box->mMinX, (float)box->mMinY, (float)midZ);
        builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)midZ);
        builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)midZ);
        builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)midZ);
        builder->vertex((float)box->mMinX, (float)box->mMinY, (float)midZ);
        builder->end();
    }
}

void LevelRenderer::renderLine(std::vector<GameRenderer::PositionStruct>* positions, std::vector<int>* colors,
                               std::vector<int>* alphas, float width)
{
    float halfWidth = width * 0.5f;
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    GlStateManager::disableCull();
    GlStateManager::disableTexture();
    GlStateManager::lineWidth(2.0f);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::enableBlend();
    GlStateManager::blendFuncSeparate(4, 5, 1, 0);
    GlStateManager::depthMask(false);
    GlStateManager::disableAlphaTest();

    builder->begin(1);

    for (unsigned int index = 0; index < positions->size(); ++index)
    {
        GameRenderer::PositionStruct position = positions->at(index);
        int color = colors->at(index);
        int alpha = alphas->at(index);
        float deltaZ;
        float deltaX;

        if (index == 0)
        {
            deltaZ = 0.0f;
            deltaX = 0.0f;

            if (!positions->empty())
            {
                GameRenderer::PositionStruct next = positions->at(1);
                deltaZ = position.z - next.z;
                deltaX = position.x - next.x;
            }
        }
        else
        {
            GameRenderer::PositionStruct previous = positions->at(index - 1);
            deltaZ = previous.z - position.z;
            deltaX = previous.x - position.x;
        }

        float angle = atan2f(deltaZ, deltaX) + 1.5707964f;
        float cosAngle = cosf(angle);
        float sinAngle = sinf(angle);
        float oppositeAngle = angle + 3.1415927f;
        float cosOpposite = cosf(oppositeAngle);
        float oppositeOffsetZ = halfWidth * sinf(oppositeAngle);

        builder->color(color, alpha);
        builder->vertex(halfWidth * cosAngle + position.x, position.y, halfWidth * sinAngle + position.z);
        builder->vertex(halfWidth * cosOpposite + position.x, position.y, position.z + oppositeOffsetZ);
    }

    builder->end();

    GlStateManager::enableCull();
    GlStateManager::enableAlphaTest();
    GlStateManager::depthMask(true);
    GlStateManager::enableTexture();
    GlStateManager::disableBlend();
}

void LevelRenderer::renderColoredBox(const AABB* box, int color, int alpha)
{
    GlStateManager::disableCull();

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    builder->begin(3);
    builder->color(color, alpha);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMaxZ);
    builder->end();

    GlStateManager::enableCull();
}

void LevelRenderer::renderStringInWorld(boost::shared_ptr<Entity> entity, const std::wstring& text, Vec3* position)
{
    float x = (float)position->x;
    float y = (float)position->y;
    float z = (float)position->z;
    float xRot = entity->mXRot;
    float yRot = entity->mYRot;
    Font* font = BlockEntityRenderDispatcher::instance->getFont();

    GlStateManager::pushMatrix();
    GlStateManager::translatef(x, y, z);
    GlStateManager::normal3f(0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(-yRot, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(xRot, 1.0f, 0.0f, 0.0f);
    GlStateManager::scalef(-GameRenderer::NAME_TAG_SCALE, -GameRenderer::NAME_TAG_SCALE, GameRenderer::NAME_TAG_SCALE);
    GlStateManager::depthMask(false);

    int halfWidth = font->width(text) / 2;

    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    GlStateManager::disableTexture();
    builder->begin();
    builder->color(0.0f, 0.0f, 0.0f, 0.25f);
    builder->vertex((float)(-(halfWidth + 1)), -1.0f, 0.0f);
    builder->vertex((float)(-(halfWidth + 1)), 9.0f, 0.0f);
    builder->vertex((float)(halfWidth + 1), 9.0f, 0.0f);
    builder->vertex((float)(halfWidth + 1), -1.0f, 0.0f);
    builder->end();
    GlStateManager::enableTexture();
    GlStateManager::depthMask(true);

    font->draw(text, -halfWidth, 0, 0xffffffff);

    GlStateManager::disableBlend();
    GlStateManager::popMatrix();
}

void LevelRenderer::renderStringOnPlane(boost::shared_ptr<Entity> entity, const std::wstring& text, Vec3* position,
                                        Vec3* axis)
{
    Font* font = BlockEntityRenderDispatcher::instance->getFont();

    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)position->x, (float)position->y, (float)position->z);
    GlStateManager::normal3f(0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(1.5707964f * 57.295776f, (float)axis->x, (float)axis->y, (float)axis->z);
    GlStateManager::scalef(-1.0f, -1.0f, 1.0f);
    GlStateManager::depthMask(false);

    int halfWidth = font->width(text) / 2;

    GlStateManager::enableBlend();
    GlStateManager::disableCull();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::depthMask(true);

    font->draw(text, -halfWidth, 0, 0xffffffff);

    GlStateManager::disableBlend();
    GlStateManager::enableCull();
    GlStateManager::popMatrix();
}

void LevelRenderer::renderDebugVolume(boost::shared_ptr<Entity> entity, const std::wstring& text, const AABB* box,
                                      int color, bool splitLargeFaces, bool depthTest, bool thickLines, bool raiseText)
{
    if (!depthTest)
    {
        GlStateManager::disableDepthTest();
    }

    GlStateManager::disableLighting();

    Vec3* centre = box->getCenter();

    if (raiseText)
    {
        centre->move(0.0, 1.0, 0.0);
    }

    renderStringInWorld(entity, text, centre);

    GlStateManager::disableTexture();

    int alpha = 0x3f;
    float width;

    if (thickLines)
    {
        width = 4.0f;
        alpha = 0x7f;
    }
    else
    {
        width = 2.0f;
    }

    GlStateManager::lineWidth(width);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::enableBlend();
    GlStateManager::blendFuncSeparate(4, 5, 1, 0);
    GlStateManager::depthMask(false);

    renderColoredLineBox(box, color, splitLargeFaces);
    renderColoredBox(box, color, alpha);

    GlStateManager::depthMask(true);
    GlStateManager::enableTexture();
    GlStateManager::disableBlend();

    if (!depthTest)
    {
        GlStateManager::enableDepthTest();
    }
}

void LevelRenderer::renderFancyTextDebugVolume(boost::shared_ptr<Entity> entity, const std::wstring& text,
                                               const AABB* box, int color, bool splitLargeFaces, bool depthTest,
                                               bool thickLines)
{
    if (!depthTest)
    {
        GlStateManager::disableDepthTest();
    }

    GlStateManager::disableLighting();

    Vec3* centre = box->getCenter();

    renderStringOnPlane(entity, text + L" TOP", Vec3::newTemp(centre->x, box->mMaxY, centre->z),
                        Vec3::newTemp(1.0, 0.0, 0.0));
    renderStringOnPlane(entity, text + L" BOTTOM", Vec3::newTemp(centre->x, box->mMinY, centre->z),
                        Vec3::newTemp(1.0, 0.0, 0.0));
    renderStringOnPlane(entity, text + L" NORTH", Vec3::newTemp(centre->x, centre->y, box->mMinZ),
                        Vec3::newTemp(0.0, 0.0, 0.0));
    renderStringOnPlane(entity, text + L" SOUTH", Vec3::newTemp(centre->x, centre->y, box->mMaxZ),
                        Vec3::newTemp(0.0, 0.0, 0.0));
    renderStringOnPlane(entity, text + L" EAST", Vec3::newTemp(box->mMaxX, centre->y, centre->z),
                        Vec3::newTemp(0.0, 1.0, 0.0));
    renderStringOnPlane(entity, text + L" WEST", Vec3::newTemp(box->mMinX, centre->y, centre->z),
                        Vec3::newTemp(0.0, -1.0, 0.0));

    GlStateManager::disableTexture();

    int alpha = 0x3f;
    float width;

    if (thickLines)
    {
        width = 4.0f;
        alpha = 0x7f;
    }
    else
    {
        width = 2.0f;
    }

    GlStateManager::lineWidth(width);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::enableBlend();
    GlStateManager::blendFuncSeparate(4, 5, 1, 0);
    GlStateManager::depthMask(false);

    renderColoredLineBox(box, color, splitLargeFaces);
    renderColoredBox(box, color, alpha);

    GlStateManager::depthMask(true);
    GlStateManager::enableTexture();
    GlStateManager::disableBlend();

    if (!depthTest)
    {
        GlStateManager::enableDepthTest();
    }
}

void LevelRenderer::renderDebugWireframeVolume(boost::shared_ptr<Entity> entity, const std::wstring& text,
                                               const AABB* box, int color, bool splitLargeFaces, bool depthTest,
                                               bool thickLines, bool raiseText)
{
    if (!depthTest)
    {
        GlStateManager::disableDepthTest();
    }

    GlStateManager::disableLighting();

    Vec3* centre = box->getCenter();

    if (raiseText)
    {
        centre->move(0.0, 1.0, 0.0);
    }

    renderStringInWorld(entity, text, centre);

    GlStateManager::disableTexture();

    if (thickLines)
    {
        GlStateManager::lineWidth(4.0f);
    }
    else
    {
        GlStateManager::lineWidth(2.0f);
    }

    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::enableBlend();
    GlStateManager::blendFuncSeparate(4, 5, 1, 0);
    GlStateManager::depthMask(false);

    renderColoredLineBox(box, color, splitLargeFaces);

    GlStateManager::depthMask(true);
    GlStateManager::enableTexture();
    GlStateManager::disableBlend();

    if (!depthTest)
    {
        GlStateManager::enableDepthTest();
    }
}

void LevelRenderer::setDirty(int minX, int minY, int minZ, int maxX, int maxY, int maxZ, Level* level)
{
    if (level == 0)
    {
        level = mLevels[mMinecraft->mPlayer->mUserIndex];
    }

    int chunkMinX = Mth::intFloorDiv(minX, 16);
    int chunkMinY = Mth::intFloorDiv(minY, 16);
    int chunkMinZ = Mth::intFloorDiv(minZ, 16);
    int chunkMaxX = Mth::intFloorDiv(maxX, 16);
    int chunkMaxY = Mth::intFloorDiv(maxY, 16);
    int chunkMaxZ = Mth::intFloorDiv(maxZ, 16);

    for (int chunkX = chunkMinX; chunkX <= chunkMaxX; ++chunkX)
    {
        for (int chunkY = chunkMinY; chunkY <= chunkMaxY; ++chunkY)
        {
            for (int chunkZ = chunkMinZ; chunkZ <= chunkMaxZ; ++chunkZ)
            {
                int index = getGlobalIndexForChunk(chunkX * 16, chunkY * 16, chunkZ * 16, level);

                if (index >= 0)
                {
                    mDirtyChunkStack.Push((int*)(index + 2));
                    PIXSetMarkerDeprecated(0.0f, "Setting chunk %d %d %d dirty", chunkX * 16, chunkY * 16, chunkZ * 16);
                }
            }
        }
    }
}

void LevelRenderer::blockChanged(Level* level, const BlockPos& pos, const BlockState* oldState,
                                 const BlockState* newState, int flags, bool immediate)
{
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;
    setDirty(x - 1, y - 1, z - 1, x + 1, y + 1, z + 1, 0);
}

void LevelRenderer::blockLightChanged(const BlockPos& pos)
{
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;
    setDirty(x - 1, y - 1, z - 1, x + 1, y + 1, z + 1, 0);
}

void LevelRenderer::setBlocksDirty(int minX, int minY, int minZ, int maxX, int maxY, int maxZ, Level* level)
{
    setDirty(minX - 1, minY - 1, minZ - 1, maxX + 1, maxY + 1, maxZ + 1, level);
}

void LevelRenderer::notifyNearbyEntities(Level* level, const BlockPos& pos, bool playing)
{
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(LivingEntity),
                                                                                  AABB::newTemp(pos)->grow(3.0));

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<LivingEntity> livingEntity = boost::static_pointer_cast<LivingEntity>(*it);

        livingEntity->setRecordPlayingNearby(pos, playing);
    }

    delete entities;
}

void LevelRenderer::cull(Culler* culler, float partialTick)
{
    int userIndex = mMinecraft->mPlayer->mUserIndex;

    FrustumCuller* frustumCuller = reinterpret_cast<FrustumCuller*>(culler);
    double xOffset = -frustumCuller->xOffset;
    double yOffset = -frustumCuller->yOffset;
    double zOffset = -frustumCuller->zOffset;
    float** frustum = frustumCuller->frustumData->frustum;

    float planes[24];

    for (int i = 0; i < 6; ++i)
    {
        float* plane = frustum[i];
        planes[i * 4 + 0] = plane[0];
        planes[i * 4 + 1] = plane[1];
        planes[i * 4 + 2] = plane[2];
        planes[i * 4 + 3] = (float)(plane[3] + plane[0] * xOffset + plane[1] * yOffset + plane[2] * zOffset);
    }

    ClipChunk* chunks = mClipChunks[userIndex].data;
    unsigned int count = mClipChunks[userIndex].length;

    for (unsigned int i = 0; i < count; ++i)
    {
        ClipChunk* chunk = &chunks[i];

        if (chunk->mGlobalIndex == -1)
        {
            chunk->mVisible = false;
            continue;
        }

        unsigned char flags = mGlobalChunkFlags[chunk->mGlobalIndex];

        if ((flags & 1) == 0 || (flags & 0x1c) == 0x1c)
        {
            chunk->mVisible = false;
            continue;
        }

        chunk->mVisible = clip(&chunk->mMinX, planes);
    }
}

void LevelRenderer::playSound(boost::shared_ptr<Player> player, const SoundEvent* sound,
                              SoundSource::ESoundSource source, double x, double y, double z, float volume, float pitch,
                              float range)
{
}

void LevelRenderer::addParticle(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, double x, double y, double z,
                                double xa, double ya, double za, arrayWithLength<int> params)
{
    addParticle(level, type, overrideLimiter, false, x, y, z, xa, ya, za, params);
}

int LevelRenderer::calculateParticleLevel(Level* level, bool overrideLimiter)
{
    int result = mMinecraft->mOptions->mParticles;

    if (overrideLimiter && result == 2)
    {
        if (level->mRandom->nextInt(10) == 0)
        {
            result = 1;

            if (level->mRandom->nextInt(3) == 0)
            {
                result = 2;
            }
        }
    }
    else if (result == 1)
    {
        if (level->mRandom->nextInt(3) == 0)
        {
            result = 2;
        }
    }

    return result;
}

Particle* LevelRenderer::addParticleInternal(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, double x,
                                             double y, double z, double xa, double ya, double za,
                                             arrayWithLength<int> params)
{
    return addParticleInternal(level, type, overrideLimiter, false, x, y, z, xa, ya, za, params);
}

void LevelRenderer::addParticle(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, bool force, double x, double y,
                                double z, double xa, double ya, double za, arrayWithLength<int> params)
{
    addParticleInternal(level, type, overrideLimiter, force, x, y, z, xa, ya, za, params);
}

void LevelRenderer::addParticle(Level* level, const ParticleType* particle, double x, double y, double z, double xa,
                                double ya, double za, arrayWithLength<int> params)
{
    addParticle(level, particle->getId(), particle->getOverrideLimiter(), x, y, z, xa, ya, za, params);
}

void LevelRenderer::destroyBlockProgress(int breakerId, const BlockPos& pos, int progress)
{
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;
    boost::unordered_map<int, BlockDestructionProgress*>::iterator entry = mDestroyingBlocks.find(breakerId);

    if ((unsigned int)progress >= 10)
    {
        if (entry != mDestroyingBlocks.end())
        {
            delete entry->second;
            mDestroyingBlocks.erase(entry);
        }

        return;
    }

    BlockDestructionProgress* destruction = 0;

    if (entry != mDestroyingBlocks.end())
    {
        destruction = entry->second;
    }

    if (destruction == 0 || destruction->getPos().x != x || destruction->getPos().y != y
        || destruction->getPos().z != z)
    {
        destruction = new BlockDestructionProgress(breakerId, pos);
        mDestroyingBlocks[breakerId] = destruction;
    }

    destruction->setProgress(progress);
    destruction->updateTick(mRenderTick);
}

void LevelRenderer::registerTextures(IconRegister* iconRegister)
{
    for (int stage = 0; stage < 10; ++stage)
    {
        mDestroyStages[stage] = iconRegister->registerIcon(L"destroy_" + _toString(stage));
    }
}

void LevelRenderer::fullyFlagRenderableBlockEntitiesToBeRemoved()
{
    EnterCriticalSection(&mBlockEntityMutex);

    for (ChunkBlockEntityMap::iterator entry = mBlockEntities.begin(); entry != mBlockEntities.end(); ++entry)
    {
        std::vector<boost::shared_ptr<BlockEntity> >& list = entry->second;

        std::vector<boost::shared_ptr<BlockEntity> >::iterator end = list.end();

        for (std::vector<boost::shared_ptr<BlockEntity> >::iterator blockEntity = list.begin(); blockEntity != end;
             ++blockEntity)
        {
            (*blockEntity)->upgradeRenderRemoveStage();
        }
    }

    LeaveCriticalSection(&mBlockEntityMutex);
}

int LevelRenderer::getGlobalIndexForChunk(int x, int y, int z, int dimension)
{
    int dimensionIndex = getDimensionIndexFromId(dimension);
    return getChunkIndex(dimensionIndex, x >> 4, y >> 4, z >> 4);
}

void LevelRenderer::setGlobalChunkFlag(int x, int y, int z, Level* level, unsigned char mask, unsigned char shift)
{
    unsigned char bits = (unsigned char)(mask << shift);
    int index = getGlobalIndexForChunk(x, y, z, level);

    if (index != -1)
    {
        mGlobalChunkFlags[index] |= bits;
    }
}

void LevelRenderer::clearGlobalChunkFlag(int x, int y, int z, Level* level, unsigned char mask, unsigned char shift)
{
    unsigned char bits = (unsigned char)(mask << shift);
    int index = getGlobalIndexForChunk(x, y, z, level);

    if (index != -1)
    {
        mGlobalChunkFlags[index] &= ~bits;
    }
}

unsigned char LevelRenderer::getGlobalChunkFlags(int x, int y, int z, Level* level)
{
    int index = getGlobalIndexForChunk(x, y, z, level);

    if (index == -1)
    {
        return 0;
    }

    return mGlobalChunkFlags[index];
}

void LevelRenderer::setGlobalChunkFlags(int x, int y, int z, Level* level, unsigned char flags)
{
    int index = getGlobalIndexForChunk(x, y, z, level);

    if (index != -1)
    {
        mGlobalChunkFlags[index] = flags;
    }
}

unsigned char LevelRenderer::incGlobalChunkRefCount(int x, int y, int z, Level* level)
{
    int index = getGlobalIndexForChunk(x, y, z, level);

    if (index != -1)
    {
        unsigned char flags = mGlobalChunkFlags[index];
        unsigned char count = (flags >> 5) & 7;
        count++;
        flags &= 0x1f;
        flags |= count << 5;
        mGlobalChunkFlags[index] = flags;

        return count;
    }

    return 0;
}

unsigned char LevelRenderer::decGlobalChunkRefCount(int x, int y, int z, Level* level)
{
    int index = getGlobalIndexForChunk(x, y, z, level);

    if (index != -1)
    {
        unsigned char flags = mGlobalChunkFlags[index];
        unsigned char count = (flags >> 5) & 7;
        count--;
        flags &= 0x1f;
        flags |= count << 5;
        mGlobalChunkFlags[index] = flags;

        return count;
    }

    return 0;
}

void LevelRenderer::playerRemoved(boost::shared_ptr<Entity> player)
{
}

int LevelRenderer::checkAllPresentChunks(bool* present)
{
    const int userIndex = mMinecraft->mPlayer->mUserIndex;
    ClipChunk* clips = mClipChunks[userIndex].data;
    int count = 0;

    for (unsigned int i = 0; i < mClipChunks[userIndex].length; ++i)
    {
        Chunk* chunk = clips[i].mChunk;
        if (chunk->mZ == 0 && mLevels[0]->reallyHasChunk(chunk->mX >> 4, chunk->mDimension >> 4))
        {
            ++count;
            for (int x = 4; x < 13; ++x)
            {
                for (int zBase = 4; zBase < 12; zBase += 4)
                {
                    for (int zOffset = 0; zOffset < 4; ++zOffset)
                    {
                        const BlockPos pos(x, 0, zBase + zOffset);
                        Block* block = mMinecraft->mLevel->getBlockState(pos)->getBlock();
                        if (block != Blocks::bedrock && block != Blocks::dirt)
                            *present = true;
                    }
                }

                const BlockPos pos(x, 0, 12);
                Block* block = mMinecraft->mLevel->getBlockState(pos)->getBlock();
                if (block != Blocks::bedrock && block != Blocks::dirt)
                    *present = true;
            }
        }
    }
    return count;
}

void LevelRenderer::playStreamingMusic(const SoundEvent* sound, const BlockPos& pos, bool playing)
{
    mMinecraft->mSoundEngine->playStreaming(sound, (float)pos.x, (float)pos.y, (float)pos.z, 1.0f, 1.0f, playing, false,
                                            ConsoleSoundEngine::eAudioDelayType_Random);
    if (sound != NULL)
    {
        RecordItem* record = RecordItem::getBySound(sound);
        if (record != NULL)
            mMinecraft->mGui->setNowPlaying(record->getDisplayName());
    }
    notifyNearbyEntities(mLevels[mMinecraft->mPlayer->mUserIndex], pos, sound != NULL);
}

Particle* LevelRenderer::addParticleInternal(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, bool force,
                                             double x, double y, double z, double xa, double ya, double za,
                                             arrayWithLength<int> params)
{
    if (mMinecraft == NULL)
        return NULL;

    boost::shared_ptr<Entity> camera = mMinecraft->getCameraEntity();
    if (mMinecraft == NULL || camera == NULL || mMinecraft->mParticleEngine == NULL)
        return NULL;

    if (Mth::isNaN(x) || Mth::isNaN(y) || Mth::isNaN(z))
        return NULL;

    if (level == NULL)
        level = mLevels[mMinecraft->mPlayer->mUserIndex];

    int particleSetting = calculateParticleLevel(level, force);
    bool exemptFromPlayerRange = type == ePARTICLE_HUGE_EXPLOSION || type == ePARTICLE_LARGE_EXPLODE
                                 || type == ePARTICLE_DRAGON_BREATH || type == ePARTICLE_CHEST_REFILL;

    if (mMinecraft->mAnimateTickLevel == NULL && !exemptFromPlayerRange)
    {
        bool nearLocalPlayer = false;
        for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> player = mMinecraft->mLocalPlayers[playerIndex];
            if (!player)
                continue;
            if (mLevels[playerIndex] != level)
                continue;

            double dy = player->mY - y;
            double dx = player->mX - x;
            double dz = player->mZ - z;
            if (dx * dx + dy * dy + dz * dz <= 256.0)
            {
                nearLocalPlayer = true;
                break;
            }
        }

        if (!nearLocalPlayer)
            return NULL;
    }

    if (particleSetting < 2)
        return mMinecraft->mParticleEngine->createParticle(type, x, y, z, xa, ya, za, params, level);

    return NULL;
}

void LevelRenderer::entityAdded(boost::shared_ptr<Entity> entity)
{
    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);

        player->prepareCustomTextures();
        if (player->mSkinLocation != L"")
            mTextures->addMemTexture(player->mSkinLocation, new MobSkinMemTextureProcessor());
        if (player->mCapeLocation != L"")
            mTextures->addMemTexture(player->mCapeLocation, new MobSkinMemTextureProcessor());
    }
}

void LevelRenderer::entityRemoved(boost::shared_ptr<Entity> entity)
{
    bool isPlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType());

    if (isPlayer)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);

        if (player->mSkinLocation != L"")
            mTextures->removeMemTexture(player->mSkinLocation);
        if (player->mCapeLocation != L"")
            mTextures->removeMemTexture(player->mCapeLocation);
    }
}

void LevelRenderer::globalLevelEvent(int type, const BlockPos& pos, int data)
{
    Level* level = mLevels[mMinecraft->mPlayer->mUserIndex];
    if (type != 1023 && type != 1028 && type != 1038)
        return;

    if (mMinecraft->getCameraEntity())
    {
        double dx = pos.x - mMinecraft->getCameraEntity()->mX;
        double dy = pos.y - mMinecraft->getCameraEntity()->mY;
        double dz = pos.z - mMinecraft->getCameraEntity()->mZ;
        double distance = std::sqrt(dx * dx + dy * dy + dz * dz);
        double x = mMinecraft->getCameraEntity()->mX;
        double y = mMinecraft->getCameraEntity()->mY;
        double z = mMinecraft->getCameraEntity()->mZ;
        if (distance > 0.0)
        {
            x += dx / distance * 2.0;
            y += dy / distance * 2.0;
            z += dz / distance * 2.0;
        }

        if (type == 1023)
            level->playLocalSound(x, y, z, SoundEvents::WITHER_SPAWN, SoundSource::eSoundSource_Hostile, 1.0f, 1.0f,
                                  false, 16.0f);
        else if (type == 1038)
            level->playLocalSound(x, y, z, SoundEvents::END_PORTAL_SPAWN, SoundSource::eSoundSource_Hostile, 1.0f, 1.0f,
                                  false, 16.0f);
        else if (type == 1028)
            level->playLocalSound(x, y, z, SoundEvents::ENDERDRAGON_DEATH, SoundSource::eSoundSource_Hostile, 5.0f,
                                  1.0f, false, 16.0f);
    }
}

void LevelRenderer::levelEvent(boost::shared_ptr<Player> player, int type, const BlockPos& pos, int data)
{
    const int x = pos.x;
    const int y = pos.y;
    const int z = pos.z;
    int playerIndex = mMinecraft->mPlayer->mUserIndex;
    if (player != NULL && player->isLocalPlayer())
        playerIndex = boost::dynamic_pointer_cast<MultiplayerLocalPlayer>(player)->mUserIndex;
    Level* level = mLevels[playerIndex];
    Random* random = level->mRandom;

    switch (type)
    {
    case LevelEvent::SOUND_DRAGON_DEATH:
        if (mMinecraft->getCameraEntity() != NULL)
        {
            const double dx = x - mMinecraft->getCameraEntity()->mX;
            const double dy = y - mMinecraft->getCameraEntity()->mY;
            const double dz = z - mMinecraft->getCameraEntity()->mZ;
            const double distance = std::sqrt(dx * dx + dy * dy + dz * dz);
            double soundX = mMinecraft->getCameraEntity()->mX;
            double soundY = mMinecraft->getCameraEntity()->mY;
            double soundZ = mMinecraft->getCameraEntity()->mZ;
            if (distance > 0.0)
            {
                soundX += dx / distance * 2.0;
                soundZ += dz / distance * 2.0;
                soundY += dy / distance * 2.0;
            }
            mLevels[playerIndex]->playLocalSound(soundX, soundY, soundZ, SoundEvents::ENDERDRAGON_DEATH,
                                                 SoundSource::eSoundSource_Hostile, 5.0f, 1.0f, false, 16.0f);
        }
        break;
    case LevelEvent::SOUND_BREWING_STAND_BREW:
        level->playLocalSound(x, y, z, SoundEvents::BREWING_STAND_BREW, SoundSource::eSoundSource_Blocks, 1.0f, 1.0f,
                              false, 16.0f);
        break;
    case LevelEvent::SOUND_CHORUS_GROW:
        level->playLocalSound(x, y, z, SoundEvents::CHORUS_FLOWER_GROW, SoundSource::eSoundSource_Blocks, 1.0f, 1.0f,
                              false, 16.0f);
        break;
    case LevelEvent::SOUND_CHORUS_DEATH:
        level->playLocalSound(x, y, z, SoundEvents::CHORUS_FLOWER_DEATH, SoundSource::eSoundSource_Blocks, 1.0f, 1.0f,
                              false, 16.0f);
        break;
    case LevelEvent::SOUND_PORTAL_TRAVEL:
        mMinecraft->mSoundEngine->playUI(SoundEvents::PORTAL_TRAVEL, 1.0f, random->nextFloat() * 0.4f + 0.8f);
        break;
    case LevelEvent::SOUND_DISPENSER_FAIL:
        level->playLocalSound(x, y, z, SoundEvents::DISPENSER_FAIL, SoundSource::eSoundSource_Blocks, 1.0f, 1.2f, false,
                              16.0f);
        break;
    case LevelEvent::SOUND_DISPENSER_DISPENSE:
        level->playLocalSound(x, y, z, SoundEvents::DISPENSER_DISPENSE, SoundSource::eSoundSource_Blocks, 1.0f, 1.0f,
                              false, 16.0f);
        break;
    case LevelEvent::SOUND_ENDER_EYE_LAUNCH:
        level->playLocalSound(x, y, z, SoundEvents::ENDEREYE_LAUNCH, SoundSource::eSoundSource_Neutral, 1.0f, 1.2f,
                              false, 16.0f);
        break;
    case LevelEvent::SOUND_FIREWORK_SHOOT:
        level->playLocalSound(x, y, z, SoundEvents::FIREWORK_SHOOT, SoundSource::eSoundSource_Neutral, 1.0f, 1.2f,
                              false, 16.0f);
        break;
    case LevelEvent::SOUND_DISPENSER_PROJECTILE_LAUNCH:
        level->playLocalSound(x, y, z, SoundEvents::DISPENSER_LAUNCH, SoundSource::eSoundSource_Blocks, 1.0f, 1.2f,
                              false, 16.0f);
        break;
    case LevelEvent::PARTICLES_SHOOT:
    {
        const int xStep = data % 3 - 1;
        const int zStep = data / 3 % 3 - 1;
        const double originX = x + xStep * 0.6 + 0.5 + xStep * 0.01;
        const double originY = y + 0.5;
        const double originZ = z + zStep * 0.6 + 0.5 + zStep * 0.01;
        for (int i = 0; i < 10; ++i)
        {
            const double speed = random->nextDouble() * 0.2 + 0.01;
            const double particleX = originX + (random->nextDouble() - 0.5) * zStep * 0.5;
            const double particleY = originY + (random->nextDouble() - 0.5) * 0.5;
            const double particleZ = originZ + (random->nextDouble() - 0.5) * xStep * 0.5;
            const double xa = xStep * speed + random->nextGaussian() * 0.01;
            const double ya = -0.03 + random->nextGaussian() * 0.01;
            const double za = zStep * speed + random->nextGaussian() * 0.01;
            addParticle(mLevels[playerIndex], PARTICLE_SMOKE, particleX, particleY, particleZ, xa, ya, za,
                        arrayWithLength<int>());
        }
        break;
    }
    case LevelEvent::PARTICLES_EYE_OF_ENDER_DEATH:
    {
        const double originX = x + 0.5;
        const double originY = y;
        const double originZ = z + 0.5;
        for (int i = 0; i < 8; ++i)
        {
            int itemId[1] = {Item::getId(Items::enderEye)};
            const double xa = random->nextGaussian() * 0.15;
            const double ya = random->nextDouble() * 0.2;
            const double za = random->nextGaussian() * 0.15;
            addParticle(mLevels[playerIndex], PARTICLE_ICON_CRACK, originX, originY, originZ, xa, ya, za,
                        arrayWithLength<int>(itemId, 1));
        }
        for (double angle = 0.0; angle < 6.2831854820251465; angle += 0.15707963705062866)
        {
            addParticle(mLevels[playerIndex], PARTICLE_ENDER, originX + std::cos(angle) * 5.0, originY - 0.4,
                        originZ + std::sin(angle) * 5.0, std::cos(angle) * -5.0, 0.0, std::sin(angle) * -5.0,
                        arrayWithLength<int>());
            addParticle(mLevels[playerIndex], PARTICLE_ENDER, originX + std::cos(angle) * 5.0, originY - 0.4,
                        originZ + std::sin(angle) * 5.0, std::cos(angle) * -7.0, 0.0, std::sin(angle) * -7.0,
                        arrayWithLength<int>());
        }
        break;
    }
    case LevelEvent::PARTICLES_SPELL_POTION_SPLASH:
    case LevelEvent::PARTICLES_INSTANT_POTION_SPLASH:
    {
        for (int i = 0; i < 8; ++i)
        {
            int itemId[1] = {Item::getId(Items::potion)};
            const double xa = random->nextGaussian() * 0.15;
            const double ya = random->nextDouble() * 0.2;
            const double za = random->nextGaussian() * 0.15;
            addParticle(mLevels[playerIndex], PARTICLE_ICON_CRACK, x, y, z, xa, ya, za,
                        arrayWithLength<int>(itemId, 1));
        }
        int particleCount = 100;
        if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureReducedParticles())
            particleCount = 20;
        if (!Minecraft::GetInstance()->GetMiniGame()->HasFeatureInvertedExplodingGrenades())
        {
            const float red = static_cast<float>(data >> 16 & 0xFF) / 255.0f;
            const float green = static_cast<float>(data >> 8 & 0xFF) / 255.0f;
            const float blue = static_cast<float>(data & 0xFF) / 255.0f;
            const ePARTICLE_TYPE particleType = type == LevelEvent::PARTICLES_INSTANT_POTION_SPLASH
                                                    ? ePARTICLE_INSTANT_SPELL
                                                    : ePARTICLE_SPELL;
            for (int i = 0; i < particleCount; ++i)
            {
                const double power = random->nextDouble() * ThrownPotion::SPLASH_RANGE;
                const double angle = random->nextDouble() * 3.1415927f * 2.0;
                const double xa = std::cos(angle) * power;
                const double ya = random->nextDouble() * 0.5 + 0.01;
                const double za = std::sin(angle) * power;
                Particle* particle = addParticleInternal(mLevels[playerIndex], particleType, false, x + xa * 0.1,
                                                         y + 0.3, z + za * 0.1, xa, ya, za, arrayWithLength<int>());
                if (particle != NULL)
                {
                    const float brightness = random->nextFloat() * 0.25f + 0.75f;
                    particle->setColor(red * brightness, green * brightness, blue * brightness);
                    particle->setPower(static_cast<float>(power));
                }
            }
        }
        mLevels[playerIndex]->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::GLASS_BREAK,
                                             SoundSource::eSoundSource_Blocks, 1.0f,
                                             mLevels[playerIndex]->mRandom->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        break;
    }
    case LevelEvent::PARTICLES_DESTROY_BLOCK:
    {
        Block* block = Block::byId(data & 0xFFF);
        if (block->mMaterial != Material::AIR)
        {
            const SoundType* soundType = block->getSoundType();
            static_cast<MultiPlayerLevel*>(mLevels[playerIndex])
                ->playLocalSound(BlockPos(x, y, z), soundType->getBreakSound(), SoundSource::eSoundSource_Blocks,
                                 (soundType->getVolume() + 1.0f) * 0.5f, soundType->getPitch() * 0.8f, false, 16.0f);
        }
        mMinecraft->mParticleEngine->destroy(BlockPos(x, y, z), block->getBlockState(data >> 12 & 0xFF));
        break;
    }
    case LevelEvent::PARTICLES_MOBBLOCK_SPAWN:
        for (int i = 0; i < 20; ++i)
        {
            const double particleX = x + 0.5 + (mLevels[playerIndex]->mRandom->nextFloat() - 0.5) * 2.0;
            const double particleY = y + 0.5 + (mLevels[playerIndex]->mRandom->nextFloat() - 0.5) * 2.0;
            const double particleZ = z + 0.5 + (mLevels[playerIndex]->mRandom->nextFloat() - 0.5) * 2.0;
            mLevels[playerIndex]->addParticle(PARTICLE_SMOKE, particleX, particleY, particleZ, 0.0, 0.0, 0.0,
                                              arrayWithLength<int>());
            mLevels[playerIndex]->addParticle(PARTICLE_FLAME, particleX, particleY, particleZ, 0.0, 0.0, 0.0,
                                              arrayWithLength<int>());
        }
        break;
    case LevelEvent::PARTICLES_PLANT_GROWTH:
        DyeItem::addGrowthParticles(level, BlockPos(x, y, z), data);
        break;
    case LevelEvent::PARTICLES_DRAGON_FIREBALL_SPLASH:
    {
        for (int i = 0; i < 100; ++i)
        {
            const double power = random->nextDouble() * DragonFireball::SPLASH_RANGE;
            const double angle = random->nextDouble() * 3.141592653589793 * 2.0;
            const double xa = std::cos(angle) * power;
            const double ya = random->nextDouble() * 0.5 + 0.01;
            const double za = std::sin(angle) * power;
            Particle* particle = addParticleInternal(mLevels[playerIndex], ePARTICLE_DRAGON_BREATH, false, x + xa * 0.1,
                                                     y + 0.3, z + za * 0.1, xa, ya, za, arrayWithLength<int>());
            if (particle != NULL)
            {
                random->nextFloat();
                particle->setPower(static_cast<float>(power));
            }
        }
        mLevels[playerIndex]->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::ENDERDRAGON_FIREBALL_EXPLODE,
                                             SoundSource::eSoundSource_Hostile, 1.0f,
                                             mLevels[playerIndex]->mRandom->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        break;
    }
    case LevelEvent::SOUND_CLOSE_WOODEN_DOOR:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::WOODEN_DOOR_CLOSE,
                              SoundSource::eSoundSource_Blocks, 1.0f, random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        break;
    case LevelEvent::SOUND_CLOSE_IRON_TRAP_DOOR:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::IRON_TRAPDOOR_CLOSE,
                              SoundSource::eSoundSource_Blocks, 1.0f, random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        break;
    case LevelEvent::SOUND_CLOSE_WOODEN_TRAP_DOOR:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::WOODEN_TRAPDOOR_CLOSE,
                              SoundSource::eSoundSource_Blocks, 1.0f, random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        break;
    case LevelEvent::SOUND_CLOSE_FENCE_GATE:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::FENCE_GATE_CLOSE,
                              SoundSource::eSoundSource_Blocks, 1.0f, random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        break;
    case LevelEvent::SOUND_CLOSE_IRON_DOOR:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::IRON_DOOR_CLOSE, SoundSource::eSoundSource_Blocks,
                              1.0f, random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        break;
    case LevelEvent::SOUND_OPEN_WOODEN_DOOR:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::WOODEN_DOOR_OPEN,
                              SoundSource::eSoundSource_Blocks, 1.0f, random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        break;
    case LevelEvent::SOUND_OPEN_WOODEN_TRAP_DOOR:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::WOODEN_TRAPDOOR_OPEN,
                              SoundSource::eSoundSource_Blocks, 1.0f, random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        break;
    case LevelEvent::SOUND_OPEN_IRON_TRAP_DOOR:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::IRON_TRAPDOOR_OPEN,
                              SoundSource::eSoundSource_Blocks, 1.0f, random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        break;
    case LevelEvent::SOUND_OPEN_FENCE_GATE:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::FENCE_GATE_OPEN, SoundSource::eSoundSource_Blocks,
                              1.0f, random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        break;
    case LevelEvent::SOUND_OPEN_IRON_DOOR:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::IRON_DOOR_OPEN, SoundSource::eSoundSource_Blocks,
                              1.0f, random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        break;
    case LevelEvent::SOUND_EXTINGUISH_FIRE:
    {
        const float first = random->nextFloat();
        const float second = random->nextFloat();
        level->playLocalSound(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f, static_cast<float>(z) + 0.5f,
                              SoundEvents::FIRE_EXTINGUISH, SoundSource::eSoundSource_Blocks, 0.5f,
                              (first - second) * 0.8f + 2.6f, false, 16.0f);
        break;
    }
    case LevelEvent::SOUND_ANVIL_BROKEN:
        level->playLocalSound(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f, static_cast<float>(z) + 0.5f,
                              SoundEvents::ANVIL_BREAK, SoundSource::eSoundSource_Blocks, 1.0f,
                              random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        break;
    case LevelEvent::SOUND_ANVIL_USED:
        level->playLocalSound(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f, static_cast<float>(z) + 0.5f,
                              SoundEvents::ANVIL_USE, SoundSource::eSoundSource_Blocks, 1.0f,
                              random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        break;
    case LevelEvent::SOUND_ANVIL_LAND:
        level->playLocalSound(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f, static_cast<float>(z) + 0.5f,
                              SoundEvents::ANVIL_LAND, SoundSource::eSoundSource_Blocks, 0.3f,
                              random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        break;
    case LevelEvent::SOUND_PLAY_RECORDING:
    {
        Item* item = Item::byId(data);
        RecordItem* record = item != NULL ? dynamic_cast<RecordItem*>(item) : NULL;
        if (record != NULL)
            mLevels[playerIndex]->playStreamingMusic(BlockPos(x, y, z), record->getSound(), true);
        else if (!mMinecraft->mSoundEngine->GetIsPlayingStreamingGameMusic())
            mLevels[playerIndex]->playStreamingMusic(BlockPos(x, y, z), NULL, true);
        mMinecraft->mLocalPlayers[playerIndex]->updateRichPresence();
        break;
    }
    case LevelEvent::SOUND_GHAST_WARNING:
    {
        const float first = random->nextFloat();
        const float second = random->nextFloat();
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::GHAST_WARN, SoundSource::eSoundSource_Hostile,
                              2.0f, (first - second) * 0.2f + 1.0f, false, 80.0f);
        break;
    }
    case LevelEvent::SOUND_GHAST_FIREBALL:
    {
        const float first = random->nextFloat();
        const float second = random->nextFloat();
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::GHAST_SHOOT, SoundSource::eSoundSource_Hostile,
                              2.0f, (first - second) * 0.2f + 1.0f, false, 80.0f);
        break;
    }
    case LevelEvent::SOUND_ZOMBIE_WOODEN_DOOR:
    {
        const float first = random->nextFloat();
        const float second = random->nextFloat();
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::ZOMBIE_ATTACK_DOOR_WOOD,
                              SoundSource::eSoundSource_Hostile, 2.0f, (first - second) * 0.2f + 1.0f, false, 16.0f);
        break;
    }
    case LevelEvent::SOUND_ZOMBIE_DOOR_CRASH:
    {
        const float first = random->nextFloat();
        const float second = random->nextFloat();
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::ZOMBIE_BREAK_DOOR_WOOD,
                              SoundSource::eSoundSource_Hostile, 2.0f, (first - second) * 0.2f + 1.0f, false, 16.0f);
        break;
    }
    case LevelEvent::SOUND_ZOMBIE_IRON_DOOR:
    {
        const float first = random->nextFloat();
        const float second = random->nextFloat();
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::ZOMBIE_ATTACK_IRON_DOOR,
                              SoundSource::eSoundSource_Hostile, 2.0f, (first - second) * 0.2f + 1.0f, false, 16.0f);
        break;
    }
    case LevelEvent::SOUND_BLAZE_FIREBALL:
    {
        const float first = random->nextFloat();
        const float second = random->nextFloat();
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::BLAZE_SHOOT, SoundSource::eSoundSource_Hostile,
                              2.0f, (first - second) * 0.2f + 1.0f, false, 16.0f);
        break;
    }
    case LevelEvent::SOUND_WITHER_BOSS_SHOOT:
    {
        const float first = random->nextFloat();
        const float second = random->nextFloat();
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::WITHER_SHOOT, SoundSource::eSoundSource_Hostile,
                              2.0f, (first - second) * 0.2f + 1.0f, false, 16.0f);
        break;
    }
    case LevelEvent::SOUND_ZOMBIE_INFECTED:
    {
        const float first = random->nextFloat();
        const float second = random->nextFloat();
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::ZOMBIE_INFECT, SoundSource::eSoundSource_Hostile,
                              2.0f, (first - second) * 0.2f + 1.0f, false, 16.0f);
        break;
    }
    case LevelEvent::SOUND_ZOMBIE_CONVERTED:
    {
        const float first = random->nextFloat();
        const float second = random->nextFloat();
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::ZOMBIE_VILLAGER_CURE,
                              SoundSource::eSoundSource_Hostile, 2.0f, (first - second) * 0.2f + 1.0f, false, 16.0f);
        break;
    }
    case LevelEvent::DRAGON_EGG_TELEPORT:
        DragonEggBlock::generateTeleportParticles(level, x, y, z, data);
        break;
    case LevelEvent::SOUND_BAT_LIFTOFF:
    {
        const float first = random->nextFloat();
        const float second = random->nextFloat();
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::BAT_TAKEOFF, SoundSource::eSoundSource_Neutral,
                              0.05f, (first - second) * 0.2f + 1.0f, false, 16.0f);
        break;
    }
    case LevelEvent::ANIMATION_END_GATEWAY_SPAWN:
    {
        level->addParticle(PARTICLE_HUGE_EXPLOSION, true, pos.x + 0.5, pos.y + 0.5, pos.z + 0.5, 0.0, 0.0, 0.0,
                           arrayWithLength<int>());
        mLevels[playerIndex]->playLocalSound(
            x + 0.5, y + 0.5, z + 0.5, SoundEvents::END_GATEWAY_SPAWN, SoundSource::eSoundSource_Blocks, 10.0f,
            ((mLevels[playerIndex]->mRandom->nextFloat() - mLevels[playerIndex]->mRandom->nextFloat()) * 0.2f + 1.0f)
                * 0.7f,
            false, 16.0f);
        break;
    }
    case LevelEvent::ANIMATION_DRAGON_SUMMON_ROAR:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::ENDERDRAGON_GROWL,
                              SoundSource::eSoundSource_Hostile, 64.0f, random->nextFloat() * 0.3f + 0.8f, false,
                              16.0f);
        break;
    case LevelEvent::CAULDRON_EXPLODE:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::CAULDRON_EXPLODE,
                              SoundSource::eSoundSource_Blocks, 0.5f,
                              static_cast<float>((random->nextGaussian() * 0.2f + 1.0) * 0.7f), false, 16.0f);
        CauldronBlock::spawnExplosionParticles(mLevels[playerIndex], x, y, z, mLevels[playerIndex]->mRandom, data, 10);
        break;
    case LevelEvent::CAULDRON_DYE_ARMOR:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::CAULDRON_DYE_ARMOR,
                              SoundSource::eSoundSource_Blocks, 0.5f, random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        CauldronBlock::spawnSplashParticles(mLevels[playerIndex], x, y, z, mLevels[playerIndex]->mRandom, data, 2);
        break;
    case LevelEvent::CAULDRON_CLEAN_ARMOR:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::CAULDRON_CLEAN_ARMOR,
                              SoundSource::eSoundSource_Blocks, 0.5f, random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        CauldronBlock::spawnBubbleParticles(mLevels[playerIndex], x, y, z, mLevels[playerIndex]->mRandom, data, 2);
        break;
    case LevelEvent::CAULDRON_FILL_POTION:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::CAULDRON_FILL_POTION,
                              SoundSource::eSoundSource_Blocks, 0.5f, random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        CauldronBlock::spawnPotionParticles(mLevels[playerIndex], x, y, z, mLevels[playerIndex]->mRandom, data, 2);
        break;
    case LevelEvent::CAULDRON_TAKE_POTION:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::CAULDRON_TAKE_POTION,
                              SoundSource::eSoundSource_Blocks, 0.5f, random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        CauldronBlock::spawnPotionParticles(mLevels[playerIndex], x, y, z, mLevels[playerIndex]->mRandom, data, 2);
        break;
    case LevelEvent::CAULDRON_FILL_WATER:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::CAULDRON_FILL_WATER,
                              SoundSource::eSoundSource_Blocks, 0.5f, random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        CauldronBlock::spawnBubbleParticles(mLevels[playerIndex], x, y, z, mLevels[playerIndex]->mRandom, data, 2);
        break;
    case LevelEvent::CAULDRON_TAKE_WATER:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::CAULDRON_TAKE_WATER,
                              SoundSource::eSoundSource_Blocks, 0.5f, random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        CauldronBlock::spawnSplashParticles(mLevels[playerIndex], x, y, z, mLevels[playerIndex]->mRandom, data, 2);
        break;
    case LevelEvent::CAULDRON_ADD_DYE:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::CAULDRON_ADD_DYE,
                              SoundSource::eSoundSource_Blocks, 0.5f, random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        CauldronBlock::spawnBubbleParticles(mLevels[playerIndex], x, y, z, mLevels[playerIndex]->mRandom, data, 2);
        break;
    case LevelEvent::CAULDRON_CLEAN_BANNER:
        level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::CAULDRON_CLEAN_BANNER,
                              SoundSource::eSoundSource_Blocks, 0.5f, random->nextFloat() * 0.1f + 0.9f, false, 16.0f);
        CauldronBlock::spawnBubbleParticles(mLevels[playerIndex], x, y, z, mLevels[playerIndex]->mRandom, data, 2);
        break;
    default:
        break;
    }
}

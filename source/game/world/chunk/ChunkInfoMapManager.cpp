#include "game/world/chunk/ChunkInfoMapManager.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/LevelRenderer.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Minecraft.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerList.hpp"

static Level* sLevel;
static ChunkSource* sServerChunkSource;
static ChunkSource* sClientChunkSource;
static LevelRenderer* sLevelRenderer;

static const int CHUNK_INFO_MAP_SPLIT_SCREEN_SIZE = 64;
static const int CHUNK_INFO_MAP_SECTION_COUNT = 16;

static const unsigned char CHUNK_FLAG_BUILT = 1;

static const unsigned int CHUNK_STATUS_COLOUR_EMPTY = 0xFF404040;
static const unsigned int CHUNK_STATUS_COLOUR_MISSING = 0xFFFF0000;
static const unsigned int CHUNK_STATUS_COLOUR_NOT_BUILT = 0xFF808000;
static const unsigned int CHUNK_STATUS_COLOUR_BUILT = 0xFF00FF00;

void ChunkInfoMapManager::Render(float width, float height)
{
}

void ChunkInfoMapManager::renderStatusMap(BufferBuilder* bufferBuilder, float width, float height)
{
    int xzSize = sLevel->mDimension->getXZSize();
    int halfXZSize = xzSize / 2;

    PlayerList* players = gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers;
    int size;
    int centreChunkX;
    int centreChunkZ;
    int playerCount = players->getPlayerCount();
    if (playerCount > 1 || xzSize < CHUNK_INFO_MAP_SPLIT_SCREEN_SIZE)
    {
        centreChunkX = 0;
        centreChunkZ = 0;
        size = xzSize;
    }
    else
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mPlayer;
        int playerChunkX = player->mChunkX;
        int playerChunkZ = player->mChunkZ;
        size = CHUNK_INFO_MAP_SPLIT_SCREEN_SIZE;
        int minCentre = size / 2 - halfXZSize;
        int maxCentre = halfXZSize - size / 2;
        centreChunkX = Mth::clamp(playerChunkX, minCentre, maxCentre);
        centreChunkZ = Mth::clamp(playerChunkZ, minCentre, maxCentre);
    }

    int originChunkX = size / 2 - centreChunkX;
    int originChunkZ = size / 2 - centreChunkZ;

    for (int column = 0; column < size; column++)
    {
        bufferBuilder->begin(3);

        for (int row = 0; row < size; row++)
        {
            int chunkX = column - originChunkX;
            int chunkZ = row - originChunkZ;

            LevelChunk* chunk = sClientChunkSource->getChunk(chunkX, chunkZ);
            if ((!chunk->isReady() || chunk->isEmpty()) && !sServerChunkSource->reallyHasChunk(chunkX, chunkZ))
            {
                chunk->isEmpty();
            }

            int blockX = chunkX * 16;
            int blockZ = chunkZ * 16;

            for (int section = 0; section < CHUNK_INFO_MAP_SECTION_COUNT; section++)
            {
                if ((sLevelRenderer->getGlobalChunkFlags(blockX, section * 16, blockZ, sLevel) & CHUNK_FLAG_BUILT) != 0)
                {
                    break;
                }
            }
        }

        bufferBuilder->end();
    }
}

void ChunkInfoMapManager::renderHeatMap(BufferBuilder* bufferBuilder, float width, float height)
{
    int xzSize = sLevel->mDimension->getXZSize();
    int halfXZSize = xzSize / 2;

    for (int column = 0; column < xzSize; column++)
    {
        int chunkX = column - halfXZSize;

        for (int row = 0; row < xzSize; row++)
        {
            sClientChunkSource->getChunk(chunkX, row - halfXZSize)->getSparseBlockAndDataMemoryAllocated();
        }
    }

    for (int column = 0; column < xzSize; column++)
    {
        bufferBuilder->begin(3);

        int chunkX = column - halfXZSize;

        for (int row = 0; row < xzSize; row++)
        {
            LevelChunk* chunk = sClientChunkSource->getChunk(chunkX, row - halfXZSize);
            chunk->getSparseBlockAndDataMemoryAllocated();
        }

        bufferBuilder->end();
    }
}

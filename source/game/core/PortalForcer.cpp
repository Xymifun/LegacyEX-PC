#include "game/core/PortalForcer.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/Mth.hpp"
#include "game/core/Direction.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPattern.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/PortalBlock.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/server/ServerLevel.hpp"

PortalForcer::PortalPosition::PortalPosition(const BlockPos& pos, long long updateTime)
    : BlockPos(pos.x, pos.y, pos.z),
      lastUpdate(updateTime)
{
}

PortalForcer::PortalForcer(ServerLevel* newLevel)
{
    level = newLevel;
    random = new Random(newLevel->getSeed());
}

PortalForcer::~PortalForcer()
{
    delete random;
    boost::unordered_map<ChunkPos, PortalPosition*, ChunkPosKeyHash, ChunkPosKeyEq>::iterator it;
    for (it = portalCache.begin(); it != portalCache.end(); ++it)
    {
        delete it->second;
    }
}

bool PortalForcer::HasCachedPortal(const ChunkPos& chunkPos, BlockPos& portalPos)
{
    boost::unordered_map<ChunkPos, PortalPosition*, ChunkPosKeyHash, ChunkPosKeyEq>::iterator it = portalCache.find(
        chunkPos);
    if (it != portalCache.end())
    {
        PortalPosition* cached = it->second;
        LevelChunk* chunk = level->getChunkLoadedOrUnloadedAt(cached->x, cached->z);
        if (chunk != NULL && !chunk->isEmpty()
            && chunk->getBlockId(cached->x & 15, cached->y, cached->z & 15) != BlockId::PORTAL)
        {
            std::vector<ChunkPos>::iterator position = std::find(portalPositions.begin(), portalPositions.end(),
                                                                 chunkPos);
            portalPositions.erase(position);
            delete portalCache[chunkPos];
            portalCache.erase(chunkPos);
        }
        else
        {
            portalPos = *cached;
            cached->lastUpdate = level->getGameTime();
            return true;
        }
    }
    return false;
}

void PortalForcer::uncacheLeastUsedPortal()
{
    if (portalPositions.empty())
    {
        return;
    }

    std::vector<ChunkPos>::iterator least = portalPositions.begin();
    long long leastTime = 0;
    for (std::vector<ChunkPos>::iterator pos = portalPositions.begin(); pos != portalPositions.end(); ++pos)
    {
        ChunkPos key = *pos;
        PortalPosition* position = portalCache[key];
        if (position->lastUpdate < leastTime || pos == portalPositions.begin())
        {
            least = pos;
            leastTime = position->lastUpdate;
        }
    }

    ChunkPos chunkPos = *least;
    delete portalCache[chunkPos];
    portalPositions.erase(least);
    portalCache.erase(chunkPos);
}

bool PortalForcer::findPortal(boost::shared_ptr<Entity> entity, float rotation, int chunkX, int chunkZ)
{
    int radius = 128;
    if (level->mDimension->getType() == DimensionType::NETHER)
    {
        radius = 48;
    }
    BlockPos portalPos = BlockPos::ZERO;
    ChunkPos chunkPos(chunkX, chunkZ);
    double bestDistance = -1.0;
    bool addToCache = true;
    int bestX = 0;
    int bestY = 0;
    int bestZ = 0;
    if (HasCachedPortal(chunkPos, portalPos))
    {
        bestDistance = 0.0;
        addToCache = false;
        bestX = portalPos.x;
        bestY = portalPos.y;
        bestZ = portalPos.z;
    }
    else
    {
        bestX = portalPos.x;
        bestY = portalPos.y;
        bestZ = portalPos.z;
        BlockPos entityPos(entity);
        int originX = Mth::floor(entity->mX);
        int originY = Mth::floor(entity->mY);
        int originZ = Mth::floor(entity->mZ);

        for (int x = originX - radius; x <= originX + radius; ++x)
        {
            double xDistance = (double)x + 0.5 - entity->mX;
            for (int z = originZ - radius; z <= originZ + radius; ++z)
            {
                double zDistance = (double)z + 0.5 - entity->mZ;
                LevelChunk* chunk = level->getChunkLoadedOrUnloadedAt(x, z);
                if (chunk == NULL || chunk->isEmpty())
                {
                    continue;
                }

                int below;
                for (int y = level->getHeight() - 1; y >= 0; y = below)
                {
                    below = y - 1;
                    if (chunk->getBlockId(x & 15, y, z & 15) == BlockId::PORTAL)
                    {
                        while (chunk->getBlockId(x & 15, below, x & 15) == BlockId::PORTAL)
                        {
                            y = below;
                            below = y - 1;
                        }
                        double yDistance = (double)y + 0.5 - (double)originY;
                        double distance = yDistance * yDistance + xDistance * xDistance + zDistance * zDistance;
                        if (bestDistance < 0.0 || distance < bestDistance)
                        {
                            bestDistance = distance;
                            bestX = x;
                            bestY = y;
                            bestZ = z;
                        }
                    }
                }
            }
        }
    }

    if (bestDistance < 0.0)
    {
        return false;
    }

    portalPos = BlockPos(bestX, bestY, bestZ);
    if (addToCache)
    {
        if (portalPositions.size() + 1 > 30)
        {
            uncacheLeastUsedPortal();
        }
        PortalPosition*& cachedPortal = portalCache[chunkPos];
        cachedPortal = new PortalPosition(portalPos, level->getGameTime());
        portalPositions.push_back(chunkPos);
    }

    BlockPattern::BlockPatternMatch* portal =
        static_cast<PortalBlock*>(Blocks::portal)->getPortalShape(level, portalPos);
    bool negative = portal->getForwards()->getClockWise()->getAxisDirection() == Direction::AxisDirection::NEGATIVE;
    double x = (double)portalPos.x + 0.5;
    double z = (double)portalPos.z + 0.5;
    double horizontal;
    if (portal->getForwards()->getAxis() == Direction::Axis::X)
    {
        horizontal = (double)portal->getFrontTopLeft().z;
    }
    else
    {
        horizontal = (double)portal->getFrontTopLeft().x;
    }
    double y = (double)(portal->getFrontTopLeft().y + 1)
               - entity->getPortalEntranceOffset()->y * (double)portal->getHeight();
    if (negative)
    {
        horizontal += 1.0;
    }
    if (portal->getForwards()->getAxis() == Direction::Axis::X)
    {
        z = (1.0 - entity->getPortalEntranceOffset()->x) * (double)portal->getWidth()
                * (double)portal->getForwards()->getClockWise()->getAxisDirection()->getStep()
            + horizontal;
    }
    else
    {
        x = (1.0 - entity->getPortalEntranceOffset()->x) * (double)portal->getWidth()
                * (double)portal->getForwards()->getClockWise()->getAxisDirection()->getStep()
            + horizontal;
    }

    float diagonal = 1.0f;
    float xFromZ = 0.0f;
    float zFromX = 0.0f;
    if (portal->getForwards()->getOpposite() != entity->getPortalEntranceForwards())
    {
        if (portal->getForwards()->getOpposite() == entity->getPortalEntranceForwards()->getOpposite())
        {
            diagonal = -1.0f;
        }
        else if (portal->getForwards()->getOpposite() == entity->getPortalEntranceForwards()->getClockWise())
        {
            diagonal = 0.0f;
            xFromZ = -1.0f;
            zFromX = 1.0f;
        }
        else
        {
            diagonal = 0.0f;
            xFromZ = 1.0f;
            zFromX = -1.0f;
        }
    }

    double oldXd = entity->mXd;
    double oldZd = entity->mZd;
    entity->mXd = oldXd * diagonal + oldZd * xFromZ;
    entity->mZd = oldXd * zFromX + oldZd * diagonal;
    entity->mYRot = rotation - (float)(entity->getPortalEntranceForwards()->getOpposite()->get2DDataValue() * 90)
                    + (float)(portal->getForwards()->get2DDataValue() * 90);

    if (eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, entity->GetType()))
    {
        boost::shared_ptr<ServerPlayer> player = boost::static_pointer_cast<ServerPlayer>(entity);
        player->mConnection->teleport(x, y, z, entity->mYRot, entity->mXRot, true, true);
    }
    else
    {
        entity->moveTo(x, y, z, entity->mYRot, entity->mXRot);
    }
    return true;
}

bool PortalForcer::createPortal(boost::shared_ptr<Entity> entity)
{
    int originX = Mth::floor(entity->mX);
    int originY = Mth::floor(entity->mY);
    int originZ = Mth::floor(entity->mZ);
    int halfSize = (level->mDimension->getXZSize() << 4) >> 1;
    int limit = halfSize - 4;
    int minimumX = originX - 48;
    int maximumX;
    if (minimumX < -limit)
    {
        originX = originX - (minimumX + limit);
        minimumX = originX - 48;
        maximumX = originX + 48;
    }
    else
    {
        maximumX = originX + 48;
        if (limit <= maximumX)
        {
            originX = limit - 48;
            minimumX = originX - 48;
            maximumX = originX + 48;
        }
    }
    if (originZ - 48 < -limit)
    {
        originZ = originZ - ((originZ - 48) + limit);
    }
    else if (limit <= originZ + 48)
    {
        originZ = limit - 48;
    }

    int orientation = random->nextInt(4);
    int bestX = originX;
    int bestY = originY;
    int bestZ = originZ;
    int bestOrientation = 0;
    double bestDistance = -1.0;

    for (int x = minimumX; x <= maximumX; ++x)
    {
        double xDistance = (double)x + 0.5 - entity->mX;
        for (int z = originZ - 48; z <= originZ + 48; ++z)
        {
            double zDistance = (double)z + 0.5 - entity->mZ;
            int y = level->getHeight() - 1;
            while (y >= 0)
            {
                if (!level->isEmptyBlock(BlockPos(x, y, z)))
                {
                    --y;
                    continue;
                }
                int lowest;
                do
                {
                    lowest = y;
                    --y;
                    if (lowest < 1)
                    {
                        break;
                    }
                } while (level->isEmptyBlock(BlockPos(x, y, z)));

                for (int attempt = 0; attempt < 4; ++attempt)
                {
                    int candidateOrientation = orientation + attempt;
                    int xDirection = candidateOrientation % 2;
                    int zDirection = 1 - xDirection;
                    if (candidateOrientation % 4 > 1)
                    {
                        xDirection = -xDirection;
                        zDirection = -zDirection;
                    }

                    bool blocked = false;
                    for (int depth = 0; depth < 3 && !blocked; ++depth)
                    {
                        for (int width = 0; width < 4 && !blocked; ++width)
                        {
                            int testX = x + (width - 1) * xDirection + depth * zDirection;
                            int testZ = z + (width - 1) * zDirection - depth * xDirection;
                            for (int yOffset = -1; yOffset < 4; ++yOffset)
                            {
                                if (testX < -limit || testX >= limit || testZ < -limit || testZ >= limit)
                                {
                                    blocked = true;
                                    break;
                                }
                                if (yOffset < 0)
                                {
                                    if (!level->getBlockState(BlockPos(testX, lowest + yOffset, testZ))
                                             ->getMaterial()
                                             ->isSolid())
                                    {
                                        blocked = true;
                                        break;
                                    }
                                }
                                else if (!level->isEmptyBlock(BlockPos(testX, lowest + yOffset, testZ)))
                                {
                                    blocked = true;
                                    break;
                                }
                            }
                        }
                    }
                    if (blocked)
                    {
                        break;
                    }

                    double yDistance = (double)lowest + 0.5 - entity->mY;
                    double distance = yDistance * yDistance + xDistance * xDistance + zDistance * zDistance;
                    if (bestDistance < 0.0 || distance < bestDistance)
                    {
                        bestDistance = distance;
                        bestX = x;
                        bestY = lowest;
                        bestZ = z;
                        bestOrientation = candidateOrientation % 4;
                    }
                }
            }
        }
    }

    if (bestDistance < 0.0)
    {
        for (int x = minimumX; x <= maximumX; ++x)
        {
            double xDistance = (double)x + 0.5 - entity->mX;
            for (int z = originZ - 48; z <= originZ + 48; ++z)
            {
                double zDistance = (double)z + 0.5 - entity->mZ;
                int y = level->getHeight() - 1;
                while (y >= 0)
                {
                    if (!level->isEmptyBlock(BlockPos(x, y, z)))
                    {
                        --y;
                        continue;
                    }
                    int lowest;
                    do
                    {
                        lowest = y;
                        --y;
                        if (lowest < 1)
                        {
                            break;
                        }
                    } while (level->isEmptyBlock(BlockPos(x, y, z)));

                    for (int attempt = 0; attempt < 2; ++attempt)
                    {
                        int xDirection = (orientation + attempt) % 2;
                        int zDirection = 1 - xDirection;
                        bool blocked = false;
                        for (int width = 0; width < 4 && !blocked; ++width)
                        {
                            int testX = x + (width - 1) * xDirection;
                            int testZ = z + (width - 1) * zDirection;
                            for (int yOffset = -1; yOffset < 4; ++yOffset)
                            {
                                if (testX < -limit || testX >= limit || testZ < -limit || testZ >= limit)
                                {
                                    blocked = true;
                                    break;
                                }
                                if (yOffset < 0)
                                {
                                    if (!level->getBlockState(BlockPos(testX, lowest + yOffset, testZ))
                                             ->getBlock()
                                             ->mMaterial->isSolid())
                                    {
                                        blocked = true;
                                        break;
                                    }
                                }
                                else if (!level->isEmptyBlock(BlockPos(testX, lowest + yOffset, testZ)))
                                {
                                    blocked = true;
                                    break;
                                }
                            }
                        }
                        if (blocked)
                        {
                            break;
                        }

                        double yDistance = (double)lowest + 0.5 - entity->mY;
                        double distance = yDistance * yDistance + xDistance * xDistance + zDistance * zDistance;
                        if (bestDistance < 0.0 || distance < bestDistance)
                        {
                            bestDistance = distance;
                            bestX = x;
                            bestY = lowest;
                            bestZ = z;
                            bestOrientation = (orientation + attempt) % 2;
                        }
                    }
                }
            }
        }
    }

    int xDirection = bestOrientation % 2;
    int zDirection = 1 - xDirection;
    if (bestOrientation % 4 > 1)
    {
        xDirection = -xDirection;
        zDirection = -zDirection;
    }

    if (bestDistance < 0.0)
    {
        bestY = Mth::clamp(bestY, 70, level->getHeight() - 10);
        for (int depth = 0; depth < 3; ++depth)
        {
            for (int width = 0; width < 2; ++width)
            {
                int x = bestX - zDirection + width * xDirection + depth * zDirection;
                int z = bestZ + xDirection + width * zDirection - depth * xDirection;
                level->setBlockAndUpdate(BlockPos(x, bestY - 1, z), Blocks::obsidian->defaultBlockState());
                level->setBlockAndUpdate(BlockPos(x, bestY, z), Blocks::air->defaultBlockState());
                level->setBlockAndUpdate(BlockPos(x, bestY + 1, z), Blocks::air->defaultBlockState());
                level->setBlockAndUpdate(BlockPos(x, bestY + 2, z), Blocks::air->defaultBlockState());
            }
        }
    }

    Direction::Axis* portalAxis = xDirection == 0 ? Direction::Axis::Z : Direction::Axis::X;
    const BlockState* portalState = Blocks::portal->defaultBlockState()->setValue(PortalBlock::AXIS, portalAxis);
    for (int pass = 0; pass < 4; ++pass)
    {
        for (int width = 0; width < 4; ++width)
        {
            int x = bestX - xDirection + width * xDirection;
            int z = bestZ - zDirection + width * zDirection;
            for (int yOffset = -1; yOffset < 4; ++yOffset)
            {
                bool frame = width == 0 || width == 3 || yOffset == -1 || yOffset == 3;
                level->setBlock(BlockPos(x, bestY + yOffset, z),
                                frame ? Blocks::obsidian->defaultBlockState() : portalState, Block::UPDATE_CLIENTS,
                                false);
            }
        }
        for (int width = 0; width < 4; ++width)
        {
            int x = bestX - xDirection + width * xDirection;
            int z = bestZ - zDirection + width * zDirection;
            for (int yOffset = -1; yOffset < 4; ++yOffset)
            {
                BlockPos pos(x, bestY + yOffset, z);
                level->updateNeighborsAt(pos, level->getBlockState(pos)->getBlock(), false);
            }
        }
    }
    return true;
}

void PortalForcer::force(boost::shared_ptr<Entity> entity, float rotation, int chunkX, int chunkZ)
{
    if (level->mDimension->getType() == DimensionType::THE_END)
    {
        int x = Mth::floor(entity->mX);
        int y = Mth::floor(entity->mY) - 1;
        int z = Mth::floor(entity->mZ);
        int xa = 1;
        int za = 0;
        for (int s = -2; s <= 2; ++s)
        {
            for (int b = -2; b <= 2; ++b)
            {
                for (int h = -1; h < 3; ++h)
                {
                    int xx = x + b * xa + s * za;
                    int yy = y + h;
                    int zz = z + b * za - s * xa;
                    bool border = h < 0;
                    level->setBlockAndUpdate(BlockPos(xx, yy, zz), border ? Blocks::obsidian->defaultBlockState()
                                                                          : Blocks::air->defaultBlockState());
                }
            }
        }
        entity->moveTo((double)x, (double)y, (double)z, entity->mYRot, 0.0f);
        entity->mXd = 0.0;
        entity->mYd = 0.0;
        entity->mZd = 0.0;
        return;
    }

    if (findPortal(entity, rotation, chunkX, chunkZ))
    {
        return;
    }

    createPortal(entity);
    findPortal(entity, rotation, chunkX, chunkZ);
}

bool PortalForcer::isBlocked(const BlockPos& pos)
{
    return !static_cast<Level*>(level)->isEmptyBlock(pos) || !static_cast<Level*>(level)->isEmptyBlock(pos.above());
}

void PortalForcer::tick(long long gameTime)
{
    (void)gameTime;
}

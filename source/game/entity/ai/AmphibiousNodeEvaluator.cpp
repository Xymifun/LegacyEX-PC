#include "game/entity/ai/AmphibiousNodeEvaluator.hpp"

#include <algorithm>

#include "game/core/EntityType.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Node.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/Direction.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockPos.hpp"

AmphibiousNodeEvaluator::AmphibiousNodeEvaluator(Entity* entity) : WalkNodeEvaluator(entity)
{
}

void AmphibiousNodeEvaluator::prepare(LevelSource* levelSource, Mob* mob)
{
    WalkNodeEvaluator::prepare(levelSource, mob);
    mob->setPathfindingMalus(BlockPathTypes::WATER, 0.0f);
    mOldWalkableCost = mob->getPathfindingMalus(BlockPathTypes::WALKABLE);
    mob->setPathfindingMalus(BlockPathTypes::WALKABLE, 6.0f);
    mOldWaterBorderCost = mob->getPathfindingMalus(BlockPathTypes::WATER_BORDER);
    mob->setPathfindingMalus(BlockPathTypes::WATER_BORDER, 4.0f);
}

void AmphibiousNodeEvaluator::done()
{
    mMob->setPathfindingMalus(BlockPathTypes::WALKABLE, mOldWalkableCost);
    mMob->setPathfindingMalus(BlockPathTypes::WATER_BORDER, mOldWaterBorderCost);
    WalkNodeEvaluator::done();
}

Node* AmphibiousNodeEvaluator::getStart()
{
    return getNode(Mth::floor(mMob->getBoundingBox()->mMinX), Mth::floor(mMob->getBoundingBox()->mMinY + 0.5),
                   Mth::floor(mMob->getBoundingBox()->mMinZ));
}

Node* AmphibiousNodeEvaluator::getGoal(double x, double y, double z)
{
    return getNode(Mth::floor(x), Mth::floor(y + 0.5), Mth::floor(z));
}

double AmphibiousNodeEvaluator::inWaterDependentPosHeight(const BlockPos& pos)
{
    if (!mMob->isInWater())
    {
        BlockPos below = pos.below();
        const BlockState* state = getBlockState(mEntity->mTickCache, mLevelSource, below);
        return pos.y - (1.0 - state->getShape(mLevelSource, below)->mMaxY);
    }
    return pos.y + 0.5;
}

Node* AmphibiousNodeEvaluator::getAcceptedNode(int x, int y, int z, int verticalRange, double maxFallDistance)
{
    Node* cachedNode = getNode(x, y, z);
    if (cachedNode->hasAcceptedNode)
        return cachedNode->acceptedNode;

    Node* node = NULL;
    BlockPos pos(x, y, z);
    if (inWaterDependentPosHeight(pos) - maxFallDistance > 1.125)
    {
        cachedNode->hasAcceptedNode = true;
        cachedNode->acceptedNode = NULL;
        return NULL;
    }

    const BlockPathTypes* type = WalkNodeEvaluator::getBlockPathType(
        mLevelSource, x, y, z, mMob, mEntityWidth, mEntityHeight, mEntityDepth, isCanOpenDoors(), isCanPassDoors());
    float malus = mMob->getPathfindingMalus(type);
    const double halfWidth = mMob->mBbWidth * 0.5;
    if (malus >= 0.0f)
    {
        node = getNode(x, y, z);
        node->type = type;
        node->costMalus = node->costMalus < malus ? malus : node->costMalus;
    }

    if (type == BlockPathTypes::WATER || type == BlockPathTypes::WALKABLE)
    {
        if (y < mMob->mLevel->getSeaLevel() - 10 && node != NULL)
            node->costMalus += 1.0f;
        cachedNode->hasAcceptedNode = true;
        cachedNode->acceptedNode = node;
        return node;
    }

    if (node == NULL && verticalRange > 0 && type != BlockPathTypes::FENCE && type != BlockPathTypes::TRAPDOOR)
        node = getAcceptedNode(x, y + 1, z, verticalRange - 1, maxFallDistance);

    if (type == BlockPathTypes::OPEN)
    {
        AABB* box = AABB::newTemp(x - halfWidth + 0.5, y + 0.001, z - halfWidth + 0.5, x + halfWidth + 0.5,
                                  static_cast<float>(y) + mMob->mBbHeight, z + halfWidth + 0.5);
        if (mMob->mLevel->hasBlockCubes(box))
        {
            cachedNode->hasAcceptedNode = true;
            cachedNode->acceptedNode = NULL;
            return NULL;
        }

        const BlockPathTypes* below = WalkNodeEvaluator::getBlockPathType(mLevelSource, x, y - 1, z, mMob, mEntityWidth,
                                                                          mEntityHeight, mEntityDepth, isCanOpenDoors(),
                                                                          isCanPassDoors());
        if (below == BlockPathTypes::BLOCKED)
        {
            node = getNode(x, y, z);
            cachedNode->acceptedNode = node;
            cachedNode->hasAcceptedNode = true;
            node->type = BlockPathTypes::WALKABLE;
            node->costMalus = node->costMalus < malus ? malus : node->costMalus;
            return node;
        }

        if (below == BlockPathTypes::WATER)
        {
            node = getNode(x, y, z);
            cachedNode->acceptedNode = node;
            cachedNode->hasAcceptedNode = true;
            node->type = BlockPathTypes::WATER;
            node->costMalus = node->costMalus < malus ? malus : node->costMalus;
            return node;
        }

        int fallDistance = 0;
        while (y > 0 && type == BlockPathTypes::OPEN)
        {
            --y;
            if (fallDistance++ >= mMob->getMaxFallDistance())
            {
                cachedNode->hasAcceptedNode = true;
                cachedNode->acceptedNode = NULL;
                return NULL;
            }
            type = WalkNodeEvaluator::getBlockPathType(mLevelSource, x, y, z, mMob, mEntityWidth, mEntityHeight,
                                                       mEntityDepth, false, false);
            malus = mMob->getPathfindingMalus(type);
            if (malus < 0.0f)
            {
                cachedNode->hasAcceptedNode = true;
                cachedNode->acceptedNode = NULL;
                return NULL;
            }
            if (type != BlockPathTypes::OPEN)
            {
                node = getNode(x, y, z);
                node->type = type;
                node->costMalus = node->costMalus < malus ? malus : node->costMalus;
                break;
            }
        }
    }

    cachedNode->hasAcceptedNode = true;
    cachedNode->acceptedNode = node;
    return node;
}

int AmphibiousNodeEvaluator::getNeighbors(arrayWithLength<Node*>* neighbors, Node* current, Node* target, float maxCost)
{
    int count = 0;
    BlockPos pos(current->x, current->y, current->z);
    double height = inWaterDependentPosHeight(pos);
    Node* south = getAcceptedNode(current->x, current->y, current->z + 1, 1, height);
    Node* west = getAcceptedNode(current->x - 1, current->y, current->z, 1, height);
    Node* east = getAcceptedNode(current->x + 1, current->y, current->z, 1, height);
    Node* north = getAcceptedNode(current->x, current->y, current->z - 1, 1, height);
    Node* up = getAcceptedNode(current->x, current->y + 1, current->z, 0, height);
    Node* down = getAcceptedNode(current->x, current->y - 1, current->z, 1, height);

    if (south != NULL && !south->closed && south->distanceTo(target) < maxCost)
        neighbors->data[count++] = south;
    if (west != NULL && !west->closed && west->distanceTo(target) < maxCost)
        neighbors->data[count++] = west;
    if (east != NULL && !east->closed && east->distanceTo(target) < maxCost)
        neighbors->data[count++] = east;
    if (north != NULL && !north->closed && north->distanceTo(target) < maxCost)
        neighbors->data[count++] = north;
    if (up != NULL && !up->closed && up->distanceTo(target) < maxCost)
        neighbors->data[count++] = up;
    if (down != NULL && !down->closed && down->distanceTo(target) < maxCost)
        neighbors->data[count++] = down;

    bool northOpen = north == NULL || north->type == BlockPathTypes::OPEN || north->costMalus != 0.0f;
    bool southOpen = south == NULL || south->type == BlockPathTypes::OPEN || south->costMalus != 0.0f;
    bool eastOpen = east == NULL || east->type == BlockPathTypes::OPEN || east->costMalus != 0.0f;
    bool westOpen = west == NULL || west->type == BlockPathTypes::OPEN || west->costMalus != 0.0f;

    if (northOpen)
    {
        if (westOpen)
        {
            Node* northWest = getAcceptedNode(current->x - 1, current->y, current->z - 1, 1, height);
            if (northWest != NULL && !northWest->closed && northWest->distanceTo(target) < maxCost)
                neighbors->data[count++] = northWest;
        }
        if (eastOpen)
        {
            Node* northEast = getAcceptedNode(current->x + 1, current->y, current->z - 1, 1, height);
            if (northEast != NULL && !northEast->closed && northEast->distanceTo(target) < maxCost)
                neighbors->data[count++] = northEast;
        }
    }

    if (southOpen)
    {
        if (westOpen)
        {
            Node* southWest = getAcceptedNode(current->x - 1, current->y, current->z + 1, 1, height);
            if (southWest != NULL && !southWest->closed && southWest->distanceTo(target) < maxCost)
                neighbors->data[count++] = southWest;
        }
        if (eastOpen)
        {
            Node* southEast = getAcceptedNode(current->x + 1, current->y, current->z + 1, 1, height);
            if (southEast != NULL && !southEast->closed && southEast->distanceTo(target) < maxCost)
                neighbors->data[count++] = southEast;
        }
    }
    return count;
}

const BlockPathTypes* AmphibiousNodeEvaluator::getBlockPathTypes(LevelSource* levelSource, int x, int y, int z,
                                                                 int sizeX, int sizeY, int sizeZ, bool canOpenDoors,
                                                                 bool canPassDoors, int& pathTypeFlags,
                                                                 const BlockPathTypes* pathType)
{
    BlockPos mobPos(mMob);
    for (int dx = 0; dx < sizeX; ++dx)
        for (int dy = 0; dy < sizeY; ++dy)
            for (int dz = 0; dz < sizeZ; ++dz)
            {
                int blockX = dx + x;
                int blockY = dy + y;
                int blockZ = dz + z;
                int key = BlockPos(blockX, blockY, blockZ).hashCode();
                boost::unordered_map<int, int>::iterator cached = mBlockPathTypeMap.find(key);
                if (cached != mBlockPathTypeMap.end())
                {
                    pathTypeFlags |= cached->second;
                    continue;
                }

                const BlockPathTypes* type = getBlockPathType(levelSource, blockX, blockY, blockZ);
                if (type == BlockPathTypes::RAIL
                    && !levelSource->getBlockState(mobPos)->getBlock()->instanceof(EBLOCK_TYPE_RAIL))
                {
                    if (!levelSource->getBlockState(mobPos.below())->getBlock()->instanceof(EBLOCK_TYPE_RAIL))
                        type = BlockPathTypes::FENCE;
                }

                if (type == BlockPathTypes::DOOR_OPEN || type == BlockPathTypes::DOOR_WOOD_CLOSED
                    || type == BlockPathTypes::DOOR_IRON_CLOSED)
                {
                    bool isTurtle = mMob->GetType() == EntityType::TURTLE;
                    if (isTurtle)
                        type = BlockPathTypes::BLOCKED;
                    else if (type == BlockPathTypes::DOOR_WOOD_CLOSED && canOpenDoors && canPassDoors)
                        type = BlockPathTypes::WALKABLE;
                    else if (type == BlockPathTypes::DOOR_OPEN && !canPassDoors)
                        type = BlockPathTypes::BLOCKED;
                }
                if (dx == 0 && dy == 0 && dz == 0)
                    pathType = type;

                pathTypeFlags |= 1 << type->id;
                mBlockPathTypeMap[key] = 1 << type->id;
            }
    return pathType;
}

const BlockPathTypes* AmphibiousNodeEvaluator::getBlockPathType(LevelSource* levelSource, int x, int y, int z)
{
    const BlockPathTypes* type = getBlockPathTypeRaw(levelSource, x, y, z);
    if (type == BlockPathTypes::WATER)
    {
        for (std::vector<const Direction*>::iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end();
             ++it)
        {
            const Direction* direction = *it;
            if (getBlockPathTypeRaw(levelSource, x + direction->getStepX(), y + direction->getStepY(),
                                    z + direction->getStepZ())
                == BlockPathTypes::BLOCKED)
                return BlockPathTypes::WATER_BORDER;
        }
        return BlockPathTypes::WATER;
    }

    if (type == BlockPathTypes::OPEN && y >= 1)
    {
        Block* blockBelow = levelSource->getBlockState(BlockPos(x, y - 1, z))->getBlock();
        const BlockPathTypes* typeBelow = getBlockPathTypeRaw(levelSource, x, y - 1, z);
        type = BlockPathTypes::WALKABLE;
        if (typeBelow == BlockPathTypes::WALKABLE || typeBelow == BlockPathTypes::OPEN
            || typeBelow == BlockPathTypes::LAVA)
            type = BlockPathTypes::OPEN;
        if (typeBelow == BlockPathTypes::DAMAGE_FIRE || blockBelow == Blocks::magma)
            type = BlockPathTypes::DAMAGE_FIRE;
        if (typeBelow == BlockPathTypes::DAMAGE_CACTUS)
            type = BlockPathTypes::DAMAGE_CACTUS;
    }
    return checkNeighbourBlocks(levelSource, x, y, z, type);
}

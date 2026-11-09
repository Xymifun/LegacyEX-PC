#include "game/entity/ai/FlyNodeEvaluator.hpp"
#include "game/world/block/BlockId.hpp"

#include <algorithm>

#include <boost/unordered_set.hpp>

#include "game/core/Mth.hpp"
#include "game/core/Node.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"

FlyNodeEvaluator::FlyNodeEvaluator(Entity* entity) : WalkNodeEvaluator(entity)
{
}

void FlyNodeEvaluator::prepare(LevelSource* levelSource, Mob* mob)
{
    WalkNodeEvaluator::prepare(levelSource, mob);
    mOldWaterCost = mob->getPathfindingMalus(BlockPathTypes::WATER);
}

void FlyNodeEvaluator::done()
{
    mMob->setPathfindingMalus(BlockPathTypes::WATER, mOldWaterCost);
    WalkNodeEvaluator::done();
}

const BlockPathTypes* FlyNodeEvaluator::getBlockPathType(Mob* mob, int x, int y, int z)
{
    return getBlockPathType(mLevelSource, x, y, z, mob, mEntityWidth, mEntityHeight, mEntityDepth, isCanOpenDoors(),
                            isCanPassDoors());
}

const BlockPathTypes* FlyNodeEvaluator::getBlockPathType(Mob* mob, const BlockPos& pos)
{
    return getBlockPathType(mob, pos.x, pos.y, pos.z);
}

Node* FlyNodeEvaluator::getStart()
{
    int y;
    if (isCanFloat() && mMob->isInWater())
    {
        y = static_cast<int>(mMob->mBoundingBox->mMinY);
        int x = Mth::floor(mMob->mX);
        int z = Mth::floor(mMob->mZ);

        int blockId = mLevelSource->getBlockId(x, y, z);
        while (blockId == BlockId::FLOWING_WATER || blockId == BlockId::WATER)
        {
            ++y;
            blockId = mLevelSource->getBlockId(x, y, z);
        }
    }
    else
    {
        y = Mth::floor(mMob->mBoundingBox->mMinY + 0.5);
    }

    BlockPos start(mMob);
    const BlockPathTypes* startType = getBlockPathType(mMob, start.x, y, start.z);
    if (mMob->getPathfindingMalus(startType) < 0.0f)
    {
        boost::unordered_set<BlockPos> positions;
        positions.insert(BlockPos(mMob->mBoundingBox->mMinX, static_cast<double>(y), mMob->mBoundingBox->mMinZ));
        positions.insert(BlockPos(mMob->mBoundingBox->mMinX, static_cast<double>(y), mMob->mBoundingBox->mMaxZ));
        positions.insert(BlockPos(mMob->mBoundingBox->mMaxX, static_cast<double>(y), mMob->mBoundingBox->mMinZ));
        positions.insert(BlockPos(mMob->mBoundingBox->mMaxX, static_cast<double>(y), mMob->mBoundingBox->mMaxZ));

        for (boost::unordered_set<BlockPos>::const_iterator it = positions.begin(); it != positions.end(); ++it)
        {
            const BlockPathTypes* type = getBlockPathType(mMob, *it);
            if (mMob->getPathfindingMalus(type) >= 0.0f)
                return NodeEvaluator::getNode(it->x, it->y, it->z);
        }
    }

    return NodeEvaluator::getNode(start.x, y, start.z);
}

Node* FlyNodeEvaluator::getGoal(double x, double y, double z)
{
    return NodeEvaluator::getNode(Mth::floor(x), Mth::floor(y), Mth::floor(z));
}

int FlyNodeEvaluator::getNeighbors(arrayWithLength<Node*>* neighbors, Node* current, Node* target, float maxCost)
{
    int count = 0;
    Node* south = getNode(current->x, current->y, current->z + 1);
    Node* west = getNode(current->x - 1, current->y, current->z);
    Node* east = getNode(current->x + 1, current->y, current->z);
    Node* north = getNode(current->x, current->y, current->z - 1);
    Node* up = getNode(current->x, current->y + 1, current->z);
    Node* down = getNode(current->x, current->y - 1, current->z);

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

    bool openNorth = north == NULL || north->costMalus != 0.0f;
    bool openSouth = south == NULL || south->costMalus != 0.0f;
    bool openEast = east == NULL || east->costMalus != 0.0f;
    bool openWest = west == NULL || west->costMalus != 0.0f;
    bool openUp = up == NULL || up->costMalus != 0.0f;
    bool openDown = down == NULL || down->costMalus != 0.0f;

    if (openNorth)
    {
        Node* northWest = openWest ? getNode(current->x - 1, current->y, current->z - 1) : NULL;
        if (northWest != NULL && !northWest->closed && northWest->distanceTo(target) < maxCost)
            neighbors->data[count++] = northWest;

        Node* northEast = openEast ? getNode(current->x + 1, current->y, current->z - 1) : NULL;
        if (northEast != NULL && !northEast->closed && northEast->distanceTo(target) < maxCost)
            neighbors->data[count++] = northEast;
    }

    if (openSouth)
    {
        Node* southWest = openWest ? getNode(current->x - 1, current->y, current->z + 1) : NULL;
        if (southWest != NULL && !southWest->closed && southWest->distanceTo(target) < maxCost)
            neighbors->data[count++] = southWest;

        Node* southEast = openEast ? getNode(current->x + 1, current->y, current->z + 1) : NULL;
        if (southEast != NULL && !southEast->closed && southEast->distanceTo(target) < maxCost)
            neighbors->data[count++] = southEast;
    }

    Node* upNorth = openNorth && openUp ? getNode(current->x, current->y + 1, current->z - 1) : NULL;
    if (upNorth != NULL && !upNorth->closed && upNorth->distanceTo(target) < maxCost)
        neighbors->data[count++] = upNorth;

    Node* upSouth = openSouth && openUp ? getNode(current->x, current->y + 1, current->z + 1) : NULL;
    if (upSouth != NULL && !upSouth->closed && upSouth->distanceTo(target) < maxCost)
        neighbors->data[count++] = upSouth;

    Node* upEast = openEast && openUp ? getNode(current->x + 1, current->y + 1, current->z) : NULL;
    if (upEast != NULL && !upEast->closed && upEast->distanceTo(target) < maxCost)
        neighbors->data[count++] = upEast;

    Node* upWest = openWest && openUp ? getNode(current->x - 1, current->y + 1, current->z) : NULL;
    if (upWest != NULL && !upWest->closed && upWest->distanceTo(target) < maxCost)
        neighbors->data[count++] = upWest;

    Node* downNorth = openNorth && openDown ? getNode(current->x, current->y - 1, current->z - 1) : NULL;
    if (downNorth != NULL && !downNorth->closed && downNorth->distanceTo(target) < maxCost)
        neighbors->data[count++] = downNorth;

    Node* downSouth = openSouth && openDown ? getNode(current->x, current->y - 1, current->z + 1) : NULL;
    if (downSouth != NULL && !downSouth->closed && downSouth->distanceTo(target) < maxCost)
        neighbors->data[count++] = downSouth;

    Node* downEast = openEast && openDown ? getNode(current->x + 1, current->y - 1, current->z) : NULL;
    if (downEast != NULL && !downEast->closed && downEast->distanceTo(target) < maxCost)
        neighbors->data[count++] = downEast;

    Node* downWest = openWest && openDown ? getNode(current->x - 1, current->y - 1, current->z) : NULL;
    if (downWest != NULL && !downWest->closed && downWest->distanceTo(target) < maxCost)
        neighbors->data[count++] = downWest;

    return count;
}

Node* FlyNodeEvaluator::getNode(int x, int y, int z)
{
    Node* node = NULL;
    const BlockPathTypes* type = getBlockPathType(mMob, x, y, z);
    float malus = mMob->getPathfindingMalus(type);
    if (malus >= 0.0f)
    {
        node = NodeEvaluator::getNode(x, y, z);
        node->costMalus = std::max(node->costMalus, malus);
        node->type = type;
        if (type == BlockPathTypes::WALKABLE)
            node->costMalus += 1.0f;
    }
    return node;
}

const BlockPathTypes* FlyNodeEvaluator::getBlockPathType(LevelSource* levelSource, int x, int y, int z, Mob* mob,
                                                         int sizeX, int sizeY, int sizeZ, bool canOpenDoors,
                                                         bool canPassDoors)
{
    int pathTypeFlags = 0;
    const BlockPathTypes* result = BlockPathTypes::BLOCKED;
    BlockPos mobPos(mob);
    result = getBlockPathTypes(levelSource, x, y, z, sizeX, sizeY, sizeZ, canOpenDoors, canPassDoors, pathTypeFlags,
                               result);
    if ((pathTypeFlags & 1 << BlockPathTypes::FENCE->id) != 0)
        return BlockPathTypes::FENCE;

    const BlockPathTypes* best = BlockPathTypes::BLOCKED;
    for (int i = 0; i < static_cast<int>(sizeof(BlockPathTypes::VALUES) / sizeof(BlockPathTypes::VALUES[0])); ++i)
    {
        const BlockPathTypes* type = BlockPathTypes::VALUES[i];
        if ((pathTypeFlags & 1 << type->id) != 0)
        {
            if (mob->getPathfindingMalus(type) < 0.0f)
                return type;
            if (mob->getPathfindingMalus(type) >= mob->getPathfindingMalus(best))
                best = type;
        }
    }

    if (result == BlockPathTypes::OPEN && mob->getPathfindingMalus(best) == 0.0f)
        return BlockPathTypes::OPEN;
    return best;
}

const BlockPathTypes* FlyNodeEvaluator::getBlockPathType(LevelSource* levelSource, int x, int y, int z)
{
    const BlockPathTypes* result = WalkNodeEvaluator::getBlockPathTypeRaw(levelSource, x, y, z);
    if (result == BlockPathTypes::OPEN && y > 0)
    {
        Block* block = levelSource->getBlock(BlockPos(x, y - 1, z));
        const BlockPathTypes* belowType = WalkNodeEvaluator::getBlockPathTypeRaw(levelSource, x, y - 1, z);
        result = belowType == BlockPathTypes::DAMAGE_FIRE || block == Blocks::magma || belowType == BlockPathTypes::LAVA
                     ? BlockPathTypes::DAMAGE_FIRE
                 : belowType == BlockPathTypes::DAMAGE_CACTUS ? BlockPathTypes::DAMAGE_CACTUS
                 : belowType == BlockPathTypes::WALKABLE || belowType == BlockPathTypes::OPEN
                         || belowType == BlockPathTypes::WATER
                     ? BlockPathTypes::OPEN
                     : BlockPathTypes::WALKABLE;
    }

    return WalkNodeEvaluator::checkNeighbourBlocks(levelSource, x, y, z, result);
}

#include "game/core/PathFinder.hpp"

#include <cmath>

#include "game/core/Path.hpp"
#include "game/entity/ai/NodeEvaluator.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

PathFinder::PathFinder(NodeEvaluator* nodeEvaluator)
    : openSet(),
      mNeighbors(new arrayWithLength<Node*>(32, true)),
      mNodeEvaluator(nodeEvaluator)
{
}

PathFinder::~PathFinder()
{
    delete[] mNeighbors->data;
    delete mNeighbors;
}

Path* PathFinder::reconstruct_path(Node* start, Node* end)
{
    unsigned int length = 1;
    Node* node = end;
    while (node->cameFrom != NULL)
    {
        ++length;
        node = node->cameFrom;
    }

    arrayWithLength<Node*> nodes(length, true);
    unsigned int last = length - 1;
    nodes.data[last] = end;
    Node** slot = nodes.data + last;
    for (node = end->cameFrom; node != NULL; node = node->cameFrom)
    {
        --slot;
        *slot = node;
    }

    Path* path = new Path(nodes);
    delete[] nodes.data;
    return path;
}

Path* PathFinder::findPath(Node* start, Node* end, float maxDistance)
{
    start->g = 0.0f;
    start->h = (float)start->distanceManhattan(end);
    start->f = start->h;
    openSet.clear();
    openSet.insert(start);

    Node* best = start;
    int iterations = 0;
    while (!openSet.isEmpty() && ++iterations < 200)
    {
        Node* current = openSet.pop();
        if (current->equals(end))
        {
            best = end;
            break;
        }
        if (current->distanceManhattan(end) < best->distanceManhattan(end))
        {
            best = current;
        }
        current->closed = true;
        int count = mNodeEvaluator->getNeighbors(mNeighbors, current, end, maxDistance);
        for (int i = 0; i < count; ++i)
        {
            Node* neighbor = mNeighbors->data[i];
            float distance = (float)current->distanceManhattan(neighbor);
            neighbor->walkedDistance = current->walkedDistance + distance;
            neighbor->cost = distance + neighbor->costMalus;
            float totalCost = current->g + neighbor->cost;
            if (neighbor->walkedDistance < maxDistance && (!neighbor->inOpenSet() || totalCost < neighbor->g))
            {
                neighbor->cameFrom = current;
                neighbor->g = totalCost;
                neighbor->h = (float)neighbor->distanceManhattan(end) + neighbor->costMalus;
                if (!neighbor->inOpenSet())
                {
                    neighbor->f = neighbor->g + neighbor->h;
                    openSet.insert(neighbor);
                }
                else
                {
                    openSet.changeCost(neighbor, neighbor->g + neighbor->h);
                }
            }
        }
    }

    if (best == start)
    {
        return NULL;
    }
    return reconstruct_path(start, best);
}

Path* PathFinder::findPath(LevelSource* levelSource, Mob* mob, double x, double y, double z, float maxDistance)
{
    openSet.clear();
    mNodeEvaluator->prepare(levelSource, mob);
    Node* start = mNodeEvaluator->getStart();
    Block* block = levelSource->getBlock(BlockPos(mob->mX, mob->mY, mob->mZ));
    if (block->instanceof(EBLOCK_TYPE_FENCE))
    {
        int mobX = (int)floor(mob->mX + 0.5);
        int mobZ = (int)floor(mob->mZ + 0.5);
        if ((double)mobX == floor(mob->mX))
            --mobX;
        if ((double)mobZ == floor(mob->mZ))
            --mobZ;
        Node* adjusted = mNodeEvaluator->getNode(mobX, start->y, mobZ);
        if (adjusted != NULL)
        {
            start = adjusted;
        }
    }
    Node* goal = mNodeEvaluator->getGoal(x, y, z);
    Path* path = findPath(start, goal, maxDistance);
    mNodeEvaluator->done();
    return path;
}

Path* PathFinder::findPath(LevelSource* levelSource, Mob* mob, const BlockPos& pos, float maxDistance)
{
    return findPath(levelSource, mob, (float)pos.x + 0.5f, (float)pos.y + 0.5f, (float)pos.z + 0.5f, maxDistance);
}

Path* PathFinder::findPath(LevelSource* levelSource, Mob* mob, Entity* entity, float maxDistance)
{
    return findPath(levelSource, mob, entity->mX, entity->mBoundingBox->mMinY, entity->mZ, maxDistance);
}

Path* PathFinder::findDirectPath(LevelSource* levelSource, Mob* mob, const BlockPos& pos, float maxDistance)
{
    openSet.clear();
    mNodeEvaluator->prepare(levelSource, mob);
    Node* start = mNodeEvaluator->getStart();
    Node* goal = mNodeEvaluator->getDirectGoal(pos);
    Path* path = findPath(start, goal, maxDistance);
    mNodeEvaluator->done();
    return path;
}

#ifndef GAME_CORE_PATHFINDER_HPP
#define GAME_CORE_PATHFINDER_HPP

#include "game/core/BinaryHeap.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class BlockPos;
class Entity;
class LevelSource;
class Mob;
struct Node;
class NodeEvaluator;
class Path;

class PathFinder
{
public:
    PathFinder(NodeEvaluator* nodeEvaluator);
    ~PathFinder();

    Path* reconstruct_path(Node* start, Node* end);
    Path* findPath(Node* start, Node* end, float maxDistance);
    Path* findPath(LevelSource* levelSource, Mob* mob, double x, double y, double z, float maxDistance);
    Path* findPath(LevelSource* levelSource, Mob* mob, const BlockPos& pos, float maxDistance);
    Path* findPath(LevelSource* levelSource, Mob* mob, Entity* entity, float maxDistance);
    Path* findDirectPath(LevelSource* levelSource, Mob* mob, const BlockPos& pos, float maxDistance);
    void setNodeEvaluator(NodeEvaluator* nodeEvaluator);

private:
    friend class PathNavigationListener;

    BinaryHeap openSet;
    arrayWithLength<Node*>* mNeighbors;
    NodeEvaluator* mNodeEvaluator;
};

#endif

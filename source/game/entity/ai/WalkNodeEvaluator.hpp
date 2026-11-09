#ifndef GAME_ENTITY_AI_WALKNODEEVALUATOR_HPP
#define GAME_ENTITY_AI_WALKNODEEVALUATOR_HPP

#include "game/entity/ai/NodeEvaluator.hpp"

class Entity;
class Direction;
class BlockPathTypes;

class WalkNodeEvaluator : public NodeEvaluator
{
public:
    WalkNodeEvaluator(Entity* entity);

    virtual void prepare(LevelSource* levelSource, Mob* mob);
    virtual void done();
    virtual Node* getStart();
    virtual Node* getGoal(double x, double y, double z);
    virtual int getNeighbors(arrayWithLength<Node*>* neighbors, Node* current, Node* target, float maxCost);
    virtual const BlockPathTypes* getBlockPathType(LevelSource* levelSource, int x, int y, int z, Mob* mob, int sizeX,
                                                   int sizeY, int sizeZ, bool canOpenDoors, bool canPassDoors);
    virtual const BlockPathTypes* getBlockPathType(LevelSource* levelSource, int x, int y, int z);
    virtual void blockChanged(const BlockPos& pos);

    const BlockPathTypes* getBlockPathType(Mob* mob, int x, int y, int z);
    const BlockPathTypes* getBlockPathType(Mob* mob, const BlockPos& pos);
    Node* getLandNode(int x, int y, int z, int verticalRange, double maxFallDistance, const Direction* direction);
    virtual const BlockPathTypes* getBlockPathTypes(LevelSource* levelSource, int x, int y, int z, int sizeX, int sizeY,
                                                    int sizeZ, bool canOpenDoors, bool canPassDoors, int& pathTypeFlags,
                                                    const BlockPathTypes* pathType);
    const BlockPathTypes* getBlockPathTypeRaw(LevelSource* levelSource, int x, int y, int z);
    const BlockPathTypes* checkNeighbourBlocks(LevelSource* levelSource, int x, int y, int z,
                                               const BlockPathTypes* pathType);

protected:
    float mOldWaterCost;

private:
    bool mRailAtStart;
    bool mRailBelowStart;

protected:
    Entity* mEntity;
};

#endif

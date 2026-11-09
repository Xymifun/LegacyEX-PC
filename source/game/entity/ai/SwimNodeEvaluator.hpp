#ifndef GAME_ENTITY_AI_SWIMNODEEVALUATOR_HPP
#define GAME_ENTITY_AI_SWIMNODEEVALUATOR_HPP

#include "game/entity/ai/NodeEvaluator.hpp"

class BlockPathTypes;

class SwimNodeEvaluator : public NodeEvaluator
{
public:
    SwimNodeEvaluator(bool allowBreaching);

    virtual Node* getStart();
    virtual Node* getGoal(double x, double y, double z);
    virtual Node* getDirectGoal(const BlockPos& pos);
    virtual int getNeighbors(arrayWithLength<Node*>* neighbors, Node* current, Node* target, float maxCost);
    virtual const BlockPathTypes* getBlockPathType(LevelSource* levelSource, int x, int y, int z, Mob* mob, int sizeX,
                                                   int sizeY, int sizeZ, bool canOpenDoors, bool canPassDoors);
    virtual const BlockPathTypes* getBlockPathType(LevelSource* levelSource, int x, int y, int z);
    virtual void blockChanged(const BlockPos& pos);

private:
    const BlockPathTypes* getPathTypeAtPos(int x, int y, int z, bool checkNeighbors);
    const BlockPathTypes* isFree(int x, int y, int z);
    Node* getWaterNode(int x, int y, int z);

    bool mAllowBreaching;
};

#endif

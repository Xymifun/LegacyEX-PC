#ifndef GAME_ENTITY_AI_FLYNODEEVALUATOR_HPP
#define GAME_ENTITY_AI_FLYNODEEVALUATOR_HPP

#include "game/entity/ai/WalkNodeEvaluator.hpp"

class Entity;

class FlyNodeEvaluator : public WalkNodeEvaluator
{
public:
    FlyNodeEvaluator(Entity* entity);

    virtual void prepare(LevelSource* levelSource, Mob* mob);
    virtual void done();
    virtual Node* getNode(int x, int y, int z);
    virtual Node* getStart();
    virtual Node* getGoal(double x, double y, double z);
    virtual int getNeighbors(arrayWithLength<Node*>* neighbors, Node* current, Node* target, float maxCost);
    virtual const BlockPathTypes* getBlockPathType(LevelSource* levelSource, int x, int y, int z, Mob* mob, int sizeX,
                                                   int sizeY, int sizeZ, bool canOpenDoors, bool canPassDoors);
    virtual const BlockPathTypes* getBlockPathType(LevelSource* levelSource, int x, int y, int z);

    const BlockPathTypes* getBlockPathType(Mob* mob, int x, int y, int z);
    const BlockPathTypes* getBlockPathType(Mob* mob, const BlockPos& pos);
};

#endif

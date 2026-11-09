#ifndef GAME_ENTITY_AI_AMPHIBIOUSNODEEVALUATOR_HPP
#define GAME_ENTITY_AI_AMPHIBIOUSNODEEVALUATOR_HPP

#include "game/entity/ai/WalkNodeEvaluator.hpp"

class BlockPos;

class AmphibiousNodeEvaluator : public WalkNodeEvaluator
{
public:
    AmphibiousNodeEvaluator(Entity* entity);

    virtual void prepare(LevelSource* levelSource, Mob* mob);
    virtual void done();
    virtual Node* getStart();
    virtual Node* getGoal(double x, double y, double z);
    virtual int getNeighbors(arrayWithLength<Node*>* neighbors, Node* current, Node* target, float maxCost);
    virtual const BlockPathTypes* getBlockPathTypes(LevelSource* levelSource, int x, int y, int z, int sizeX, int sizeY,
                                                    int sizeZ, bool canOpenDoors, bool canPassDoors, int& pathTypeFlags,
                                                    const BlockPathTypes* pathType);
    virtual const BlockPathTypes* getBlockPathType(LevelSource* levelSource, int x, int y, int z);

    double inWaterDependentPosHeight(const BlockPos& pos);
    Node* getAcceptedNode(int x, int y, int z, int verticalRange, double maxFallDistance);

private:
    float mOldWalkableCost;
    float mOldWaterBorderCost;
};

#endif

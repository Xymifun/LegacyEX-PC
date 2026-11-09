#ifndef GAME_WORLD_GENERATION_LAYERGENERATIONRULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_LAYERGENERATIONRULEDEFINITION_HPP

#include <vector>

#include "game/minigame/GameRuleDefinition.hpp"

class AABB;
class AnyCombinationOfRuleDefinition;
class CombinationDefinitionRuleDefinition;
class LayerFill;
class LayerGenerator;
class LayerSize;
class Random;

class LayerGenerationRuleDefinition : public GameRuleDefinition
{
public:
    LayerGenerationRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);
    std::vector<LayerGenerator*>* CreateLayers(Random& random, const AABB* area, unsigned int layerCount,
                                               float scale) const;

private:
    AnyCombinationOfRuleDefinition* mDefinition;
    std::vector<CombinationDefinitionRuleDefinition*> mCombinationDefinitions;
    int mLayerHeight;
    int mLayerDirection;
};

#endif

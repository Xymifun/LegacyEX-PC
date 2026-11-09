#ifndef GAME_WORLD_GENERATION_SIMPLEPATCHESLAYERTHEMERULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_SIMPLEPATCHESLAYERTHEMERULEDEFINITION_HPP

#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"
#include "game/world/generation/LayerTheme.hpp"

class SimplePatchesLayerThemeRuleDefinition : public LayerAttributeChildRuleDefinition<LayerTheme*>
{
public:
    SimplePatchesLayerThemeRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual LayerTheme* Generate() const;

private:
    int mBlockId;
    int mBlockAux;
    float mNoiseScale;
    float mPatchThreshold;
};

#endif

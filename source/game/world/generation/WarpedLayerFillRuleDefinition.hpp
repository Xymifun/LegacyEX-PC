#ifndef GAME_WORLD_GENERATION_WARPEDLAYERFILLRULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_WARPEDLAYERFILLRULEDEFINITION_HPP

#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"

class LayerFill;

class WarpedLayerFillRuleDefinition : public LayerAttributeChildRuleDefinition<LayerFill*>
{
public:
    WarpedLayerFillRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual LayerFill* Generate() const;

private:
    float mNoiseScale;
    int mOffset;
    int mThickness;
};

#endif

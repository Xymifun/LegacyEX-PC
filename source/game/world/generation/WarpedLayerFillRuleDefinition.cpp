#include "game/world/generation/WarpedLayerFillRuleDefinition.hpp"

#include "game/core/GameRules_FloatAttribute.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/world/generation/WarpedLayerFill.hpp"

WarpedLayerFillRuleDefinition::WarpedLayerFillRuleDefinition() : mNoiseScale(10.0f), mOffset(4), mThickness(1)
{
    addFloatAttribute(ConsoleGameRules::eGameRuleAttr_scale, &mNoiseScale)->setMin(1.0f)->setMax(254.0f);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_dY, &mOffset)->setMin(0)->setMax(9);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_thickness, &mThickness)->setMin(1)->setMax(9);
}

ConsoleGameRules::EGameRuleType WarpedLayerFillRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_WarpedLayerFill;
}

LayerFill* WarpedLayerFillRuleDefinition::Generate() const
{
    return new WarpedLayerFill(mNoiseScale, mOffset, mThickness);
}

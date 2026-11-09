#include "game/world/generation/LayerFillRuleDefinition.hpp"

#include "game/world/generation/BasicLayerFillRuleDefinition.hpp"
#include "game/world/generation/CurvedLayerFillRuleDefinition.hpp"
#include "game/world/generation/WarpedLayerFillRuleDefinition.hpp"

LayerFillRuleDefinition::LayerFillRuleDefinition()
    : LayerAttributeRuleDefinition<LayerFill*>(ConsoleGameRules::eGameRuleType_LayerFill, CreateFuncImpl)
{
}

LayerAttributeChildRuleDefinition<LayerFill*>* LayerFillRuleDefinition::CreateFuncImpl(
    ConsoleGameRules::EGameRuleType type)
{
    switch (type)
    {
    case ConsoleGameRules::eGameRuleType_BasicLayerFill:
        return new BasicLayerFillRuleDefinition();

    case ConsoleGameRules::eGameRuleType_CurvedLayerFill:
        return new CurvedLayerFillRuleDefinition();

    case ConsoleGameRules::eGameRuleType_WarpedLayerFill:
        return new WarpedLayerFillRuleDefinition();

    default:
        break;
    }

    return NULL;
}

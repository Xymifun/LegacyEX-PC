#include "game/world/generation/LayerSizeRuleDefinition.hpp"

#include "game/minigame/LinearBlendSizeRuleDefinition.hpp"
#include "game/minigame/RandomizeSizeRuleDefinition.hpp"
#include "game/world/generation/UniformLayerSizeRuleDefinition.hpp"

LayerSizeRuleDefinition::LayerSizeRuleDefinition()
    : LayerAttributeRuleDefinition<LayerSize*>(ConsoleGameRules::eGameRuleType_LayerSize, CreateFuncImpl)
{
}

LayerAttributeChildRuleDefinition<LayerSize*>* LayerSizeRuleDefinition::CreateFuncImpl(
    ConsoleGameRules::EGameRuleType type)
{
    switch (type)
    {
    case ConsoleGameRules::eGameRuleType_UniformSize:
        return new UniformLayerSizeRuleDefinition();

    case ConsoleGameRules::eGameRuleType_RandomizeSize:
        return new RandomizeSizeRuleDefinition();

    case ConsoleGameRules::eGameRuleType_LinearBlendSize:
        return new LinearBlendSizeRuleDefinition();

    default:
        break;
    }

    return NULL;
}

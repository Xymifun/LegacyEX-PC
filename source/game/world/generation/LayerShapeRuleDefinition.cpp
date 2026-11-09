#include "game/world/generation/LayerShapeRuleDefinition.hpp"

#include "game/minigame/BasicShapeRuleDefinition.hpp"
#include "game/minigame/PatchyShapeRuleDefinition.hpp"
#include "game/minigame/RingShapeRuleDefinition.hpp"
#include "game/minigame/SpiralShapeRuleDefinition.hpp"
#include "game/minigame/StarShapeRuleDefinition.hpp"

LayerShapeRuleDefinition::LayerShapeRuleDefinition()
    : LayerAttributeRuleDefinition<LayerShape*>(ConsoleGameRules::eGameRuleType_LayerShape, CreateFuncImpl)
{
}

LayerAttributeChildRuleDefinition<LayerShape*>* LayerShapeRuleDefinition::CreateFuncImpl(
    ConsoleGameRules::EGameRuleType type)
{
    switch (type)
    {
    case ConsoleGameRules::eGameRuleType_BasicShape:
        return new BasicShapeRuleDefinition();

    case ConsoleGameRules::eGameRuleType_StarShape:
        return new StarShapeRuleDefinition();

    case ConsoleGameRules::eGameRuleType_PatchyShape:
        return new PatchyShapeRuleDefinition();

    case ConsoleGameRules::eGameRuleType_RingShape:
        return new RingShapeRuleDefinition();

    case ConsoleGameRules::eGameRuleType_SpiralShape:
        return new SpiralShapeRuleDefinition();
    }

    return NULL;
}

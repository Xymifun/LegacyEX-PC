#include "game/world/generation/BasicLayerFillRuleDefinition.hpp"

#include "game/core/IGameRuleAttribute.hpp"
#include "game/world/generation/FunctionLayerFill.hpp"

namespace BasicLayerFillRuleDefinitionConstants
{
const ConsoleGameRules::EGameRuleAttr LayerFillTypeAttribute = ConsoleGameRules::eGameRuleAttr_type;
const ConsoleGameRules::EGameRuleType BasicLayerFillActionType = ConsoleGameRules::eGameRuleType_BasicLayerFill;
LayerFill* const NoGeneratedLayerFill = NULL;
}

BasicLayerFillRuleDefinition::BasicLayerFillRuleDefinition() : mType(L"")
{
    addStringAttribute(BasicLayerFillRuleDefinitionConstants::LayerFillTypeAttribute, &mType);
}

ConsoleGameRules::EGameRuleType BasicLayerFillRuleDefinition::getActionType()
{
    return BasicLayerFillRuleDefinitionConstants::BasicLayerFillActionType;
}

LayerFill* BasicLayerFillRuleDefinition::Generate() const
{
    if (mType.compare(L"ReverseCone") == 0)
    {
        return new FunctionLayerFill(FunctionLayerFill::ISLAND);
    }
    else if (mType.compare(L"Uniform1") == 0)
    {
        return new FunctionLayerFill(FunctionLayerFill::UNIFORM);
    }
    else
    {
        return NULL;
    }
}

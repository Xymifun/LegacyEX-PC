#include "game/minigame/LinearBlendSizeRuleDefinition.hpp"

#include "game/core/LinearBlendSize.hpp"

LinearBlendSizeRuleDefinition::LinearBlendSizeRuleDefinition() : mMaxSize(-1), mMinSize(-1)
{
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_upper, &mMaxSize);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_lower, &mMinSize);
}

ConsoleGameRules::EGameRuleType LinearBlendSizeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_LinearBlendSize;
}

LayerSize* LinearBlendSizeRuleDefinition::Generate() const
{
    return new LinearBlendSize(mMinSize, mMaxSize);
}

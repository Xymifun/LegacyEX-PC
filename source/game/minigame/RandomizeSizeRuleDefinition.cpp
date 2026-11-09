#include "game/minigame/RandomizeSizeRuleDefinition.hpp"

#include "game/util/RandomizeSize.hpp"

RandomizeSizeRuleDefinition::RandomizeSizeRuleDefinition() : mAmount(-1.0f), mChild(NULL)
{
}

ConsoleGameRules::EGameRuleType RandomizeSizeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_RandomizeSize;
}

LayerSize* RandomizeSizeRuleDefinition::Generate() const
{
    if (mChild == NULL)
    {
        return NULL;
    }

    return new RandomizeSize(mChild->Generate(), mAmount);
}

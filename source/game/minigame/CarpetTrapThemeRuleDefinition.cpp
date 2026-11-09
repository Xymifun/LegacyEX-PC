#include "game/minigame/CarpetTrapThemeRuleDefinition.hpp"

#include "game/core/GameRules_FloatAttribute.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/world/generation/CarpetTrapLayerTheme.hpp"

CarpetTrapThemeRuleDefinition::CarpetTrapThemeRuleDefinition() : mCarpetColour(0)
{
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_blockAux, &mCarpetColour)->setMin(0)->setMax(15);

    addFloatAttribute(ConsoleGameRules::eGameRuleAttr_scale, &mNoiseScale)->setMin(0.0f)->setMax(100.0f);
}

ConsoleGameRules::EGameRuleType CarpetTrapThemeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_CarpetTrapTheme;
}

LayerTheme* CarpetTrapThemeRuleDefinition::Generate() const
{
    return new CarpetTrapLayerTheme(5, mNoiseScale, mCarpetColour);
}

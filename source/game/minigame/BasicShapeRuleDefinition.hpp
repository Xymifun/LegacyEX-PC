#ifndef GAME_MINIGAME_BASICSHAPERULEDEFINITION_HPP
#define GAME_MINIGAME_BASICSHAPERULEDEFINITION_HPP

#include <string>

#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"
#include "game/world/generation/LayerShape.hpp"

class BasicShapeRuleDefinition : public LayerAttributeChildRuleDefinition<LayerShape*>
{
public:
    BasicShapeRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();

    virtual LayerShape* Generate() const;

protected:
    std::wstring mShape;
};

#endif

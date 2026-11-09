#ifndef GAME_MINIGAME_PATCHYSHAPERULEDEFINITION_HPP
#define GAME_MINIGAME_PATCHYSHAPERULEDEFINITION_HPP

#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"
#include "game/world/generation/LayerShape.hpp"

class PatchyShapeRuleDefinition : public LayerAttributeChildRuleDefinition<LayerShape*>
{
public:
    PatchyShapeRuleDefinition();


    virtual ConsoleGameRules::EGameRuleType getActionType();

    virtual LayerShape* Generate() const;

protected:
    double mScale;
};

#endif

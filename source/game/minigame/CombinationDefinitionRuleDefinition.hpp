#ifndef GAME_MINIGAME_COMBINATIONDEFINITIONRULEDEFINITION_HPP
#define GAME_MINIGAME_COMBINATIONDEFINITIONRULEDEFINITION_HPP

#include <string>
#include "game/minigame/AnyCombinationOfRuleDefinition.hpp"

class AnyCombinationOfRuleDefinition;
class LayerGenerationRuleDefinition;

class CombinationDefinitionRuleDefinition : public AnyCombinationOfRuleDefinition
{
public:
    CombinationDefinitionRuleDefinition(LayerGenerationRuleDefinition* parent);


    virtual ConsoleGameRules::EGameRuleType getActionType();

    const std::wstring& GetName() const;

private:
    std::wstring mDefinitionName;
};
#endif

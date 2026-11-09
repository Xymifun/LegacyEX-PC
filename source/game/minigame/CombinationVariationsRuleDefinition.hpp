#ifndef GAME_MINIGAME_COMBINATIONVARIATIONSRULEDEFINITION_HPP
#define GAME_MINIGAME_COMBINATIONVARIATIONSRULEDEFINITION_HPP

#include <vector>

#include "game/minigame/GameRuleDefinition.hpp"

class AnyCombinationOfRuleDefinition;
class LayerGenerationRuleDefinition;

class CombinationVariationsRuleDefinition : public GameRuleDefinition
{
public:
    CombinationVariationsRuleDefinition(LayerGenerationRuleDefinition* parent,
                                        AnyCombinationOfRuleDefinition* definition);

    std::vector<AnyCombinationOfRuleDefinition*>* GetCombinations() const;
    bool IsForWholeLevel() const;
    virtual void writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount);
    virtual void addAttribute(const std::wstring& name, const std::wstring& value);
    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);
    virtual ConsoleGameRules::EGameRuleType getActionType();

private:
    LayerGenerationRuleDefinition* mParent;
    AnyCombinationOfRuleDefinition* mDefinition;
    bool mForWholeLevel;
    std::vector<AnyCombinationOfRuleDefinition*> mCombinations;
};

#endif

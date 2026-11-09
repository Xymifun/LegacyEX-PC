#ifndef GAME_WORLD_GENERATION_LAYERATTRIBUTERULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_LAYERATTRIBUTERULEDEFINITION_HPP

#include <string>
#include <vector>

#include "game/data/GameRuleSaveInterface.hpp"
#include "game/minigame/GameRuleDefinition.hpp"

template <class T>
class LayerAttributeChildRuleDefinition;

template <class T>
class LayerAttributeRuleDefinition : public GameRuleDefinition
{
public:
    typedef LayerAttributeChildRuleDefinition<T>* (*CreateChildFunc)(ConsoleGameRules::EGameRuleType type);

    LayerAttributeRuleDefinition(ConsoleGameRules::EGameRuleType actionType, CreateChildFunc createChild);
    virtual ~LayerAttributeRuleDefinition() {}

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount);
    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);
    virtual void addAttribute(const std::wstring& name, const std::wstring& value);

private:
    ConsoleGameRules::EGameRuleType mActionType;
    bool mWholeLevel;
    CreateChildFunc mCreateChild;
    std::vector<LayerAttributeChildRuleDefinition<T>*> mChildren;
};

#endif

#ifndef GAME_MINIGAME_ALLOWINRULEDEFINITION_HPP
#define GAME_MINIGAME_ALLOWINRULEDEFINITION_HPP

#include <string>
#include <vector>

#include "game/data/GameRuleSaveInterface.hpp"
#include "game/minigame/GameRuleDefinition.hpp"

class AllowInRuleDefinition : public GameRuleDefinition
{
public:
    AllowInRuleDefinition();


    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount);
    virtual void addAttribute(const std::wstring& name, const std::wstring& value);

    const std::vector<std::wstring>& getTargetLayers() const;
    const std::vector<std::wstring>& getPermissions() const;

private:
    std::vector<std::wstring> mTargetLayers;
    std::vector<std::wstring> mPermissions;
};

#endif

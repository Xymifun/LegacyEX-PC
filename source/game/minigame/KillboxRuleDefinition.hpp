#ifndef GAME_MINIGAME_KILLBOXRULEDEFINITION_HPP
#define GAME_MINIGAME_KILLBOXRULEDEFINITION_HPP

#include <string>

#include "game/minigame/NamedAreaRuleDefinition.hpp"

class AABB;

class KillboxRuleDefinition : public NamedAreaRuleDefinition
{
public:
    KillboxRuleDefinition(AABB* legacyArea = NULL);

    virtual ~KillboxRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void WriteAttributesAsXML(std::string& xml);

    void ConvertFromLegacy(AABB* legacyArea);

    void GetDeathMessageIDS(int& deathMessageIDS, std::wstring& deathMessage);

protected:
    int mDeathMessageIDS;
};

#endif

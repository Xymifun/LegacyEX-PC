#ifndef GAME_MINIGAME_TARGETAREARULEDEFINITION_HPP
#define GAME_MINIGAME_TARGETAREARULEDEFINITION_HPP

#include <string>

#include "game/minigame/NamedAreaRuleDefinition.hpp"

class TargetAreaRuleDefinition : public NamedAreaRuleDefinition
{
    friend class CommonMasterGameMode;
    friend class PlayersEnteredVolumeCondition;

public:
    TargetAreaRuleDefinition();

    virtual ~TargetAreaRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void WriteAttributesAsXML(std::string& xml);
    virtual int getPointValue();
    virtual int getSize();

    bool mPrimary;

protected:
    int mPointValue;
    int mLaps;
};

#endif

#ifndef GAME_WORLD_GENERATION_PERCENTAGEPLAYERSVOTEDCONDITION_HPP
#define GAME_WORLD_GENERATION_PERCENTAGEPLAYERSVOTEDCONDITION_HPP

#include "game/core/IntegerCondition.hpp"

class PercentagePlayersVotedCondition : public IntegerCondition
{
public:
    PercentagePlayersVotedCondition(const XmlObject& object);
    virtual ~PercentagePlayersVotedCondition();

    virtual int GetValue(MasterGameMode* gameMode) const;
};

#endif

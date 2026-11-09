#include "game/world/generation/PercentagePlayersVotedCondition.hpp"

PercentagePlayersVotedCondition::PercentagePlayersVotedCondition(const XmlObject& object) : IntegerCondition(object)
{
}

PercentagePlayersVotedCondition::~PercentagePlayersVotedCondition()
{
}

int PercentagePlayersVotedCondition::GetValue(MasterGameMode* gameMode) const
{
    return 50;
}

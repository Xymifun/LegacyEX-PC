#include "game/core/TeamsCondition.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/MasterGameMode.hpp"

TeamsCondition::~TeamsCondition()
{
}

int TeamsCondition::GetValue(MasterGameMode* gameMode) const
{
    return gameMode->GetActiveTeamCount();
}

void TeamsCondition::AddIntParams(MasterGameMode* gameMode, std::vector<int>* params) const
{
    params->push_back(GetThreshold(gameMode));
    int message = IDS_MG_TEAMS;
    params->push_back(message);
}

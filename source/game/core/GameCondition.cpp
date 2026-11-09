#include "game/core/GameCondition.hpp"

#include "game/core/MasterGameMode.hpp"

bool GameCondition::UpdateMessage(MasterGameMode* gameMode, int message) const
{
    std::vector<int> params;
    params.push_back(0);
    AddIntParams(gameMode, &params);
    params.push_back(message);
    gameMode->BroadcastChat(GetMessage(), params);
    return true;
}

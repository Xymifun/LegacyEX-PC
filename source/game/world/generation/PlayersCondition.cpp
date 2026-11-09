#include "game/world/generation/PlayersCondition.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/GameModeOptions.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/XmlObject.hpp"
#include "game/core/XmlValue.hpp"

PlayersCondition::PlayersCondition(const XmlObject& object) : IntegerCondition(object)
{
    m_activeOnly = object[L"activeOnly"].asBool();
    m_readyOnly = object[L"readyOnly"].asBool();
}

PlayersCondition::~PlayersCondition()
{
}

int PlayersCondition::GetValue(MasterGameMode* gameMode) const
{
    return gameMode->GetPlayerCount(m_activeOnly, m_readyOnly);
}

void PlayersCondition::AddIntParams(MasterGameMode* gameMode, std::vector<int>* params) const
{
    int threshold = GetThreshold(gameMode);
    params->push_back(threshold);
    int stringId = IDS_MG_PLAYERS;
    params->push_back(stringId);
}

bool PlayersCondition::IsSatisfied(MasterGameMode* gameMode) const
{
    bool satisfied = true;
    if (Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) == 0
        && !IntegerCondition::IsSatisfied(gameMode))
    {
        satisfied = false;
    }
    return satisfied != false;
}

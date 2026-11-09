#include "game/core/ConditionConnective.hpp"

#include "game/core/LastKillTimeCondition.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/PostGameTimeCondition.hpp"
#include "game/core/RoundTimeCondition.hpp"
#include "game/core/ShowdownTimeCondition.hpp"
#include "game/core/TeamsCondition.hpp"
#include "game/core/XmlChild.hpp"
#include "game/core/XmlObject.hpp"
#include "game/world/generation/PercentagePlayersVotedCondition.hpp"
#include "game/world/generation/PlayersCondition.hpp"
#include "game/world/generation/PlayersEnteredVolumeCondition.hpp"
#include "game/world/generation/TimeSincePlayersCondition.hpp"
#include "game/core/StringUtils.hpp"

ConditionConnective::ConditionConnective(const XmlObject& object)
{
    m_type = parseType(object.mName).second;

    for (std::vector<XmlChild>::const_iterator child = object.getChildrenRaw().begin();
         child != object.getChildrenRaw().end(); ++child)
    {
        if (child->m_type == 1)
        {
            GameCondition* condition = parseCondition(*child->xml());
            if (condition != NULL)
                m_conditions.push_back(condition);
        }
    }
}

ConditionConnective::~ConditionConnective()
{
    for (std::vector<GameCondition*>::iterator it = m_conditions.begin(); it != m_conditions.end(); ++it)
        delete *it;
}

std::pair<bool, int> ConditionConnective::parseType(std::wstring name)
{
    if (name == L"Conjunction")
        return std::make_pair(true, 0);
    if (name == L"Disjunction")
        return std::make_pair(true, 1);
    return std::make_pair(false, 0);
}

GameCondition* ConditionConnective::parseCondition(const XmlObject& object)
{
    if (equalsIgnoreCase(object.mName, L"PlayersCondition"))
        return new PlayersCondition(object);
    else if (equalsIgnoreCase(object.mName, L"TeamsCondition"))
        return new TeamsCondition(object);
    else if (equalsIgnoreCase(object.mName, L"RoundTimeCondition"))
        return new RoundTimeCondition(object);
    else if (equalsIgnoreCase(object.mName, L"ShowdownTimeCondition"))
        return new ShowdownTimeCondition(object);
    else if (equalsIgnoreCase(object.mName, L"PostGameTimeCondition"))
        return new PostGameTimeCondition(object);
    else if (equalsIgnoreCase(object.mName, L"LastKillTimeCondition"))
        return new LastKillTimeCondition(object);
    else if (equalsIgnoreCase(object.mName, L"TimeSincePlayersCondition"))
        return new TimeSincePlayersCondition(object);
    else if (equalsIgnoreCase(object.mName, L"PlayersVotedPercent"))
        return new PercentagePlayersVotedCondition(object);
    else if (equalsIgnoreCase(object.mName, L"Conjunction"))
        return new ConditionConnective(object);
    else if (equalsIgnoreCase(object.mName, L"Disjunction"))
        return new ConditionConnective(object);
    else if (equalsIgnoreCase(object.mName, L"PlayersEnteredVolumeCondition"))
        return new PlayersEnteredVolumeCondition(object);
    return NULL;
}

GameCondition* ConditionConnective::FirstConditionWithValue(bool value, MasterGameMode* gameMode) const
{
    for (std::vector<GameCondition*>::const_iterator it = m_conditions.begin(); it != m_conditions.end(); ++it)
    {
        if ((*it)->IsSatisfied(gameMode) == value)
            return *it;
    }
    return NULL;
}

bool ConditionConnective::IsSatisfied(MasterGameMode* gameMode) const
{
    bool satisfied;
    switch (m_type)
    {
    case 0:
        return FirstConditionWithValue(false, gameMode) == NULL;
    case 1:
        satisfied = FirstConditionWithValue(true, gameMode) != NULL;
        break;
    default:
        satisfied = true;
        break;
    }
    return satisfied;
}

unsigned char ConditionConnective::GetMessage() const
{
    return 0;
}

void ConditionConnective::AddIntParams(MasterGameMode* gameMode, std::vector<int>* params) const
{
}

bool ConditionConnective::UpdateMessage(MasterGameMode* gameMode, int message) const
{
    for (std::vector<GameCondition*>::const_iterator it = m_conditions.begin(); it != m_conditions.end(); ++it)
    {
        if ((*it)->UpdateMessage(gameMode, message))
            return true;
    }
    return false;
}

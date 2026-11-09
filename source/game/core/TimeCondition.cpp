#include "game/core/TimeCondition.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/XmlConstruct.hpp"
#include "game/core/XmlObject.hpp"
#include "game/core/XmlValue.hpp"
#include "game/minigame/MiniGameDef.hpp"

TimeCondition::TimeCondition(const XmlObject& object)
    : IntegerCondition(object, object[L"op"].maybeString().fmap(parseOperator).otherwise(EOperator_GreaterThanOrEquals))
{
    m_offset = 0;

    maybe<int> offset = object[L"offset"].maybeInt();
    if (offset.isSet())
        m_offset = offset.get();
}

void TimeCondition::AddIntParams(MasterGameMode* gameMode, std::vector<int>* params) const
{
    int threshold = GetThreshold(gameMode);
    params->push_back(threshold);
    int unit = IDS_MG_SECONDS;
    params->push_back(unit);
}

unsigned int TimeCondition::GetThreshold(MasterGameMode* gameMode) const
{
    int threshold;
    if (m_option >= 0)
    {
        threshold = static_cast<int>(
            CommonMasterGameMode::GetMiniGame()->HostOption_Time(static_cast<eCustomGameModeHostOption>(m_option)));
    }
    else
    {
        threshold = m_threshold;
    }
    return static_cast<unsigned int>(threshold + m_offset);
}

bool TimeCondition::IsSatisfied(MasterGameMode* gameMode) const
{
    int value;
    if (m_option >= 0)
    {
        value = static_cast<int>(
            CommonMasterGameMode::GetMiniGame()->HostOption_Time(static_cast<eCustomGameModeHostOption>(m_option)));
    }
    else
    {
        value = m_threshold;
    }

    if (value == -1)
        return false;
    if (value == -2)
        return true;
    return IntegerCondition::IsSatisfied(gameMode);
}

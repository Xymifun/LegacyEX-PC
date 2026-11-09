#include "game/core/IntegerCondition.hpp"

#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/XmlConstruct.hpp"
#include "game/core/XmlObject.hpp"
#include "game/core/XmlValue.hpp"
#include "game/minigame/MiniGameDef.hpp"

IntegerCondition::IntegerCondition(const XmlObject& object)
{
    _init(object, object[L"op"].maybeString().fmap(parseOperator).get());
}

void IntegerCondition::_init(const XmlObject& object, EOperator op)
{
    m_threshold = object[L"value"].maybeInt().otherwise(0);

    m_option = object[L"value"]
                   .maybeString()
                   .fmap(GameModeOptions::parse)
                   .otherwise(static_cast<eCustomGameModeHostOption>(-1));

    m_operator = op;
}

maybe<EOperator> IntegerCondition::parseOperator(std::wstring name)
{
    if (name == L"LessThan")
        return maybe<EOperator>(EOperator_LessThan);
    if (name == L"LessThanOrEquals")
        return maybe<EOperator>(EOperator_LessThanOrEquals);
    if (name == L"GreaterThan")
        return maybe<EOperator>(EOperator_GreaterThan);
    if (name == L"GreaterThanOrEquals")
        return maybe<EOperator>(EOperator_GreaterThanOrEquals);
    return maybe<EOperator>();
}

bool IntegerCondition::IsSatisfied(MasterGameMode* gameMode) const
{
    int value = GetValue(gameMode);
    int threshold = static_cast<int>(GetThreshold(gameMode));

    switch (m_operator)
    {
    case EOperator_LessThan:
        return value < threshold;
    case EOperator_LessThanOrEquals:
        return value <= threshold;
    case EOperator_GreaterThan:
        return value > threshold;
    case EOperator_GreaterThanOrEquals:
        return value >= threshold;
    }
    return false;
}

unsigned int IntegerCondition::GetThreshold(MasterGameMode* gameMode) const
{
    if (m_option >= 0)
        return CommonMasterGameMode::GetMiniGame()->HostOption_Time(static_cast<eCustomGameModeHostOption>(m_option));
    return static_cast<unsigned int>(m_threshold);
}

unsigned char IntegerCondition::GetMessage() const
{
    static const unsigned char messages[] = {0x82, 0x83, 0x84, 0x85};
    unsigned int op = static_cast<unsigned int>(m_operator);
    if (op > EOperator_GreaterThanOrEquals)
        return 0x82;
    return messages[op];
}

void IntegerCondition::AddIntParams(MasterGameMode* gameMode, std::vector<int>* params) const
{
    params->push_back(GetThreshold(gameMode));
}

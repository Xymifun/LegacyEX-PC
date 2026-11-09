#include "game/core/GameRules_DoubleAttribute.hpp"

#include "game/core/IGameRuleAttribute.hpp"
#include "game/util/StringUtils.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/core/StringFormat.hpp"
#include "game/core/StringConversion.hpp"

#include <algorithm>
#include <string>


GameRules_DoubleAttribute::GameRules_DoubleAttribute(double* value) : mValue(value)
{
    mHasMin = false;
    mHasMax = false;
    mMin = 0.0;
    mMax = 0.0;
}

GameRules_DoubleAttribute::~GameRules_DoubleAttribute()
{
}

void GameRules_DoubleAttribute::setValue(const std::wstring& value)
{
    mHasValue = true;

    *mValue = _fromString<double>(value);

    if (mHasMin)
    {
        *mValue = std::max(mMin, *mValue);
    }

    if (mHasMax)
    {
        *mValue = std::min(mMax, *mValue);
    }
}

void GameRules_DoubleAttribute::save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface)
{
    saveInterface->saveAttribute(attribute, _toString<double>(*mValue));
}

void GameRules_DoubleAttribute::writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml)
{
    xml.append(formatstr(" %ls=\"%.1f\"", gRuleAttributeNames[attribute], *mValue));
}

GameRules_DoubleAttribute* GameRules_DoubleAttribute::setMin(double min)
{
    mMin = min;
    mHasMin = true;
    return this;
}

GameRules_DoubleAttribute* GameRules_DoubleAttribute::setMax(double max)
{
    mMax = max;
    mHasMax = true;
    return this;
}

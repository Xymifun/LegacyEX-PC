#include "game/core/GameRules_IntAttribute.hpp"

#include "game/core/IGameRuleAttribute.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/core/StringFormat.hpp"

#include "game/core/StringConversion.hpp"

#include <algorithm>
#include <string>

GameRules_IntAttribute::GameRules_IntAttribute(int* value) : mValue(value)
{
    mHasMin = false;
    mHasMax = false;
    mMin = 0;
    mMax = 0;
}

GameRules_IntAttribute::~GameRules_IntAttribute()
{
}

void GameRules_IntAttribute::setValue(const std::wstring& value)
{
    mHasValue = true;

    *mValue = _fromString<int>(value);

    if (mHasMin)
    {
        *mValue = std::max(*mValue, mMin);
    }

    if (mHasMax)
    {
        *mValue = std::min(*mValue, mMax);
    }
}

void GameRules_IntAttribute::save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface)
{
    saveInterface->saveAttribute(attribute, _toString<int>(*mValue));
}

void GameRules_IntAttribute::writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml)
{
    xml.append(formatstr(" %ls=\"%d\"", gRuleAttributeNames[attribute], *mValue));
}

GameRules_IntAttribute* GameRules_IntAttribute::setMin(int min)
{
    mMin = min;
    mHasMin = true;
    return this;
}

GameRules_IntAttribute* GameRules_IntAttribute::setMax(int max)
{
    mMax = max;
    mHasMax = true;
    return this;
}

#include "game/core/GameRules_FloatAttribute.hpp"

#include "game/core/IGameRuleAttribute.hpp"
#include "game/util/StringUtils.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/core/StringFormat.hpp"
#include "game/core/StringConversion.hpp"

#include <algorithm>
#include <string>


GameRules_FloatAttribute::GameRules_FloatAttribute(float* value) : mValue(value)
{
    mHasMin = false;
    mHasMax = false;
    mMin = 0.0f;
    mMax = 0.0f;
}

GameRules_FloatAttribute::~GameRules_FloatAttribute()
{
}

void GameRules_FloatAttribute::setValue(const std::wstring& value)
{
    mHasValue = true;

    *mValue = _fromString<float>(value);

    if (mHasMin)
    {
        *mValue = std::max(mMin, *mValue);
    }

    if (mHasMax)
    {
        *mValue = std::min(mMax, *mValue);
    }
}

void GameRules_FloatAttribute::save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface)
{
    saveInterface->saveAttribute(attribute, _toString(*mValue));
}

void GameRules_FloatAttribute::writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml)
{
    xml.append(formatstr(" %ls=\"%.1f\"", gRuleAttributeNames[attribute], *mValue));
}

GameRules_FloatAttribute* GameRules_FloatAttribute::setMin(float min)
{
    mMin = min;
    mHasMin = true;
    return this;
}

GameRules_FloatAttribute* GameRules_FloatAttribute::setMax(float max)
{
    mMax = max;
    mHasMax = true;
    return this;
}

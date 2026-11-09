#include "game/core/GameRules_BoolAttribute.hpp"

#include "game/core/IGameRuleAttribute.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/core/StringFormat.hpp"

#include <string>

GameRules_BoolAttribute::GameRules_BoolAttribute(bool* value) : mValue(value)
{
}

GameRules_BoolAttribute::~GameRules_BoolAttribute()
{
}

void GameRules_BoolAttribute::setValue(const std::wstring& value)
{
    mHasValue = true;

    *mValue = (value.compare(L"true") == 0);
}

void GameRules_BoolAttribute::save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface)
{
    saveInterface->saveAttribute(attribute, *mValue ? L"true" : L"false");
}

void GameRules_BoolAttribute::writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml)
{
    xml.append(formatstr(" %ls=\"%s\"", gRuleAttributeNames[attribute], *mValue ? "true" : "false"));
}

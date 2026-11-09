#include "game/core/GameRules_HexAttribute.hpp"

#include "game/core/IGameRuleAttribute.hpp"
#include "game/core/StringConversion.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/core/StringFormat.hpp"

#include <string>

GameRules_HexAttribute::GameRules_HexAttribute(int* value) : mValue(value)
{
}

GameRules_HexAttribute::~GameRules_HexAttribute()
{
}

void GameRules_HexAttribute::setValue(const std::wstring& value)
{
    mHasValue = true;

    *mValue = _fromHEXString<int>(value);
}

void GameRules_HexAttribute::save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface)
{
    saveInterface->saveAttribute(attribute, _toString<int>(*mValue));
}

void GameRules_HexAttribute::writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml)
{
    xml.append(formatstr(" %ls=\"%x\"", gRuleAttributeNames[attribute], *mValue));
}

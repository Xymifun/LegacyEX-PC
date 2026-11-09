#include "game/core/GameRules_UnsignedCharAttribute.hpp"

#include "game/core/IGameRuleAttribute.hpp"
#include "game/util/StringUtils.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/core/StringFormat.hpp"
#include "game/core/StringConversion.hpp"

#include <string>


GameRules_UnsignedCharAttribute::GameRules_UnsignedCharAttribute(unsigned char* value) : mValue(value)
{
}

GameRules_UnsignedCharAttribute::~GameRules_UnsignedCharAttribute()
{
}

void GameRules_UnsignedCharAttribute::setValue(const std::wstring& value)
{
    mHasValue = true;

    *mValue = _fromString<int>(value);
}

void GameRules_UnsignedCharAttribute::save(ConsoleGameRules::EGameRuleAttr attribute,
                                           GameRuleSaveInterface* saveInterface)
{
    saveInterface->saveAttribute(attribute, _toString<unsigned char>(*mValue));
}

void GameRules_UnsignedCharAttribute::writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml)
{
    xml.append(formatstr(" %ls=\"%d\"", gRuleAttributeNames[attribute], *mValue));
}

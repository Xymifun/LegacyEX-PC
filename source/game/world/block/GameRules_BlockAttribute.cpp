#include "game/world/block/GameRules_BlockAttribute.hpp"

#include "game/core/IGameRuleAttribute.hpp"
#include "game/util/StringUtils.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/world/block/Block.hpp"
#include "game/core/StringFormat.hpp"

#include "game/core/StringConversion.hpp"

#include <string>


GameRules_BlockAttribute::GameRules_BlockAttribute(Block** value) : mValue(value)
{
}

GameRules_BlockAttribute::~GameRules_BlockAttribute()
{
}

void GameRules_BlockAttribute::setValue(const std::wstring& value)
{
    mHasValue = true;

    *mValue = Block::byId(_fromString<int>(value));
}

void GameRules_BlockAttribute::save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface)
{
    saveInterface->saveAttribute(attribute, _toString<Block*>(*mValue));
}

void GameRules_BlockAttribute::writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml)
{
    xml.append(formatstr(" %ls=\"%ls\"", gRuleAttributeNames[attribute], _toString<Block*>(*mValue).c_str()));
}

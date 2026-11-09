#include "game/world/block/GameRules_BlockStateAttribute.hpp"

#include "game/core/IGameRuleAttribute.hpp"
#include "game/util/StringUtils.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/core/StringFormat.hpp"
#include "game/core/StringUtils.hpp"

#include "game/core/StringConversion.hpp"

#include <string>
#include <vector>


GameRules_BlockStateAttribute::GameRules_BlockStateAttribute(const BlockState** value) : mValue(value)
{
}

GameRules_BlockStateAttribute::~GameRules_BlockStateAttribute()
{
}

void GameRules_BlockStateAttribute::setValue(const std::wstring& value)
{
    mHasValue = true;

    if (value == L"Leave")
    {
        *mValue = NULL;
        return;
    }

    if (value.find(L':') != std::wstring::npos)
    {
        std::vector<std::wstring> parts = stringSplit(value, L':');

        Block* block = Block::byString(parts[0]);

        if (block == NULL)
        {
            block = Block::byId(_fromString<int>(parts[0]));
        }

        *mValue = block->getBlockState(_fromString<int>(parts[1]));
    }
    else
    {
        Block* block = Block::byString(value);

        if (block == NULL)
        {
            block = Block::byId(_fromString<int>(value));
        }

        *mValue = block->defaultBlockState();
    }
}

void GameRules_BlockStateAttribute::save(ConsoleGameRules::EGameRuleAttr attribute,
                                         GameRuleSaveInterface* saveInterface)
{
    saveInterface->saveAttribute(attribute, _toString<const BlockState*>(*mValue));
}

void GameRules_BlockStateAttribute::writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml)
{
    if (*mValue == NULL)
    {
        xml.append(formatstr(" %ls=\"Leave\"", gRuleAttributeNames[attribute]));
    }
    else
    {
        Block* block = (*mValue)->getBlock();

        int id = Block::getId(block);
        int aux = block->getSpawnResourcesAuxValue(*mValue);
        xml.append(formatstr(" %ls=\"%i:%i\"", gRuleAttributeNames[attribute], id, aux));
    }
}

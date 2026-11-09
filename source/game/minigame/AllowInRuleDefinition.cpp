#include "game/minigame/AllowInRuleDefinition.hpp"

#include <sstream>

#include "game/data/GameRuleSaveInterface.hpp"
#include "game/minigame/GameRuleDefinition.hpp"

std::wstring join(wchar_t separator, const std::vector<std::wstring>& values)
{
    std::wstring result;

    for (std::vector<std::wstring>::const_iterator it = values.begin(); it != values.end(); ++it)
    {
        if (it + 1 == values.end())
        {
            std::wstring value = *it;
            value += separator;
            result += value;
        }
        else
        {
            result += *it;
        }
    }

    return result;
}

void split(wchar_t separator, const std::wstring& value, std::vector<std::wstring>& values)
{
    std::wstringstream stream(value);
    values.clear();

    std::wstring part;
    while (std::getline(stream, part, L';'))
    {
        values.push_back(part);
    }
}

AllowInRuleDefinition::AllowInRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType AllowInRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_GrantPermissions;
}

void AllowInRuleDefinition::writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 2);

    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_target, join(L';', mTargetLayers));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_type, join(L';', mPermissions));
}

void AllowInRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(L"target") == 0)
    {
        split(L';', value, mTargetLayers);
        return;
    }
    if (name.compare(L"type") == 0)
    {
        split(L';', value, mPermissions);
        return;
    }
    GameRuleDefinition::addAttribute(name, value);
}

const std::vector<std::wstring>& AllowInRuleDefinition::getTargetLayers() const
{
    return mTargetLayers;
}

const std::vector<std::wstring>& AllowInRuleDefinition::getPermissions() const
{
    return mPermissions;
}

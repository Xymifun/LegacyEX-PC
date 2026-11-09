#include "game/data/DosGameRuleSaveInterface.hpp"

#include "game/core/DataOutputStream.hpp"

static const int sGameRuleTypeCount = 77;

int DosGameRuleSaveInterface::getKeyFor(int tagId) const
{
    return mTagKeys->find(tagId)->second;
}

int DosGameRuleSaveInterface::getKeyFor(ConsoleGameRules::EGameRuleAttr attribute) const
{
    return getKeyFor(sGameRuleTypeCount + static_cast<int>(attribute));
}

int DosGameRuleSaveInterface::getKeyFor(ConsoleGameRules::EGameRuleType type) const
{
    return getKeyFor(static_cast<int>(type));
}

DosGameRuleSaveInterface::DosGameRuleSaveInterface(DataOutputStream* output, boost::unordered_map<int, int>* tagKeys)
    : mOutput(output),
      mTagKeys(tagKeys)
{
}

void DosGameRuleSaveInterface::startTag(ConsoleGameRules::EGameRuleType type)
{
    GameRuleSaveInterface::startTag(type);

    mOutput->writeInt(getKeyFor(type));
}

void DosGameRuleSaveInterface::startAttributes(int attributeCount)
{
    GameRuleSaveInterface::startAttributes(attributeCount);

    mOutput->writeInt(attributeCount);
}

void DosGameRuleSaveInterface::saveAttribute(ConsoleGameRules::EGameRuleAttr attribute, const std::wstring& value)
{
    GameRuleSaveInterface::saveAttribute(attribute, value);

    mOutput->writeInt(getKeyFor(attribute));
    mOutput->writeUTF(value);
}

void DosGameRuleSaveInterface::startChildren(int childCount)
{
    GameRuleSaveInterface::startChildren(childCount);

    mOutput->writeInt(childCount);
}

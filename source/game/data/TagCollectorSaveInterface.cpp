#include "game/data/TagCollectorSaveInterface.hpp"

TagCollectorSaveInterface::TagCollectorSaveInterface(boost::unordered_set<int>* tagTypes,
                                                     boost::unordered_set<int>* attributes)
    : mTagTypes(tagTypes),
      mAttributes(attributes)
{
}

void TagCollectorSaveInterface::startTag(ConsoleGameRules::EGameRuleType type)
{
    GameRuleSaveInterface::startTag(type);

    mTagTypes->insert(type);
}

void TagCollectorSaveInterface::saveAttribute(ConsoleGameRules::EGameRuleAttr attribute, const std::wstring& value)
{
    GameRuleSaveInterface::saveAttribute(attribute, value);

    mAttributes->insert(attribute);
}

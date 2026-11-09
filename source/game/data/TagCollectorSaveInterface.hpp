#ifndef GAME_DATA_TAGCOLLECTORSAVEINTERFACE_HPP
#define GAME_DATA_TAGCOLLECTORSAVEINTERFACE_HPP

#include <string>

#include <boost/unordered_set.hpp>

#include "game/data/GameRuleSaveInterface.hpp"

class TagCollectorSaveInterface : public GameRuleSaveInterface
{
public:
    TagCollectorSaveInterface(boost::unordered_set<int>* tagTypes, boost::unordered_set<int>* attributes);

    virtual void startTag(ConsoleGameRules::EGameRuleType type);
    virtual void saveAttribute(ConsoleGameRules::EGameRuleAttr attribute, const std::wstring& value);

private:
    boost::unordered_set<int>* mTagTypes;
    boost::unordered_set<int>* mAttributes;
};

#endif

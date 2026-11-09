#ifndef GAME_DATA_DOSGAMERULESAVEINTERFACE_HPP
#define GAME_DATA_DOSGAMERULESAVEINTERFACE_HPP

#include <string>

#include <boost/unordered_map.hpp>

#include "game/data/GameRuleSaveInterface.hpp"

class DataOutputStream;

class DosGameRuleSaveInterface : public GameRuleSaveInterface
{
public:
    DosGameRuleSaveInterface(DataOutputStream* output, boost::unordered_map<int, int>* tagKeys);

    virtual void startTag(ConsoleGameRules::EGameRuleType type);
    virtual void startAttributes(int attributeCount);
    virtual void saveAttribute(ConsoleGameRules::EGameRuleAttr attribute, const std::wstring& value);
    virtual void startChildren(int childCount);

private:
    int getKeyFor(int tagId) const;
    int getKeyFor(ConsoleGameRules::EGameRuleAttr attribute) const;
    int getKeyFor(ConsoleGameRules::EGameRuleType type) const;

    DataOutputStream* mOutput;
    boost::unordered_map<int, int>* mTagKeys;
};

#endif

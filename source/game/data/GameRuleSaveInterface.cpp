#include "game/data/GameRuleSaveInterface.hpp"

void GameRuleSaveInterface::startTag(ConsoleGameRules::EGameRuleType type)
{
    Partials partial = {mPartials.size(), type, 0, 0, 0, 0, false, false};
    mPartials.push_back(partial);
}

void GameRuleSaveInterface::startAttributes(int attributeCount)
{
    mPartials.back().mAttributesStarted = true;
    mPartials.back().mAttributeCount = attributeCount;
}

void GameRuleSaveInterface::saveAttribute(ConsoleGameRules::EGameRuleAttr attribute, const std::wstring& value)
{
    mPartials.back().mAttributesSaved++;
}

void GameRuleSaveInterface::startChildren(int childCount)
{
    mPartials.back().mChildrenStarted = true;
    mPartials.back().mChildCount = childCount;
}

void GameRuleSaveInterface::finishTag()
{
    mPartials.pop_back();

    if (!mPartials.empty())
    {
        mPartials.back().mChildrenFinished++;
    }
}

#include "game/item/RandomItemSetRuleDefinition.hpp"

#include "game/core/WeighedTreasure.hpp"
#include "game/item/WeighedTreasureItemRuleDefinition.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelRuleset.hpp"

RandomItemSetRuleDefinition::RandomItemSetRuleDefinition()
{
    mId = L"";
    mMaxCount = 0;
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_id, &mId);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_maxCount, &mMaxCount);
}

RandomItemSetRuleDefinition::~RandomItemSetRuleDefinition()
{
    for (std::vector<WeighedTreasureItemRuleDefinition*>::iterator it = mItems.begin(); it != mItems.end(); ++it)
    {
        delete *it;
    }
}

std::wstring RandomItemSetRuleDefinition::getId()
{
    return mId;
}

void RandomItemSetRuleDefinition::populateContainer(boost::shared_ptr<Container> container, LevelRuleset* ruleset)
{
    if (mItems.size() > 0)
    {
        Random random;
        arrayWithLength<WeighedTreasure*> treasures(static_cast<unsigned int>(mItems.size()), true);
        for (unsigned int i = 0; i < treasures.length; ++i)
        {
            treasures.data[i] = mItems[i]->getItem();
        }

        WeighedTreasure::addChestItems(&random, treasures, container, mMaxCount, false);
        for (unsigned int i = 0; i < treasures.length; ++i)
        {
            delete treasures.data[i];
        }
        delete[] treasures.data;
    }
    else if (ruleset != NULL)
    {
        RandomItemSetRuleDefinition* itemSet = ruleset->getRandomItemSet(mId);
        if (itemSet != NULL)
        {
            itemSet->populateContainer(container, ruleset);
        }
    }
}

void RandomItemSetRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    for (std::vector<WeighedTreasureItemRuleDefinition*>::iterator it = mItems.begin(); it != mItems.end(); ++it)
    {
        children->push_back(*it);
    }
}

ConsoleGameRules::EGameRuleType RandomItemSetRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_RandomItemSet;
}

GameRuleDefinition* RandomItemSetRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    GameRuleDefinition* rule = NULL;
    if (type == ConsoleGameRules::eGameRuleType_WeighedTreasureItem)
    {
        rule = new WeighedTreasureItemRuleDefinition();
        mItems.push_back(static_cast<WeighedTreasureItemRuleDefinition*>(rule));
    }
    return rule;
}

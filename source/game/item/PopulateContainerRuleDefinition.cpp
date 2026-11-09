#include "game/item/PopulateContainerRuleDefinition.hpp"

#include <boost/pointer_cast.hpp>

#include "game/item/Container.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"

PopulateContainerRuleDefinition::PopulateContainerRuleDefinition() : GameRuleDefinition()
{
    mX = 0;
    mY = 0;
    mZ = 0;
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_x, &mX);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_y, &mY);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_z, &mZ);
}

PopulateContainerRuleDefinition::~PopulateContainerRuleDefinition()
{
    for (std::vector<AddItemRuleDefinition*>::iterator it = mAddItems.begin(); it != mAddItems.end(); ++it)
    {
        delete *it;
    }
    for (std::vector<RandomItemSetRuleDefinition*>::iterator it = mRandomItemSets.begin(); it != mRandomItemSets.end();
         ++it)
    {
        delete *it;
    }
}

void PopulateContainerRuleDefinition::GetPos(int& x, int& y, int& z)
{
    x = mX;
    y = mY;
    z = mZ;
}

void PopulateContainerRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    for (std::vector<AddItemRuleDefinition*>::iterator it = mAddItems.begin(); it != mAddItems.end(); ++it)
    {
        children->push_back(*it);
    }
}

GameRuleDefinition* PopulateContainerRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    GameRuleDefinition* child = NULL;
    if (type == ConsoleGameRules::eGameRuleType_AddItem)
    {
        child = new AddItemRuleDefinition();
        mAddItems.push_back(static_cast<AddItemRuleDefinition*>(child));
    }
    else if (type == ConsoleGameRules::eGameRuleType_RandomItemSet)
    {
        child = new RandomItemSetRuleDefinition();
        mRandomItemSets.push_back(static_cast<RandomItemSetRuleDefinition*>(child));
    }
    return child;
}

bool PopulateContainerRuleDefinition::evaluateRule(Level* level, LevelRuleset* ruleset, const MiniGameDef& miniGameDef)
{
    (void)miniGameDef;
    boost::shared_ptr<Container> container = boost::dynamic_pointer_cast<Container>(
        level->getBlockEntity(BlockPos(mX, mY, mZ)));
    if (container)
    {
        container->clearContent();
        for (std::vector<RandomItemSetRuleDefinition*>::iterator it = mRandomItemSets.begin();
             it != mRandomItemSets.end(); ++it)
        {
            (*it)->populateContainer(container, ruleset);
        }
        int slot = 0;
        for (std::vector<AddItemRuleDefinition*>::iterator it = mAddItems.begin();
             it != mAddItems.end() && slot < container->getContainerSize(); ++it, ++slot)
        {
            AddItemRuleDefinition* addItem = *it;
            addItem->addItemToContainer(container, slot);
        }
    }
    return true;
}

ConsoleGameRules::EGameRuleType PopulateContainerRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_PopulateContainer;
}

#include "game/item/PlaceContainerRuleDefinition.hpp"

#include <boost/pointer_cast.hpp>

#include "game/item/Container.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/core/BoundingBox.hpp"
#include "game/util/Random.hpp"
#include "game/world/generation/StructurePiece.hpp"

PlaceContainerRuleDefinition::PlaceContainerRuleDefinition() : XboxStructureActionPlaceBlock()
{
    mBlock = Blocks::chest;
    mChance = 1.0f;
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_facing, &mAuxValue);
    addFloatAttribute(ConsoleGameRules::eGameRuleAttr_probability, &mChance);
}

PlaceContainerRuleDefinition::~PlaceContainerRuleDefinition()
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

bool PlaceContainerRuleDefinition::placeContainerInLevel(Level* level, const BlockPos& pos, LevelRuleset* ruleset)
{
    if (level->getBlockEntity(pos))
    {
        level->removeBlockEntity(pos);
        level->setBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL, false);
    }

    Random random;
    if (random.nextFloat() < mChance)
    {
        level->setBlock(pos, mBlock->getBlockState(mAuxValue), Block::UPDATE_ALL, false);
        boost::shared_ptr<Container> container = boost::dynamic_pointer_cast<Container>(level->getBlockEntity(pos));
        if (container)
        {
            for (std::vector<RandomItemSetRuleDefinition*>::iterator it = mRandomItemSets.begin();
                 it != mRandomItemSets.end(); ++it)
            {
                (*it)->populateContainer(container, ruleset);
            }

            unsigned int slot = 0;
            for (std::vector<AddItemRuleDefinition*>::iterator it = mAddItems.begin();
                 it != mAddItems.end() && slot < container->getContainerSize(); ++it, ++slot)
            {
                AddItemRuleDefinition* addItem = *it;
                addItem->addItemToContainer(container, static_cast<int>(slot));
            }
        }
        return true;
    }
    return false;
}

bool PlaceContainerRuleDefinition::placeContainerInChunk(StructurePiece* structurePiece, Level* level, BoundingBox* box)
{
    BlockPos pos(structurePiece->getWorldX(mEndX, mEndZ), structurePiece->getWorldY(mEndY),
                 structurePiece->getWorldZ(mEndX, mEndZ));
    if (box->isInside(pos))
    {
        return placeContainerInLevel(level, pos, NULL);
    }
    return false;
}

void PlaceContainerRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    for (std::vector<AddItemRuleDefinition*>::iterator it = mAddItems.begin(); it != mAddItems.end(); ++it)
    {
        children->push_back(*it);
    }
}

GameRuleDefinition* PlaceContainerRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
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

bool PlaceContainerRuleDefinition::evaluateRule(Level* level, LevelRuleset* ruleset, const MiniGameDef& miniGameDef)
{
    (void)miniGameDef;
    return placeContainerInLevel(level, BlockPos(mEndX, mEndY, mEndZ), ruleset);
}

ConsoleGameRules::EGameRuleType PlaceContainerRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_PlaceContainer;
}

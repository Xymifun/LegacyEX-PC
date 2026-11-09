#include "game/minigame/CompoundGameRuleDefinition.hpp"

#include <string>

#include "game/core/GameRule.hpp"
#include "game/item/CollectItemRuleDefinition.hpp"
#include "game/item/DistributeItemsRuleDefinition.hpp"
#include "game/item/PlaceContainerRuleDefinition.hpp"
#include "game/item/PopulateContainerRuleDefinition.hpp"
#include "game/minigame/CompleteAllRuleDefinition.hpp"
#include "game/world/block/UseBlockRuleDefinition.hpp"
#include "game/world/generation/UpdatePlayerRuleDefinition.hpp"

template <typename T> std::wstring _toString(T value);

CompoundGameRuleDefinition::CompoundGameRuleDefinition()
{
    mCompletedChild = NULL;
}

CompoundGameRuleDefinition::~CompoundGameRuleDefinition()
{
    for (std::vector<GameRuleDefinition*>::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        delete *it;
    }
}

void CompoundGameRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    for (std::vector<GameRuleDefinition*>::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        children->push_back(*it);
    }
}

void CompoundGameRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children,
                                             ConsoleGameRules::EGameRuleType type)
{
    for (std::vector<GameRuleDefinition*>::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        if ((*it)->getActionType() == type)
        {
            children->push_back(*it);
        }
    }
}

GameRuleDefinition* CompoundGameRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    GameRuleDefinition* pChild = NULL;

    if (type == ConsoleGameRules::eGameRuleType_CompleteAll)
    {
        pChild = new CompleteAllRuleDefinition();
    }
    else
    {
        if (type == ConsoleGameRules::eGameRuleType_CollectItem)
        {
            pChild = new CollectItemRuleDefinition();
        }
        else if (type == ConsoleGameRules::eGameRuleType_UseBlock)
        {
            pChild = new UseBlockRuleDefinition();
        }
        else if (type == ConsoleGameRules::eGameRuleType_UpdatePlayer)
        {
            pChild = new UpdatePlayerRuleDefinition();
        }
        else if (type == ConsoleGameRules::eGameRuleType_PlaceContainer)
        {
            pChild = new PlaceContainerRuleDefinition();
        }
        else if (type == ConsoleGameRules::eGameRuleType_DistributeItems)
        {
            pChild = new DistributeItemsRuleDefinition();
        }
        else if (type == ConsoleGameRules::eGameRuleType_PopulateContainer)
        {
            pChild = new PopulateContainerRuleDefinition();
        }
    }

    if (pChild != NULL)
    {
        mChildren.push_back(pChild);
    }

    return pChild;
}

void CompoundGameRuleDefinition::populateGameRule(GameRulesInstance::EGameRulesInstanceType type, GameRule* gameRule)
{
    int index = 0;

    for (std::vector<GameRuleDefinition*>::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        GameRule* pChildRule = new GameRule(*it, gameRule->mConnection);

        (*it)->populateGameRule(type, pChildRule);

        GameRule::_ValueType value;
        value.pGameRule = pChildRule;
        value.bIsGameRule = true;

        gameRule->setParameter(L"rule" + _toString(index), value);

        ++index;
    }

    GameRuleDefinition::populateGameRule(type, gameRule);
}

bool CompoundGameRuleDefinition::onUseBlock(GameRule* gameRule, int blockId, int x, int y, int z)
{
    bool bTriggered = false;

    for (GameRule::ParameterMap::iterator it = gameRule->mParameters.begin(); it != gameRule->mParameters.end(); ++it)
    {
        if (it->second.bIsGameRule)
        {
            bool childTriggered = it->second.pGameRule->getGameRuleDefinition()->onUseBlock(it->second.pGameRule,
                                                                                            blockId, x, y, z);
            if (!bTriggered && childTriggered)
            {
                mCompletedChild = it->second.pGameRule->getGameRuleDefinition();
                bTriggered = true;
            }
        }
    }

    return bTriggered;
}

bool CompoundGameRuleDefinition::onCollectItem(GameRule* gameRule, not_null_ptr<ItemInstance> item)
{
    bool bTriggered = false;

    for (GameRule::ParameterMap::iterator it = gameRule->mParameters.begin(); it != gameRule->mParameters.end(); ++it)
    {
        if (it->second.bIsGameRule)
        {
            bool childTriggered = it->second.pGameRule->getGameRuleDefinition()->onCollectItem(it->second.pGameRule,
                                                                                               item);
            if (!bTriggered && childTriggered)
            {
                mCompletedChild = it->second.pGameRule->getGameRuleDefinition();
                bTriggered = true;
            }
        }
    }

    return bTriggered;
}

void CompoundGameRuleDefinition::postProcessPlayer(boost::shared_ptr<Player> player)
{
    for (std::vector<GameRuleDefinition*>::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        (*it)->postProcessPlayer(player);
    }
}

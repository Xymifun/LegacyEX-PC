#include "game/item/CollectItemRuleDefinition.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/GameRule.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/network/Connection.hpp"
#include "game/network/UpdateGameRuleProgressPacket.hpp"

CollectItemRuleDefinition::CollectItemRuleDefinition() : GameRuleDefinition(), mIcon(0), mAuxValue(0), mGoal(0)
{
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_itemId, &mIcon);
    addUnsignedCharAttribute(ConsoleGameRules::eGameRuleAttr_auxValue, &mAuxValue);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_quantity, &mGoal);
}

CollectItemRuleDefinition::~CollectItemRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType CollectItemRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_CollectItem;
}

int CollectItemRuleDefinition::getGoal()
{
    return mGoal;
}

int CollectItemRuleDefinition::getProgress(GameRule* gameRule)
{
    GameRule::_ValueType value = gameRule->getParameter(L"iQuantity");
    return value.iValue;
}

int CollectItemRuleDefinition::getIcon()
{
    return mIcon;
}

int CollectItemRuleDefinition::getAuxValue()
{
    return mAuxValue;
}

void CollectItemRuleDefinition::populateGameRule(GameRulesInstance::EGameRulesInstanceType type, GameRule* gameRule)
{
    GameRule::_ValueType value;
    value.iValue = 0;
    gameRule->setParameter(L"iQuantity", value);
    GameRuleDefinition::populateGameRule(type, gameRule);
}

bool CollectItemRuleDefinition::onCollectItem(GameRule* gameRule, not_null_ptr<ItemInstance> item)
{
    bool collected = false;
    if (Item::getId(item->getItem()) == mIcon && item->getAuxValue() == mAuxValue && item->get4JData() == mDataTag
        && !getComplete(gameRule))
    {
        GameRule::_ValueType value = gameRule->getParameter(L"iQuantity");
        value.iValue += item->getCount();
        gameRule->setParameter(L"iQuantity", value);
        collected = true;

        if (mGoal <= value.iValue)
        {
            setComplete(gameRule, true);
            if (gameRule->mConnection != 0)
            {
                boost::shared_ptr<Packet> packet(new UpdateGameRuleProgressPacket(getActionType(), mDescriptionName,
                                                                                  mIcon, mAuxValue, mDataTag, 0, 0));
                gameRule->mConnection->send(packet);
            }
        }
    }

    return collected;
}

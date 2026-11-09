#include "game/minigame/PowerupRuleDefinition.hpp"

#include "game/item/AddItemRuleDefinition.hpp"
#include "game/item/Inventory.hpp"
#include "game/world/generation/Player.hpp"

PowerupRuleDefinition::PowerupRuleDefinition()
{
}

PowerupRuleDefinition::~PowerupRuleDefinition()
{
}

void PowerupRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    for (std::vector<AddItemRuleDefinition*>::iterator it = mPowerups.begin(); it != mPowerups.end(); ++it)
    {
        children->push_back(*it);
    }
}

GameRuleDefinition* PowerupRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    GameRuleDefinition* rule = NULL;
    if (type == ConsoleGameRules::eGameRuleType_AddItem)
    {
        rule = new AddItemRuleDefinition();
        mPowerups.push_back(static_cast<AddItemRuleDefinition*>(rule));
    }
    return rule;
}

void PowerupRuleDefinition::GiveRandomPowerup(boost::shared_ptr<Player> player)
{
    int powerupCount = static_cast<int>(mPowerups.size());
    mPowerups.at(static_cast<unsigned int>(mRandom.nextInt(powerupCount)))->addItemToContainer(player->mInventory, -1);
}

ConsoleGameRules::EGameRuleType PowerupRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_Powerup;
}

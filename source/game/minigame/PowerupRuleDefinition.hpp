#ifndef GAME_MINIGAME_POWERUPRULEDEFINITION_HPP
#define GAME_MINIGAME_POWERUPRULEDEFINITION_HPP

#include <boost/shared_ptr.hpp>
#include <vector>

#include "game/minigame/NamedAreaRuleDefinition.hpp"
#include "game/util/Random.hpp"

class AddItemRuleDefinition;
class Player;

class PowerupRuleDefinition : public NamedAreaRuleDefinition
{
public:
    PowerupRuleDefinition();
    virtual ~PowerupRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);

    void GiveRandomPowerup(boost::shared_ptr<Player> player);

private:
    std::vector<AddItemRuleDefinition*> mPowerups;
    Random mRandom;
};

#endif

#ifndef GAME_MINIGAME_COMPOUNDGAMERULEDEFINITION_HPP
#define GAME_MINIGAME_COMPOUNDGAMERULEDEFINITION_HPP

#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"
#include "game/minigame/GameRuleDefinition.hpp"

class GameRule;
class ItemInstance;
class Player;

class CompoundGameRuleDefinition : public GameRuleDefinition
{
public:
    CompoundGameRuleDefinition();

    virtual ~CompoundGameRuleDefinition();

    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual void getChildren(std::vector<GameRuleDefinition*>* children, ConsoleGameRules::EGameRuleType type);
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);
    virtual void populateGameRule(GameRulesInstance::EGameRulesInstanceType type, GameRule* gameRule);
    virtual bool onUseBlock(GameRule* gameRule, int blockId, int x, int y, int z);
    virtual bool onCollectItem(GameRule* gameRule, not_null_ptr<ItemInstance> item);
    virtual void postProcessPlayer(boost::shared_ptr<Player> player);

protected:
    std::vector<GameRuleDefinition*> mChildren;
    GameRuleDefinition* mCompletedChild;
};

#endif

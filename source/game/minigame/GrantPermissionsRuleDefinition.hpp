#ifndef GAME_MINIGAME_GRANTPERMISSIONSRULEDEFINITION_HPP
#define GAME_MINIGAME_GRANTPERMISSIONSRULEDEFINITION_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/minigame/GameRuleDefinition.hpp"

class AllowInRuleDefinition;
class BlockPos;
class Level;
class LevelRuleset;
class Player;

class GrantPermissionsRuleDefinition : public GameRuleDefinition
{
public:
    GrantPermissionsRuleDefinition();
    virtual ~GrantPermissionsRuleDefinition();
    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);
    std::vector<const AllowInRuleDefinition*>* getLayers(const std::wstring& name);
    void sortChild(const AllowInRuleDefinition* child);
    bool anyRulePermitsAction(const std::vector<const AllowInRuleDefinition*>& rules,
                              const boost::shared_ptr<Player>& player, Level* level, LevelRuleset* ruleset,
                              const BlockPos& pos);
    bool canDestroy(const boost::shared_ptr<Player>& player, Level* level, LevelRuleset* ruleset, const BlockPos& pos);
    bool canPlace(const boost::shared_ptr<Player>& player, Level* level, LevelRuleset* ruleset, const BlockPos& pos);
    bool canUse(const boost::shared_ptr<Player>& player, Level* level, LevelRuleset* ruleset, const BlockPos& pos);

private:
    std::vector<AllowInRuleDefinition*> mAllowInRules;
    std::vector<const AllowInRuleDefinition*> mUnsortedRules;
    std::vector<const AllowInRuleDefinition*> mDestroyRules;
    std::vector<const AllowInRuleDefinition*> mPlaceRules;
    std::vector<const AllowInRuleDefinition*> mUseRules;
};

#endif

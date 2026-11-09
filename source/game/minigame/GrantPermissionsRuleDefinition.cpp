#include "game/minigame/GrantPermissionsRuleDefinition.hpp"

#include "game/minigame/AllowInRuleDefinition.hpp"
#include "game/minigame/NamedAreaRuleDefinition.hpp"
#include "game/core/Team.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"

GrantPermissionsRuleDefinition::GrantPermissionsRuleDefinition()
{
}

GrantPermissionsRuleDefinition::~GrantPermissionsRuleDefinition()
{
    for (std::vector<AllowInRuleDefinition*>::iterator it = mAllowInRules.begin(); it != mAllowInRules.end(); ++it)
    {
        delete *it;
    }
}

void GrantPermissionsRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    children->insert(children->end(), mAllowInRules.begin(), mAllowInRules.end());
}

GameRuleDefinition* GrantPermissionsRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    if (type == ConsoleGameRules::eGameRuleType_AllowIn)
    {
        AllowInRuleDefinition* child = new AllowInRuleDefinition();
        mAllowInRules.push_back(child);
        mUnsortedRules.push_back(child);
        return child;
    }
    return NULL;
}

std::vector<const AllowInRuleDefinition*>* GrantPermissionsRuleDefinition::getLayers(const std::wstring& name)
{
    if (name.compare(L"Destroy") == 0)
    {
        return &mDestroyRules;
    }
    if (name.compare(L"Place") == 0)
    {
        return &mPlaceRules;
    }
    if (name.compare(L"Use") == 0)
    {
        return &mUseRules;
    }
    return NULL;
}

void GrantPermissionsRuleDefinition::sortChild(const AllowInRuleDefinition* child)
{
    const std::vector<std::wstring>& permissions = child->getPermissions();
    for (std::vector<std::wstring>::const_iterator it = permissions.begin(); it != permissions.end(); ++it)
    {
        getLayers(*it)->push_back(child);
    }
}

bool GrantPermissionsRuleDefinition::anyRulePermitsAction(const std::vector<const AllowInRuleDefinition*>& rules,
                                                          const boost::shared_ptr<Player>& player, Level* level,
                                                          LevelRuleset* ruleset, const BlockPos& pos)
{
    while (mUnsortedRules.size() > 0)
    {
        sortChild(mUnsortedRules.back());
        mUnsortedRules.pop_back();
    }

    int teamMask = player->getTeam() == NULL ? 0 : 1 << player->getTeam()->getTeamIndex();

    for (std::vector<const AllowInRuleDefinition*>::const_iterator it = rules.begin(); it != rules.end(); ++it)
    {
        const AllowInRuleDefinition* rule = *it;
        if (rule->mDataTag == 0 || (rule->mDataTag & teamMask) > 0)
        {
            const std::vector<std::wstring>& targetLayers = rule->getTargetLayers();
            for (std::vector<std::wstring>::const_iterator layer = targetLayers.begin(); layer != targetLayers.end();
                 ++layer)
            {
                NamedAreaRuleDefinition* area = ruleset->getNamedAreaRule(*layer);
                if (area != NULL && area->containsBlock(Block::getId(level->getBlock(pos)), pos))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool GrantPermissionsRuleDefinition::canDestroy(const boost::shared_ptr<Player>& player, Level* level,
                                                LevelRuleset* ruleset, const BlockPos& pos)
{
    return anyRulePermitsAction(mDestroyRules, player, level, ruleset, pos);
}

bool GrantPermissionsRuleDefinition::canPlace(const boost::shared_ptr<Player>& player, Level* level,
                                              LevelRuleset* ruleset, const BlockPos& pos)
{
    return anyRulePermitsAction(mPlaceRules, player, level, ruleset, pos);
}

bool GrantPermissionsRuleDefinition::canUse(const boost::shared_ptr<Player>& player, Level* level,
                                            LevelRuleset* ruleset, const BlockPos& pos)
{
    return anyRulePermitsAction(mUseRules, player, level, ruleset, pos);
}

ConsoleGameRules::EGameRuleType GrantPermissionsRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_GrantPermissions;
}

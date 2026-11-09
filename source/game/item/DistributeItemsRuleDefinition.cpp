#include "game/item/DistributeItemsRuleDefinition.hpp"

#include <boost/pointer_cast.hpp>

#include "game/item/Container.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/WorldPositionRuleDefinition.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"

class DistributeItemsWorldPositionAccess : public WorldPositionRuleDefinition
{
public:
    static BlockPos getPosition(const WorldPositionRuleDefinition* position)
    {
        const DistributeItemsWorldPositionAccess* access = static_cast<const DistributeItemsWorldPositionAccess*>(
            position);
        return BlockPos(access->mX, access->mY, access->mZ);
    }
};

DistributeItemsRuleDefinition::DistributeItemsRuleDefinition() : GameRuleDefinition()
{
    mLootGroup.assign(L"");
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_id, &mLootGroup);
}

DistributeItemsRuleDefinition::~DistributeItemsRuleDefinition()
{
    for (std::vector<WorldPositionRuleDefinition*>::iterator it = mPositions.begin(); it != mPositions.end(); ++it)
    {
        delete *it;
    }
}

void DistributeItemsRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    for (std::vector<WorldPositionRuleDefinition*>::iterator it = mPositions.begin(); it != mPositions.end(); ++it)
    {
        children->push_back(*it);
    }
}

ConsoleGameRules::EGameRuleType DistributeItemsRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_DistributeItems;
}

GameRuleDefinition* DistributeItemsRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    GameRuleDefinition* rule = NULL;
    if (type == ConsoleGameRules::eGameRuleType_WorldPosition)
    {
        rule = new WorldPositionRuleDefinition();
        mPositions.push_back(static_cast<WorldPositionRuleDefinition*>(rule));
    }
    return rule;
}

std::vector<boost::shared_ptr<Container> > DistributeItemsRuleDefinition::getContainers(
    Level* level, const std::vector<WorldPositionRuleDefinition*>& positions)
{
    std::vector<boost::shared_ptr<Container> > containers;

    for (std::vector<WorldPositionRuleDefinition*>::const_iterator it = positions.begin(); it != positions.end(); ++it)
    {
        BlockPos position = DistributeItemsWorldPositionAccess::getPosition(*it);
        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(position);
        boost::shared_ptr<Container> container = boost::dynamic_pointer_cast<Container>(blockEntity);
        if (container)
        {
            containers.push_back(container);
        }
    }

    return containers;
}

bool DistributeItemsRuleDefinition::evaluateRule(Level* level, LevelRuleset* ruleset, const MiniGameDef& miniGameDef)
{
    (void)ruleset;
    std::vector<boost::shared_ptr<Container> > containers = getContainers(level, mPositions);
    return miniGameDef.PopulateContainers(&containers, mLootGroup);
}

std::wstring DistributeItemsRuleDefinition::getId() const
{
    return mLootGroup;
}

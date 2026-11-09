#ifndef GAME_ITEM_PLACECONTAINERRULEDEFINITION_HPP
#define GAME_ITEM_PLACECONTAINERRULEDEFINITION_HPP

#include <stdint.h>
#include <vector>

#include "game/item/AddItemRuleDefinition.hpp"
#include "game/item/RandomItemSetRuleDefinition.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/XboxStructureActionPlaceBlock.hpp"

class BoundingBox;
class Level;
class LevelRuleset;
class MiniGameDef;
class StructurePiece;

class PlaceContainerRuleDefinition : public XboxStructureActionPlaceBlock
{
public:
    PlaceContainerRuleDefinition();
    virtual ~PlaceContainerRuleDefinition();

    bool placeContainerInLevel(Level* level, const BlockPos& pos, LevelRuleset* ruleset);
    bool placeContainerInChunk(StructurePiece* structurePiece, Level* level, BoundingBox* box);
    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);
    virtual bool evaluateRule(Level* level, LevelRuleset* ruleset, const MiniGameDef& miniGameDef);
    virtual ConsoleGameRules::EGameRuleType getActionType();

private:
    std::vector<AddItemRuleDefinition*> mAddItems;
    std::vector<RandomItemSetRuleDefinition*> mRandomItemSets;
    float mChance;
};

#endif

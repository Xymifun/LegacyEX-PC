#ifndef GAME_WORLD_BLOCK_XBOXSTRUCTUREACTIONPLACEBLOCK_HPP
#define GAME_WORLD_BLOCK_XBOXSTRUCTUREACTIONPLACEBLOCK_HPP

#include <stdint.h>

#include "game/world/generation/ConsoleGenerateStructureAction.hpp"

class Block;
class BoundingBox;
class Level;
class StructurePiece;

class XboxStructureActionPlaceBlock : public ConsoleGenerateStructureAction
{
public:
    XboxStructureActionPlaceBlock();
    virtual ~XboxStructureActionPlaceBlock() {}

    uint32_t placeBlockInLevel(StructurePiece* structurePiece, Level* level, BoundingBox* box);
    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual uint32_t getEndX();
    virtual uint32_t getEndY();
    virtual uint32_t getEndZ();

protected:
    int mEndX;
    int mEndY;
    int mEndZ;
    Block* mBlock;
    int mAuxValue;
};

#endif

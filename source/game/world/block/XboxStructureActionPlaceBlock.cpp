#include "game/world/block/XboxStructureActionPlaceBlock.hpp"

#include "game/core/BoundingBox.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/generation/StructurePiece.hpp"

XboxStructureActionPlaceBlock::XboxStructureActionPlaceBlock()
{
    mAuxValue = 0;
    mEndZ = 0;
    mEndY = 0;
    mEndX = 0;
    mBlock = NULL;

    addIntAttribute(ConsoleGameRules::eGameRuleAttr_x, &mEndX);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_y, &mEndY);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_z, &mEndZ);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_data, &mAuxValue);
    addBlockAttribute(ConsoleGameRules::eGameRuleAttr_block, &mBlock);
}

uint32_t XboxStructureActionPlaceBlock::placeBlockInLevel(StructurePiece* structurePiece, Level* level,
                                                          BoundingBox* box)
{
    structurePiece->placeBlock(level, mBlock->getBlockState(mAuxValue), mEndX, mEndY, mEndZ, box);
    return 1;
}

ConsoleGameRules::EGameRuleType XboxStructureActionPlaceBlock::getActionType()
{
    return ConsoleGameRules::eGameRuleType_PlaceBlock;
}

uint32_t XboxStructureActionPlaceBlock::getEndX()
{
    return mEndX;
}

uint32_t XboxStructureActionPlaceBlock::getEndY()
{
    return mEndY;
}

uint32_t XboxStructureActionPlaceBlock::getEndZ()
{
    return mEndZ;
}

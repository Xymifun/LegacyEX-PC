#include "game/world/generation/XboxStructureActionGenerateBox.hpp"

#include "game/core/BoundingBox.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/StructurePiece.hpp"

XboxStructureActionGenerateBox::XboxStructureActionGenerateBox()
{
    mEndX = 0;
    mEndY = 0;
    mOuterBlock = NULL;
    mStartZ = 0;
    mEndZ = 0;
    mStartY = 0;
    mInnerBlock = NULL;
    mExistingOnly = false;
    mStartX = 0;

    addIntAttribute(ConsoleGameRules::eGameRuleAttr_x0, &mStartX);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_y0, &mStartY);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_z0, &mStartZ);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_x1, &mEndX);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_y1, &mEndY);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_z1, &mEndZ);
    addBlockAttribute(ConsoleGameRules::eGameRuleAttr_edgeBlock, &mOuterBlock);
    addBlockAttribute(ConsoleGameRules::eGameRuleAttr_fillBlock, &mInnerBlock);
    addBoolAttribute(ConsoleGameRules::eGameRuleAttr_skipAir, &mExistingOnly);
}

uint32_t XboxStructureActionGenerateBox::generateBoxInLevel(StructurePiece* structurePiece, Level* level,
                                                            BoundingBox* box)
{
    structurePiece->generateBox(level, box, mStartX, mStartY, mStartZ, mEndX, mEndY, mEndZ,
                                mOuterBlock->defaultBlockState(), mInnerBlock->defaultBlockState(), mExistingOnly);
    return 1;
}

ConsoleGameRules::EGameRuleType XboxStructureActionGenerateBox::getActionType()
{
    return ConsoleGameRules::eGameRuleType_GenerateBox;
}

uint32_t XboxStructureActionGenerateBox::getEndX()
{
    return mEndX;
}

uint32_t XboxStructureActionGenerateBox::getEndY()
{
    return mEndY;
}

uint32_t XboxStructureActionGenerateBox::getEndZ()
{
    return mEndZ;
}

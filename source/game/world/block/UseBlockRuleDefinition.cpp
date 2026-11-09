#include "game/world/block/UseBlockRuleDefinition.hpp"

#include "game/core/GameRule.hpp"
#include "game/minigame/GameRuleDefinition.hpp"

UseBlockRuleDefinition::UseBlockRuleDefinition()
{
    mBlockId = -1;
    mUsePosition = false;
    mX = 0;
    mY = 0;
    mZ = 0;

    addIntAttribute(ConsoleGameRules::eGameRuleAttr_blockId, &mBlockId);
    addBoolAttribute(ConsoleGameRules::eGameRuleAttr_useCoords, &mUsePosition);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_x, &mX);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_y, &mY);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_z, &mZ);
}

ConsoleGameRules::EGameRuleType UseBlockRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_UseBlock;
}

bool UseBlockRuleDefinition::onUseBlock(GameRule* gameRule, int blockId, int x, int y, int z)
{
    bool statusChanged = false;
    if (mBlockId == blockId)
    {
        if (!mUsePosition || (mX == x && mY == y && mZ == z))
        {
            if (!getComplete(gameRule))
            {
                statusChanged = true;
                setComplete(gameRule, true);
            }
        }
    }
    return statusChanged;
}

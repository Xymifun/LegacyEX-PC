#include "game/world/block/MushroomBlockThemeRuleDefinition.hpp"

#include "game/world/block/Blocks.hpp"
#include "game/world/block/HugeMushroomBlock.hpp"
#include "game/world/block/MushroomBlockLayerTheme.hpp"

static Block* sDefaultMushroomBlock = Blocks::red_mushroom_block;

MushroomBlockThemeRuleDefinition::MushroomBlockThemeRuleDefinition() : mBlock(sDefaultMushroomBlock)
{
    addBlockAttribute(ConsoleGameRules::eGameRuleAttr_blockId, &mBlock);
}

ConsoleGameRules::EGameRuleType MushroomBlockThemeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_MushroomBlockTheme;
}

LayerTheme* MushroomBlockThemeRuleDefinition::Generate() const
{
    HugeMushroomBlock* mushroomBlock = dynamic_cast<HugeMushroomBlock*>(mBlock);
    return new MushroomBlockLayerTheme(mushroomBlock != NULL ? mushroomBlock
                                                             : static_cast<HugeMushroomBlock*>(sDefaultMushroomBlock));
}

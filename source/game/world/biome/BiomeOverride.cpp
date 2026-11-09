#include "game/world/biome/BiomeOverride.hpp"

#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"

BiomeOverride::BiomeOverride() : mFillerBlock(0), mTopBlock(0), mBiomeId(0)
{
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_biomeId, &mBiomeId);
    addUnsignedCharAttribute(ConsoleGameRules::eGameRuleAttr_blockId, &mTopBlock);
    addUnsignedCharAttribute(ConsoleGameRules::eGameRuleAttr_topblockId, &mFillerBlock);
}

ConsoleGameRules::EGameRuleType BiomeOverride::getActionType()
{
    return ConsoleGameRules::eGameRuleType_BiomeOverride;
}

bool BiomeOverride::isBiome(int biomeId)
{
    return mBiomeId == biomeId;
}

void BiomeOverride::getBlockValues(Biome::BlockPair* topBlock, Biome::BlockPair* fillerBlock)
{
    topBlock->blockId = mTopBlock;

    Block* top = Block::byId(mTopBlock);
    topBlock->blockData = top->convertBlockStateToLegacyData(top->defaultBlockState());

    fillerBlock->blockId = mFillerBlock;

    Block* filler = Block::byId(mFillerBlock);
    fillerBlock->blockData = filler->convertBlockStateToLegacyData(filler->defaultBlockState());
}

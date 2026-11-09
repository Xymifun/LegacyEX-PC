#include "game/item/LootItemRandomDifficultyChanceCondition.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/LootTableContext.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/Random.hpp"

LootItemRandomDifficultyChanceCondition::LootItemRandomDifficultyChanceCondition(float peacefulChance, float easyChance,
                                                                                 float normalChance, float hardChance)
{
    chances[Difficulty::PEACEFUL->getId()] = peacefulChance;
    chances[Difficulty::EASY->getId()] = easyChance;
    chances[Difficulty::NORMAL->getId()] = normalChance;
    chances[Difficulty::HARD->getId()] = hardChance;
}

bool LootItemRandomDifficultyChanceCondition::applies(Random* random, LootTableContext& context)
{
    return random->nextFloat() < chances[context.mLevel->getDifficulty()->getId()];
}

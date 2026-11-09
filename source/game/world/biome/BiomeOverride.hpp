#ifndef GAME_WORLD_BIOME_BIOMEOVERRIDE_HPP
#define GAME_WORLD_BIOME_BIOMEOVERRIDE_HPP

#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/biome/Biome.hpp"

class BiomeOverride : public GameRuleDefinition
{
public:
    BiomeOverride();


    virtual ConsoleGameRules::EGameRuleType getActionType();

    bool isBiome(int biomeId);
    void getBlockValues(Biome::BlockPair* topBlock, Biome::BlockPair* fillerBlock);

protected:
    unsigned char mFillerBlock;
    unsigned char mTopBlock;
    int mBiomeId;
};

#endif

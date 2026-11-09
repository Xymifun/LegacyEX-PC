#ifndef GAME_WORLD_GENERATION_STARTFEATURE_HPP
#define GAME_WORLD_GENERATION_STARTFEATURE_HPP

#include <string>

#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/generation/StructureFeature.hpp"

class Direction;

class StartFeature : public GameRuleDefinition
{
public:
    StartFeature();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount);
    virtual void addAttribute(const std::wstring& name, const std::wstring& value);

    bool isFeatureChunk(int chunkX, int chunkZ, StructureFeature::EFeatureTypes featureType,
                        const Direction** orientation);

    int getFeatureType();
    int getChunkX();
    int getChunkZ();

private:
    int mChunkX;
    int mChunkZ;
    const Direction* mOrientation;
    int mFeatureType;
};

#endif

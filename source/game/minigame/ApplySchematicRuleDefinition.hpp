#ifndef GAME_MINIGAME_APPLYSCHEMATICRULEDEFINITION_HPP
#define GAME_MINIGAME_APPLYSCHEMATICRULEDEFINITION_HPP

#include <string>

#include "game/minigame/GameRuleDefinition.hpp"
#include "game/core/ConsoleSchematicFile.hpp"
#include "game/util/Vec3.hpp"

class AABB;
class BlockPos;
class GameRuleSaveInterface;
class LevelChunk;
class LevelGenerationOptions;

class ApplySchematicRuleDefinition : public GameRuleDefinition
{
public:
    ApplySchematicRuleDefinition(LevelGenerationOptions* options);
    virtual ~ApplySchematicRuleDefinition();

    void reset();
    bool isComplete();
    void updateLocationBox();
    void processSchematic(AABB* chunkBox, LevelChunk* chunk);
    void processSchematicLighting(AABB* chunkBox, LevelChunk* chunk);
    int getMinY();
    bool checkIntersects(const BlockPos& min, const BlockPos& max);
    std::wstring getSchematicName();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount);
    virtual void addAttribute(const std::wstring& name, const std::wstring& value);

private:
    LevelGenerationOptions* mLevelGenerationOptions;
    std::wstring mSchematicName;
    ConsoleSchematicFile* mSchematicFile;
    Vec3* mOrigin;
    AABB* mLocationBox;
    ConsoleSchematicFile::ESchematicRotation mRotation;
    int mDimension;
    long long mBlocksApplied;
    long long mLightingApplied;
    bool mComplete;
};

#endif

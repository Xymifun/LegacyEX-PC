#ifndef GAME_WORLD_LEVELRULESET_HPP
#define GAME_WORLD_LEVELRULESET_HPP

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/minigame/CompoundGameRuleDefinition.hpp"
#include "game/world/LevelRules.hpp"

class AABB;
class ActiveViewAreaRuleDefinition;
class Block;
class BlockLayerRuleDefinition;
class CheckpointRuleDefinition;
class CollisionExceptionGameRuleDefinition;
class KillboxRuleDefinition;
class GrantPermissionsRuleDefinition;
class LayerGenerationRuleDefinition;
class LevelGenerationOptions;
class NamedAreaRuleDefinition;
class PowerupRuleDefinition;
class RandomItemSetRuleDefinition;
class StringTable;
class TargetAreaRuleDefinition;
class ThermalAreaRuleDefinition;

class LevelRuleset : public CompoundGameRuleDefinition
{
public:
    LevelRuleset();
    virtual ~LevelRuleset();

    RandomItemSetRuleDefinition* getRandomItemSet(const std::wstring& name);
    LevelGenerationOptions* getGenerationOptions();
    LevelRules::ELevelRuleType getRuleType();
    unsigned int getThemeId();

    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    void addUniqueRule(ConsoleGameRules::EGameRuleType type, GameRuleDefinition* rule);
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);
    virtual void addAttribute(const std::wstring& name, const std::wstring& value);
    void loadStringTable(StringTable* stringTable);
    wchar_t* getString(const std::wstring& key);
    NamedAreaRuleDefinition* getNamedAreaRule(const std::wstring& name);
    AABB* getNamedArea(const std::wstring& name);
    void getNamedAreas(int areaType, std::vector<AABB*>* areas);
    void getNonSpecialisedNamedAreas(std::vector<AABB*>* areas);
    void getActiveChunkAreas(std::vector<AABB*>* areas);
    void getActiveChunkAreas(std::vector<NamedAreaRuleDefinition*>* areas);
    void getActiveViewAreas(std::vector<AABB*>* areas);
    void getActiveViewAreas(std::vector<ActiveViewAreaRuleDefinition*>* areas);
    void getTargetAreas(std::vector<TargetAreaRuleDefinition*>* areas);
    void getRingsWithinArea(std::vector<TargetAreaRuleDefinition*>* rings, AABB* area);
    void getThermalAreas(std::vector<ThermalAreaRuleDefinition*>* areas);
    void getPowerups(std::vector<PowerupRuleDefinition*>* powerups);
    void getPlayerBoundsVolumes(std::vector<NamedAreaRuleDefinition*>* areas);
    void getKillboxes(std::vector<KillboxRuleDefinition*>* killboxes);
    std::vector<CheckpointRuleDefinition*>* getCheckpoints();
    CheckpointRuleDefinition* getCheckpointByID(int id);
    std::vector<CheckpointRuleDefinition*> getCheckpointsBySequenceID(int sequenceId);
    GameRuleDefinition* getRuleByType(ConsoleGameRules::EGameRuleType type);
    virtual void getChildren(std::vector<GameRuleDefinition*>* children, ConsoleGameRules::EGameRuleType type);
    StringTable* getStringTable();
    void setGenerationOptions(LevelGenerationOptions* options);
    bool ShouldCollideWith(Block* block);
    void WriteRulesetXMLData();
    virtual ConsoleGameRules::EGameRuleType getActionType();

private:
    std::vector<NamedAreaRuleDefinition*> mNamedAreas;
    std::vector<RandomItemSetRuleDefinition*> mRandomItemSets;
    boost::unordered_map<ConsoleGameRules::EGameRuleType, GameRuleDefinition*> mRules;
    std::vector<CollisionExceptionGameRuleDefinition*> mCollisionExceptions;
    std::vector<CheckpointRuleDefinition*> mCheckpoints;
    std::vector<PowerupRuleDefinition*> mPowerups;
    std::vector<KillboxRuleDefinition*> mKillboxes;
    LevelGenerationOptions* mGenerationOptions;
    LevelRules::ELevelRuleType mRuleType;
    StringTable* mStringTable;
};

#endif

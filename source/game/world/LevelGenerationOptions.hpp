#ifndef GAME_WORLD_LEVELGENERATIONOPTIONS_HPP
#define GAME_WORLD_LEVELGENERATIONOPTIONS_HPP

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/core/File.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/StartFeature.hpp"

class ApplySchematicRuleDefinition;
class BiomeOverride;
class ConsoleGenerateStructure;
class ConsoleSchematicFile;
class Direction;
class DLCPack;
class GameRulesSource;
class LevelChunk;
class LevelRules;
class LevelRuleset;
class StringTable;

class LevelGenerationOptions : public GameRuleDefinition
{
public:
    enum EMapSize
    {
        eMapSize_Small,
        eMapSize_Medium,
        eMapSize_Large
    };

    typedef boost::unordered_map<std::wstring, ConsoleSchematicFile*> SchematicFileMap;

    enum eSrc
    {
        eSrc_Unknown = 0,
        eSrc_Save = 1,
        eSrc_DLC = 2,
        eSrc_Local = 3,
        eSrc_Tutorial = 4
    };

    LevelGenerationOptions(DLCPack* pack = NULL);
    virtual ~LevelGenerationOptions();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount);
    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);
    virtual void addAttribute(const std::wstring& name, const std::wstring& value);

    unsigned int getThemeId() const;
    static unsigned short GetThemeFromUniqueId(int uniqueId);
    static unsigned int CreateUniqueId(int themeId, int mapSize);
    unsigned int getUniqueId();
    GameRulesSource* info();
    void setSrc(eSrc source);
    eSrc getSrc();
    bool isLocal();
    bool isTutorial();
    bool isFromSave();
    bool isFromDLC();
    bool requiresTexturePack();
    unsigned int getTexturePackId();
    const wchar_t* getString(const std::wstring& key);
    std::wstring getDefaultSaveName();
    const wchar_t* getWorldName();
    std::wstring getWorldNameKey();
    const wchar_t* getDisplayName();
    const wchar_t* getWorldDescription();
    const wchar_t* getWorldPreviewImage();
    std::wstring getGrfPath();
    bool requiresBaseSave();
    void setBaseSaveRoot(const std::wstring& root);
    std::wstring getBaseSaveRoot();
    std::wstring getBaseSavePath();
    bool isGameModeLobby();
    unsigned int getMapSize() const;
    void setGrSource(GameRulesSource* source);
    void setRequiresTexturePack(bool required);
    void setTexturePackId(unsigned int id);
    void setDefaultSaveName(const std::wstring& name);
    void setWorldName(const std::wstring& name);
    void setDisplayName(const std::wstring& name);
    void setWorldDescription(const std::wstring& description);
    void setWorldPreviewImage(const std::wstring& image);
    void setGrfPath(const std::wstring& path);
    void setBaseSavePath(const std::wstring& path);
    void setIsGameModeLobby(bool lobby);
    bool ready();
    void reset_finish();
    bool requiresGameRules();
    LevelRuleset* getRequiredGameRules();
    void reset_start();
    static void loadGameRulesHeaders(DLCPack* pack, StringTable* table, LevelGenerationOptions* options);
    void deleteBaseSaveData();
    void setBaseSaveData(unsigned char* data, unsigned int size);
    void setBaseSaveData(File file);
    unsigned char* getBaseSaveData(unsigned int& size);
    bool hasBaseSaveData();
    bool hasLoadedData();
    void setLoadedData();
    void clearSchematics();
    DLCPack* getPack() const { return mPack; }
    unsigned int getParentPackId();
    bool hasLevelSeed();
    long long getLevelSeed();
    bool getLevelHasBeenInCreative();
    BlockPos* getSpawnPos();
    bool getuseFlatWorld();
    int getCloudHeight();
    int getFogDistance();
    int getDayTime();
    static unsigned short GetMapSizeFromUniqueId(int uniqueId);
    ConsoleSchematicFile* getSchematicFile(const std::wstring& name) const;
    void processSchematics(LevelChunk* chunk);
    void processSchematicsLighting(LevelChunk* chunk);
    bool checkIntersects(const BlockPos& min, const BlockPos& max);
    void setCRCIsValid(bool valid);
    bool isCRCValid();
    ConsoleSchematicFile* loadSchematicFile(const std::wstring& name, unsigned char* data, unsigned int size);
    void setRequiredGameRules(LevelRuleset* ruleset);
    void getBiomeOverride(int biome, Biome::BlockPair* top, Biome::BlockPair* filler);
    bool isFeatureChunk(int chunkX, int chunkZ, StructureFeature::EFeatureTypes type, const Direction** direction);
    bool hasFeature(StructureFeature::EFeatureTypes type);
    BlockPos* getClosestFeature(StructureFeature::EFeatureTypes type, const BlockPos& pos);
    void loadStringTable(StringTable* table);
    void copyStringToTable(const std::wstring& value, StringTable* table);
    SchematicFileMap* getUnfinishedSchematicFiles();
    static int packMounted(void* data, int mountIndex, unsigned int mounted, unsigned int flags);
    void loadBaseSaveData();
    bool canUnload();
    void releaseSchematicFile(const std::wstring& name);

private:
    friend class LevelRules;
    friend class TextureThemeRuleDefinition;

    eSrc mSource;
    GameRulesSource* mGameRulesSource;
    bool mLoadedData;
    unsigned char* mBaseSaveData;
    unsigned int mBaseSaveDataSize;
    std::wstring mBaseSaveRoot;
    bool mHasLevelSeed;
    long long mLevelSeed;
    bool mUseFlatWorld;
    BlockPos* mSpawnPos;
    bool mLevelHasBeenInCreative;
    int mMapSize;
    int mThemeId;
    int mCloudHeight;
    int mFogDistance;
    int mDayTime;
    std::vector<ApplySchematicRuleDefinition*> mSchematicRules;
    std::vector<ConsoleGenerateStructure*> mStructures;
    bool mMinYCalculated;
    int mMinY;
    SchematicFileMap mSchematicFiles;
    std::vector<BiomeOverride*> mBiomeOverrides;
    std::vector<StartFeature*> mFeatures;
    bool mRequiresGameRules;
    LevelRuleset* mRequiredGameRules;
    std::wstring mWorldDescription;
    std::wstring mWorldPreviewImage;
    StringTable* mStringTable;
    DLCPack* mPack;
    bool mPackMountPending;
    bool mCRCValid;
};

#endif

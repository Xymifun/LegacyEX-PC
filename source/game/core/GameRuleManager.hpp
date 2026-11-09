#ifndef GAME_CORE_GAMERULEMANAGER_HPP
#define GAME_CORE_GAMERULEMANAGER_HPP

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/data/GameRuleSaveInterface.hpp"
#include "game/world/LevelRules.hpp"

class DataInputStream;
class DataOutputStream;
class CMinecraftApp;
class DLCPack;
class File;
class GameRuleDefinition;
class LevelChunk;
class LevelGenerationOptions;
class LevelGenerators;
class LevelRuleset;
class StringTable;
class DLCGameRulesHeader;

class GameRuleManager
{
public:
    GameRuleManager();

    void unloadCurrentGameRules();
    void setLevelGenerationOptions(LevelGenerationOptions* options);
    static void MakeCRCTable();
    static unsigned int UpdateCRC(unsigned int crc, unsigned char* data, int length);
    static unsigned int CRC(unsigned char* data, int length);
    unsigned int addLevelGenerationOptions(LevelGenerationOptions* options);
    void readAttributes(DataInputStream* input, std::vector<std::wstring>* names, GameRuleDefinition* definition);
    void readChildren(DataInputStream* input, std::vector<std::wstring>* names,
                      boost::unordered_map<int, ConsoleGameRules::EGameRuleType>* types,
                      GameRuleDefinition* definition);
    unsigned int readRuleFile(LevelGenerationOptions* options, unsigned char* data, unsigned int size,
                              StringTable* table);
    void loadGameRules(DLCPack* pack);
    void loadGameRules(LevelGenerationOptions* options, unsigned char* data, unsigned int size);
    LevelGenerationOptions* loadGameRules(unsigned char* data, unsigned int size);
    LevelGenerationOptions* getLevelGenerationOptions();
    void writeRuleFile(DataOutputStream* output);
    void saveGameRules(unsigned char** data, unsigned int* size);
    void processSchematics(LevelChunk* chunk);
    void processSchematicsLighting(LevelChunk* chunk);
    unsigned int loadGameRulesPack(File* file);
    void loadDefaultGameRules();
    std::vector<LevelGenerationOptions*>* getLevelGenerators();
    LevelRuleset* getGameRuleDefinitions();
    const wchar_t* GetGameRulesString(const std::wstring& key);
    void readHeader(DLCGameRulesHeader* header);
    LevelRuleset* getDebugGameRuleDefinitions();
    void setDebugGameRuleDefinitions(LevelRuleset* ruleset);

    static const wchar_t* sAttributeNames[ConsoleGameRules::eGameRuleAttr_Count];
    static const wchar_t* sTagNames[ConsoleGameRules::eGameRuleType_Count];

private:
    friend class CMinecraftApp;

    LevelGenerationOptions* mLevelGenerationOptions;
    LevelRuleset* mGameRuleDefinitions;
    LevelGenerators* mLevelGenerators;
    LevelRules* mLevelRules;
    LevelRuleset* mDebugGameRuleDefinitions;
};


#endif

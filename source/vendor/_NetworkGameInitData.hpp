#ifndef VENDOR__NETWORKGAMEINITDATA_HPP
#define VENDOR__NETWORKGAMEINITDATA_HPP

#include <vector>

#include "game/core/FileHeader.hpp"
#include "game/world/LevelRules.hpp"

class LevelGenerationOptions;
class LevelType;
class SuperflatConfig;
struct DiskSaveBuffer;

class _NetworkGameInitData
{
public:
    _NetworkGameInitData();

    long long m_seed;
    DiskSaveBuffer* m_saveData;
    int m_hostOptions;
    int m_miniGameHostOptions;
    LevelGenerationOptions* m_levelGenerationOptions;
    int m_texturePackId;
    bool m_findSeed;
    int m_xzSize;
    unsigned char m_hellScale;
    ESavePlatform m_savePlatform;
    int m_gameType;
    SuperflatConfig* m_levelTypeOptions;
    std::vector<LevelRules::LevelRulesetGroup*> m_ruleGroups;
    unsigned char m_biomeScale;
    int m_biomeCentreXChunk;
    int m_biomeCentreZChunk;
    LevelType* m_levelType;
    unsigned char m_recordLevelResume;
};

#endif

#ifndef GAME_WORLD_LEVELSETTINGS_HPP
#define GAME_WORLD_LEVELSETTINGS_HPP
#include "game/core/GameType.hpp"
#include "game/world/LevelType.hpp"
class LevelData;
class SuperflatConfig;
class LevelSettings
{
public:
    LevelSettings(long long, const GameType*, bool, bool, bool, LevelType*, int, int, int, bool, unsigned char, int,
                  int);
    explicit LevelSettings(LevelData*);
    void _init(long long, const GameType*, bool, bool, bool, LevelType*, int, int, int, bool, unsigned char, int, int);
    void enableStartingBonusItems();
    void enableSinglePlayerCommands();
    void setLevelTypeOptions(SuperflatConfig*);
    bool hasStartingBonusItems();
    long long getSeed();
    const GameType* getGameType();
    bool isHardcore();
    LevelType* getLevelType();
    bool getAllowCommands();
    bool isGenerateMapFeatures();
    int getCloudHeight();
    bool useNewSeaLevel();
    int getXZSize();
    int getHellScale();
    bool getModernEnd();
    unsigned char getBiomeScale();
    int getBiomeCentreXChunk();
    int getBiomeCentreZChunk();
    static const GameType* validateGameType(int);
    SuperflatConfig* getLevelTypeOptions();

private:
    long long mSeed;
    const GameType* mGameType;
    bool mGenerateMapFeatures;
    bool mHardcore;
    int mCloudHeight;
    bool mUseNewSeaLevel;
    LevelType* mLevelType;
    bool mAllowCommands;
    bool mStartingBonusItems;
    SuperflatConfig* mLevelTypeOptions;
    int mXZSize;
    int mHellScale;
    bool mModernEnd;
    unsigned char mBiomeScale;
    int mBiomeCentreXChunk;
    int mBiomeCentreZChunk;
};
#endif

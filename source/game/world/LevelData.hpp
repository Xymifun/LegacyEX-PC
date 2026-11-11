#ifndef SOURCE_GAME_WORLD_LEVELDATA_HPP
#define SOURCE_GAME_WORLD_LEVELDATA_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/world/block/BlockPos.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/GameType.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/TagUpdateWalker.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/LevelType.hpp"

class LevelSettings;
class SuperflatConfig;
class Player;

#ifdef MINECRAFT_PC
#include "platform/pc/PcModStorage.hpp"
#endif

class LevelData
{
public:
#ifdef MINECRAFT_PC
    std::vector<std::pair<std::wstring, int> > mModBlockIds;
    PcModData mModData;
#endif
    LevelData();
    explicit LevelData(CompoundTag* tag);
    LevelData(LevelSettings* settings, const std::wstring& name);
    LevelData(LevelData* other);
    virtual ~LevelData();
    virtual CompoundTag* createTag();
    virtual CompoundTag* createTag(std::vector<boost::shared_ptr<Player> >* players);
    static void addWalkers(DataFixerUpper* fixer);
    virtual void setTagData(CompoundTag* tag);
    virtual long long getSeed();
    virtual int getXSpawn();
    virtual int getYSpawn();
    virtual int getZSpawn();
    virtual int getXStronghold();
    virtual int getZStronghold();
    virtual int getXStrongholdEndPortal();
    virtual int getZStrongholdEndPortal();
    virtual long long getGameTime();
    virtual long long getDayTime();
    virtual long long getSizeOnDisk();
    virtual CompoundTag* getLoadedPlayerTag();
    virtual void setSeed(long long value);
    virtual void setXSpawn(int value);
    virtual void setYSpawn(int value);
    virtual void setZSpawn(int value);
    virtual void setHasStronghold();
    virtual bool getHasStronghold();
    virtual void setXStronghold(int value);
    virtual void setZStronghold(int value);
    virtual void setHasStrongholdEndPortal();
    virtual bool getHasStrongholdEndPortal();
    virtual void setXStrongholdEndPortal(int value);
    virtual void setZStrongholdEndPortal(int value);
    virtual void setGameTime(long long value);
    virtual void setDayTime(long long value);
    virtual void setSizeOnDisk(long long value);
    virtual void setLoadedPlayerTag(CompoundTag* tag);
    virtual void setSpawn(const BlockPos& pos);
    virtual std::wstring getLevelName();
    virtual void setLevelName(const std::wstring& name);
    virtual int getVersion();
    virtual void setVersion(int value);
    virtual long long getLastPlayed();
    virtual int getClearWeatherTime();
    virtual void setClearWeatherTime(int value);
    virtual bool isThundering();
    virtual void setThundering(bool value);
    virtual int getThunderTime();
    virtual void setThunderTime(int value);
    virtual bool isRaining();
    virtual void setRaining(bool value);
    virtual int getRainTime();
    virtual void setRainTime(int value);
    virtual const GameType* getGameType();
    virtual bool isGenerateMapFeatures();
    virtual void setGenerateMapFeatures(bool value);
    virtual bool getSpawnBonusChest();
    virtual void setGameType(const GameType* type);
    virtual bool useNewSeaLevel();
    virtual bool getHasBeenInCreative();
    virtual void setHasBeenInCreative(bool value);
    virtual LevelType* getGeneratorType();
    virtual void setGeneratorType(LevelType* type);
    virtual SuperflatConfig* getGeneratorOptions();
    virtual void setGeneratorOptions(SuperflatConfig* options);
    virtual bool isHardcore();
    virtual void setHardcore(bool value);
    virtual bool getAllowCommands();
    virtual void setAllowCommands(bool value);
    virtual bool isInitialized();
    virtual void setInitialized(bool value);
    virtual GameRules* getGameRules();
    virtual const Difficulty* getDifficulty();
    virtual void setDifficulty(const Difficulty* difficulty);
    virtual int getCloudHeight();
    virtual void setCloudHeight(int value);
    virtual bool isDifficultyLocked();
    virtual void setDifficultyLocked(bool value);
    virtual CompoundTag* getDimensionData(const DimensionType* dimension);
    virtual void setDimensionData(const DimensionType* dimension, CompoundTag* tag);
    virtual int getXZSize();
    virtual int getHellScale();
    virtual void setModernEnd();
    virtual bool getModernEnd();
    virtual unsigned char getBiomeScale();
    virtual int getBiomeCentreXChunk();
    virtual int getBiomeCentreZChunk();

private:
    long long mSeed;
    LevelType* mGeneratorType;
    SuperflatConfig* mGeneratorOptions;
    int mXSpawn;
    int mYSpawn;
    int mZSpawn;
    long long mGameTime;
    long long mDayTime;
    long long mLastPlayed;
    long long mSizeOnDisk;
    CompoundTag* mLoadedPlayerTag;
    std::wstring mLevelName;
    int mVersion;
    int mClearWeatherTime;
    bool mRaining;
    int mRainTime;
    bool mThundering;
    int mThunderTime;
    const GameType* mGameType;
    bool mGenerateMapFeatures;
    bool mHardcore;
    bool mAllowCommands;
    bool mInitialized;
    const Difficulty* mDifficulty;
    bool mDifficultyLocked;
    boost::unordered_map<const DimensionType*, CompoundTag*> mDimensionData;
    int mCloudHeight;
    bool mUseNewSeaLevel;
    bool mHasBeenInCreative;
    bool mSpawnBonusChest;
    int mXZSize;
    int mHellScale;
    bool mModernEnd;
    int mSpawnXStronghold;
    int mSpawnYStronghold;
    int mSpawnZStronghold;
    bool mHasStronghold;
    int mXStrongholdEndPortal;
    int mZStrongholdEndPortal;
    bool mHasStrongholdEndPortal;
    unsigned char mBiomeScale;
    int mBiomeCentreXChunk;
    int mBiomeCentreZChunk;
    GameRules mGameRules;
};

#endif

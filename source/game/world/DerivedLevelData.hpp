#ifndef GAME_WORLD_DERIVEDLEVELDATA_HPP
#define GAME_WORLD_DERIVEDLEVELDATA_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/world/LevelData.hpp"

class DerivedLevelData : public LevelData
{
public:
    explicit DerivedLevelData(LevelData* levelData);
    virtual CompoundTag* createTag(std::vector<boost::shared_ptr<Player> >* players);
    virtual void setTagData(CompoundTag* tag);
    virtual long long getSeed();
    virtual int getXSpawn();
    virtual int getYSpawn();
    virtual int getZSpawn();
    virtual long long getGameTime();
    virtual long long getDayTime();
    virtual long long getSizeOnDisk();
    virtual CompoundTag* getLoadedPlayerTag();
    virtual void setSeed(long long value);
    virtual void setXSpawn(int value);
    virtual void setYSpawn(int value);
    virtual void setZSpawn(int value);
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
    virtual bool isThundering();
    virtual int getThunderTime();
    virtual bool isRaining();
    virtual int getRainTime();
    virtual const GameType* getGameType();
    virtual void setThundering(bool value);
    virtual void setThunderTime(int value);
    virtual void setRaining(bool value);
    virtual void setRainTime(int value);
    virtual bool isGenerateMapFeatures();
    virtual void setGameType(const GameType* type);
    virtual bool isHardcore();
    virtual LevelType* getGeneratorType();
    virtual void setGeneratorType(LevelType* type);
    virtual bool getAllowCommands();
    virtual void setAllowCommands(bool value);
    virtual bool isInitialized();
    virtual void setInitialized(bool value);
    virtual GameRules* getGameRules();
    virtual const Difficulty* getDifficulty();
    virtual void setDifficulty(const Difficulty* difficulty);
    virtual bool isDifficultyLocked();
    virtual void setDifficultyLocked(bool value);
    virtual void setDimensionData(const DimensionType* dimension, CompoundTag* tag);
    virtual CompoundTag* getDimensionData(const DimensionType* dimension);
    virtual int getXZSize();
    virtual int getHellScale();
    virtual void setModernEnd();
    virtual bool getModernEnd();

private:
    LevelData* mLevelData;
};

#endif

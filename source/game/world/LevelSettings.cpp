#include "game/world/LevelSettings.hpp"
#include "game/world/LevelData.hpp"
#include "game/core/SuperflatConfig.hpp"

LevelSettings::LevelSettings(long long seed, const GameType* gameType, bool generateMapFeatures, bool hardcore,
                             bool allowCommands, LevelType* levelType, int xzSize, int hellScale, int cloudHeight,
                             bool modernEnd, unsigned char biomeScale, int biomeCentreXChunk, int biomeCentreZChunk)
    : mSeed(seed),
      mGameType(gameType),
      mGenerateMapFeatures(generateMapFeatures),
      mHardcore(hardcore),
      mCloudHeight(cloudHeight),
      mUseNewSeaLevel(allowCommands),
      mLevelType(levelType),
      mStartingBonusItems(false)
{
    _init(seed, gameType, generateMapFeatures, hardcore, allowCommands, levelType, xzSize, hellScale, cloudHeight,
          modernEnd, biomeScale, biomeCentreXChunk, biomeCentreZChunk);
}

LevelSettings::LevelSettings(LevelData* levelData)
{
    _init(levelData->getSeed(), levelData->getGameType(), levelData->isGenerateMapFeatures(), levelData->isHardcore(),
          levelData->useNewSeaLevel(), levelData->getGeneratorType(), levelData->getXZSize(), levelData->getHellScale(),
          levelData->getCloudHeight(), levelData->getModernEnd(),
          static_cast<unsigned char>(levelData->getBiomeScale()), levelData->getBiomeCentreXChunk(),
          levelData->getBiomeCentreZChunk());
}

void LevelSettings::_init(long long seed, const GameType* gameType, bool generateMapFeatures, bool hardcore,
                          bool allowCommands, LevelType* levelType, int xzSize, int hellScale, int cloudHeight,
                          bool modernEnd, unsigned char biomeScale, int biomeCentreXChunk, int biomeCentreZChunk)
{
    mGenerateMapFeatures = generateMapFeatures;
    mGameType = gameType;
    mHardcore = hardcore;
    mCloudHeight = cloudHeight;
    mUseNewSeaLevel = allowCommands;
    mLevelType = levelType;
    mAllowCommands = false;
    mXZSize = xzSize;
    mStartingBonusItems = false;
    mHellScale = hellScale;
    mBiomeScale = biomeScale;
    mModernEnd = modernEnd;
    mSeed = seed;
    mBiomeCentreXChunk = biomeCentreXChunk;
    mLevelTypeOptions = 0;
    mBiomeCentreZChunk = biomeCentreZChunk;
}

void LevelSettings::enableStartingBonusItems()
{
    mStartingBonusItems = true;
}

void LevelSettings::enableSinglePlayerCommands()
{
    mAllowCommands = true;
}

void LevelSettings::setLevelTypeOptions(SuperflatConfig* options)
{
    mLevelTypeOptions = options;
}

bool LevelSettings::hasStartingBonusItems()
{
    return mStartingBonusItems;
}

long long LevelSettings::getSeed()
{
    return mSeed;
}

const GameType* LevelSettings::getGameType()
{
    return mGameType;
}

bool LevelSettings::isHardcore()
{
    return mHardcore;
}

LevelType* LevelSettings::getLevelType()
{
    return mLevelType;
}

bool LevelSettings::getAllowCommands()
{
    return mAllowCommands;
}

bool LevelSettings::isGenerateMapFeatures()
{
    return mGenerateMapFeatures;
}

int LevelSettings::getCloudHeight()
{
    return mCloudHeight;
}

bool LevelSettings::useNewSeaLevel()
{
    return mUseNewSeaLevel;
}

int LevelSettings::getXZSize()
{
    return mXZSize;
}

int LevelSettings::getHellScale()
{
    return mHellScale;
}

bool LevelSettings::getModernEnd()
{
    return mModernEnd;
}

unsigned char LevelSettings::getBiomeScale()
{
    return mBiomeScale;
}

int LevelSettings::getBiomeCentreXChunk()
{
    return mBiomeCentreXChunk;
}

int LevelSettings::getBiomeCentreZChunk()
{
    return mBiomeCentreZChunk;
}

const GameType* LevelSettings::validateGameType(int gameType)
{
    return GameType::byId(gameType);
}

SuperflatConfig* LevelSettings::getLevelTypeOptions()
{
    return mLevelTypeOptions;
}

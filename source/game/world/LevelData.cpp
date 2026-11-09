#include "game/world/LevelData.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcModContent.hpp"
#endif

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/core/System.hpp"
#include "game/data/ByteArrayTag.hpp"
#include "game/world/LevelSettings.hpp"

#ifdef MINECRAFT_PC
static const int LEVEL_MAX_WIDTH = 640;
static const int LEVEL_MIN_WIDTH = 54;
static const int HELL_LEVEL_MAX_SCALE = 8;
static const int HELL_LEVEL_MIN_SCALE = 3;
static const int HELL_LEVEL_MAX_WIDTH = 80;
#else
static const int LEVEL_MAX_WIDTH = 54;
static const int LEVEL_MIN_WIDTH = 54;
static const int HELL_LEVEL_MAX_SCALE = 3;
static const int HELL_LEVEL_MIN_SCALE = 3;
static const int HELL_LEVEL_MAX_WIDTH = 18;
#endif

LevelData::LevelData()
{
    mSeed = 0;
    mGeneratorType = 0;
    mGeneratorOptions = 0;
    mXSpawn = 0;
    mYSpawn = 0;
    mZSpawn = 0;
    mGameTime = 0;
    mDayTime = 0;
    mLastPlayed = 0;
    mSizeOnDisk = 0;
    mLoadedPlayerTag = 0;
    mLevelName = L"";
    mVersion = 0;
    mClearWeatherTime = 0;
    mRaining = false;
    mRainTime = 0;
    mThundering = false;
    mThunderTime = 0;
    mGameType = 0;
    mGenerateMapFeatures = false;
    mHardcore = false;
    mAllowCommands = false;
    mInitialized = false;
    mDifficulty = Difficulty::NORMAL;
    mCloudHeight = 128;
    mUseNewSeaLevel = false;
    mHasBeenInCreative = false;
    mSpawnBonusChest = false;
    mXZSize = 0;
    mHellScale = 0;
    mModernEnd = true;
    mSpawnXStronghold = 0;
    mSpawnYStronghold = 0;
    mSpawnZStronghold = 0;
    mHasStronghold = false;
    mXStrongholdEndPortal = 0;
    mZStrongholdEndPortal = 0;
    mHasStrongholdEndPortal = false;
    mBiomeScale = 0;
    mBiomeCentreXChunk = 0;
    mBiomeCentreZChunk = 0;
}

LevelData::LevelData(CompoundTag* tag)
{
    mGeneratorOptions = 0;
    mSeed = tag->getLong(L"RandomSeed");
    mGeneratorType = LevelType::DEFAULT;

    if (tag->contains(L"generatorName", Tag::TAG_STRING))
    {
        std::wstring generatorName = tag->getString(L"generatorName");
        LevelType* generatorType = LevelType::getLevelType(generatorName);
        if (generatorType == 0)
        {
            mGeneratorType = LevelType::DEFAULT;
        }
        else
        {
            mGeneratorType = generatorType;
            if (mGeneratorType->hasReplacement())
            {
                tag->contains(L"generatorVersion");
                int generatorVersion = tag->getInt(L"generatorVersion");
                mGeneratorType = mGeneratorType->getReplacementForVersion(generatorVersion);
            }
        }

        bool isFlat = false;
        if (tag->contains(L"generatorOptions", Tag::TAG_BYTE_ARRAY) && generatorName == L"flat")
        {
            isFlat = true;
        }

        if (isFlat)
        {
            mGeneratorOptions = new SuperflatConfig();
            ByteArrayTag* options = dynamic_cast<ByteArrayTag*>(tag->get(L"generatorOptions"));
            if (options != 0)
            {
                mGeneratorOptions->fromTag(options);
            }
            else
            {
                mGeneratorOptions->makeDefaults();
            }
        }
        else
        {
            mGeneratorOptions = 0;
        }
    }

    mGameType = GameType::byId(tag->getInt(L"GameType"));

    if (tag->contains(L"MapFeatures", Tag::TAG_ANY_NUMERIC))
    {
        mGenerateMapFeatures = tag->getBoolean(L"MapFeatures");
    }
    else
    {
        mGenerateMapFeatures = true;
    }

    mSpawnBonusChest = tag->getBoolean(L"spawnBonusChest");
    mXSpawn = tag->getInt(L"SpawnX");
    mYSpawn = tag->getInt(L"SpawnY");
    mZSpawn = tag->getInt(L"SpawnZ");
    mGameTime = tag->getLong(L"Time");

    if (tag->contains(L"DayTime", Tag::TAG_ANY_NUMERIC))
    {
        mDayTime = tag->getLong(L"DayTime");
    }
    else
    {
        mDayTime = mGameTime;
    }

    mLastPlayed = tag->getLong(L"LastPlayed");
    mSizeOnDisk = tag->getLong(L"SizeOnDisk");
    mLevelName = tag->getString(L"LevelName");
    mVersion = tag->getInt(L"version");
    mClearWeatherTime = tag->getInt(L"clearWeatherTime");
    mRainTime = tag->getInt(L"rainTime");
    mRaining = tag->getBoolean(L"raining");
    mThunderTime = tag->getInt(L"thunderTime");
    mThundering = tag->getBoolean(L"thundering");
    mHardcore = tag->getBoolean(L"hardcore");

    if (tag->contains(L"initialized", Tag::TAG_ANY_NUMERIC))
    {
        mInitialized = tag->getBoolean(L"initialized");
    }
    else
    {
        mInitialized = true;
    }

    if (tag->contains(L"allowCommands", Tag::TAG_ANY_NUMERIC))
    {
        mAllowCommands = tag->getBoolean(L"allowCommands");
    }
    else
    {
        mAllowCommands = (mGameType == GameType::CREATIVE);
    }

    if (tag->contains(L"Difficulty", Tag::TAG_ANY_NUMERIC))
    {
        mDifficulty = Difficulty::byId(tag->getByte(L"Difficulty"));
    }

    if (tag->contains(L"DifficultyLocked", Tag::TAG_BYTE))
    {
        mDifficultyLocked = tag->getBoolean(L"DifficultyLocked");
    }

    if (tag->contains(L"DimensionData", Tag::TAG_COMPOUND))
    {
        CompoundTag* dimensionData = tag->getCompound(L"DimensionData");
        std::vector<std::wstring>* keys = dimensionData->getAllKeys();
        for (std::vector<std::wstring>::iterator it = keys->begin(); it != keys->end(); ++it)
        {
            const DimensionType* dimensionType = DimensionType::getByName(*it);
            CompoundTag* dimensionTag = dimensionData->getCompound(*it);
            mDimensionData[dimensionType] = dimensionTag->copy();
        }
        delete keys;
    }

    if (tag->contains(L"cloudHeight"))
    {
        mCloudHeight = tag->getInt(L"cloudHeight");
    }
    else
    {
        mCloudHeight = 128;
    }

    mUseNewSeaLevel = tag->getBoolean(L"newSeaLevel");
    mHasBeenInCreative = tag->getBoolean(L"hasBeenInCreative");
    mHasStronghold = tag->getBoolean(L"hasStronghold");

    if (mHasStronghold)
    {
        mSpawnXStronghold = tag->getInt(L"StrongholdX");
        mSpawnYStronghold = tag->getInt(L"StrongholdY");
        mSpawnZStronghold = tag->getInt(L"StrongholdZ");
    }
    else
    {
        mSpawnYStronghold = 0;
        mSpawnXStronghold = 0;
        mSpawnZStronghold = 0;
    }
    mHasStrongholdEndPortal = tag->getBoolean(L"hasStrongholdEndPortal");

    if (mHasStrongholdEndPortal)
    {
        mXStrongholdEndPortal = tag->getInt(L"StrongholdEndPortalX");
        mZStrongholdEndPortal = tag->getInt(L"StrongholdEndPortalZ");
    }
    else
    {
        mXStrongholdEndPortal = 0;
        mZStrongholdEndPortal = 0;
    }
    mXZSize = tag->getInt(L"XZSize");
    mHellScale = tag->getInt(L"HellScale");
    mModernEnd = tag->contains(L"ModernEnd") && tag->getBoolean(L"ModernEnd");

    int xzSize = mXZSize;
    int hellScale = mHellScale;
    if (xzSize > LEVEL_MAX_WIDTH)
    {
        xzSize = LEVEL_MAX_WIDTH;
    }
    if (xzSize < LEVEL_MIN_WIDTH)
    {
        xzSize = LEVEL_MIN_WIDTH;
    }
    if (hellScale > HELL_LEVEL_MAX_SCALE)
    {
        hellScale = HELL_LEVEL_MAX_SCALE;
    }
    if (hellScale < HELL_LEVEL_MIN_SCALE)
    {
        hellScale = HELL_LEVEL_MIN_SCALE;
    }
    mXZSize = xzSize;
    while (xzSize / hellScale > HELL_LEVEL_MAX_WIDTH && hellScale < HELL_LEVEL_MAX_SCALE)
    {
        ++hellScale;
    }
    mHellScale = hellScale;

    if (tag->contains(L"BiomeScale"))
    {
        mBiomeScale = tag->getInt(L"BiomeScale");
        if (mBiomeScale > 2)
            mBiomeScale = 0;
        mBiomeCentreXChunk = tag->getInt(L"BiomeCentreXChunk");
        mBiomeCentreZChunk = tag->getInt(L"BiomeCentreZChunk");
    }
    else
    {
        mBiomeScale = 0;
        mBiomeCentreXChunk = 0;
        mBiomeCentreZChunk = 0;
    }
    mLoadedPlayerTag = 0;
#ifdef MINECRAFT_PC
    PcModContentReadWorldIds(tag, &mModBlockIds);
#endif
}

LevelData::LevelData(LevelSettings* settings, const std::wstring& name)
    : mSeed(settings->getSeed()),
      mGeneratorType(settings->getLevelType()),
      mGeneratorOptions(settings->getLevelTypeOptions()),
      mXSpawn(0),
      mYSpawn(0),
      mZSpawn(0),
      mGameTime(-1),
      mDayTime(-1),
      mLastPlayed(0),
      mSizeOnDisk(0),
      mLoadedPlayerTag(0),
      mLevelName(name),
      mVersion(0),
      mClearWeatherTime(0),
      mRaining(false),
      mRainTime(0),
      mThundering(false),
      mThunderTime(0),
      mGameType(settings->getGameType()),
      mGenerateMapFeatures(settings->isGenerateMapFeatures()),
      mHardcore(settings->isHardcore()),
      mAllowCommands(settings->getAllowCommands()),
      mInitialized(false),
      mDifficulty(0),
      mDifficultyLocked(false),
      mCloudHeight(settings->getCloudHeight()),
      mUseNewSeaLevel(settings->useNewSeaLevel()),
      mHasBeenInCreative(settings->getGameType() == GameType::CREATIVE),
      mSpawnBonusChest(settings->hasStartingBonusItems()),
      mXZSize(0),
      mHellScale(0),
      mModernEnd(false),
      mSpawnXStronghold(0),
      mSpawnYStronghold(0),
      mSpawnZStronghold(0),
      mHasStronghold(false),
      mXStrongholdEndPortal(0),
      mZStrongholdEndPortal(0),
      mHasStrongholdEndPortal(false),
      mBiomeScale(0),
      mBiomeCentreXChunk(0),
      mBiomeCentreZChunk(0),
      mGameRules()
{
    int xzSize = settings->getXZSize();
    int hellScale = settings->getHellScale();
    bool modernEnd = settings->getModernEnd();

    if (xzSize > LEVEL_MAX_WIDTH)
    {
        xzSize = LEVEL_MAX_WIDTH;
    }
    if (xzSize < LEVEL_MIN_WIDTH)
    {
        xzSize = LEVEL_MIN_WIDTH;
    }
    if (hellScale > HELL_LEVEL_MAX_SCALE)
    {
        hellScale = HELL_LEVEL_MAX_SCALE;
    }
    if (hellScale < HELL_LEVEL_MIN_SCALE)
    {
        hellScale = HELL_LEVEL_MIN_SCALE;
    }
    mXZSize = xzSize;
    while (xzSize / hellScale > HELL_LEVEL_MAX_WIDTH && hellScale < HELL_LEVEL_MAX_SCALE)
    {
        ++hellScale;
    }
    mHellScale = hellScale;
    mModernEnd = modernEnd;

    mBiomeScale = settings->getBiomeScale();
    mBiomeCentreXChunk = settings->getBiomeCentreXChunk();
    mBiomeCentreZChunk = settings->getBiomeCentreZChunk();
}

LevelData::LevelData(LevelData* other)
{
    mSeed = other->mSeed;
    mGeneratorType = other->mGeneratorType;
    mGeneratorOptions = other->mGeneratorOptions;
    mXSpawn = other->mXSpawn;
    mYSpawn = other->mYSpawn;
    mZSpawn = other->mZSpawn;
    mGameTime = other->mGameTime;
    mDayTime = other->mDayTime;
    mLastPlayed = other->mLastPlayed;
    mSizeOnDisk = other->mSizeOnDisk;
    mLoadedPlayerTag = other->mLoadedPlayerTag;
    mLevelName = other->mLevelName;
    mVersion = other->mVersion;
    mClearWeatherTime = other->mClearWeatherTime;
    mRaining = other->mRaining;
    mRainTime = other->mRainTime;
    mThundering = other->mThundering;
    mThunderTime = other->mThunderTime;
    mGameType = other->mGameType;
    mGenerateMapFeatures = other->mGenerateMapFeatures;
    mHardcore = other->mHardcore;
    mAllowCommands = other->mAllowCommands;
    mInitialized = other->mInitialized;
    mDifficulty = other->mDifficulty;
    mDifficultyLocked = other->mDifficultyLocked;
    mCloudHeight = other->mCloudHeight;
    mUseNewSeaLevel = other->mUseNewSeaLevel;
    mHasBeenInCreative = other->mHasBeenInCreative;
    mSpawnBonusChest = other->mSpawnBonusChest;
    mXZSize = other->mXZSize;
    mHellScale = other->mHellScale;
    mModernEnd = other->mModernEnd;
    mSpawnXStronghold = other->mSpawnXStronghold;
    mSpawnYStronghold = other->mSpawnYStronghold;
    mSpawnZStronghold = other->mSpawnZStronghold;
    mHasStronghold = other->mHasStronghold;
    mXStrongholdEndPortal = other->mXStrongholdEndPortal;
    mZStrongholdEndPortal = other->mZStrongholdEndPortal;
    mHasStrongholdEndPortal = other->mHasStrongholdEndPortal;
    mBiomeScale = other->mBiomeScale;
    mBiomeCentreXChunk = other->mBiomeCentreXChunk;
    mBiomeCentreZChunk = other->mBiomeCentreZChunk;
#ifdef MINECRAFT_PC
    mModBlockIds = other->mModBlockIds;
#endif
}

LevelData::~LevelData()
{
    for (boost::unordered_map<const DimensionType*, CompoundTag*>::iterator it = mDimensionData.begin();
         it != mDimensionData.end(); ++it)
    {
        if (it->second != 0)
        {
            delete it->second;
        }
    }
}

CompoundTag* LevelData::createTag()
{
    CompoundTag* tag = new CompoundTag();
    setTagData(tag);
    return tag;
}

CompoundTag* LevelData::createTag(std::vector<boost::shared_ptr<Player> >* players)
{
    return createTag();
}

void LevelData::addWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_Level,
                     new TagUpdateWalker(L"Player", 10, DataFixTypes::eDataFixTypes_Player));
}

void LevelData::setTagData(CompoundTag* tag)
{
    tag->putInt(SharedConstants::DATA_VERSION_TAG, 922);
    tag->putLong(L"RandomSeed", mSeed);
#ifdef MINECRAFT_PC
    PcModContentWriteWorldIds(tag, mModBlockIds);
#endif
    tag->putString(L"generatorName", mGeneratorType->getGeneratorName());
    tag->putInt(L"generatorVersion", mGeneratorType->getVersion());

    if (mGeneratorOptions != 0)
    {
        mGeneratorOptions->setVersion(5);
        tag->put(L"generatorOptions", mGeneratorOptions->toTag());
    }

    tag->putInt(L"GameType", mGameType->getId());
    tag->putBoolean(L"MapFeatures", mGenerateMapFeatures);
    tag->putBoolean(L"spawnBonusChest", mSpawnBonusChest);
    tag->putInt(L"SpawnX", mXSpawn);
    tag->putInt(L"SpawnY", mYSpawn);
    tag->putInt(L"SpawnZ", mZSpawn);
    tag->putLong(L"Time", mGameTime);
    tag->putLong(L"DayTime", mDayTime);
    tag->putLong(L"SizeOnDisk", mSizeOnDisk);
    tag->putLong(L"LastPlayed", System::processTimeInMilliSecs());
    tag->putString(L"LevelName", mLevelName);
    tag->putInt(L"version", mVersion);
    tag->putInt(L"clearWeatherTime", mClearWeatherTime);
    tag->putInt(L"rainTime", mRainTime);
    tag->putBoolean(L"raining", mRaining);
    tag->putInt(L"thunderTime", mThunderTime);
    tag->putBoolean(L"thundering", mThundering);
    tag->putBoolean(L"hardcore", mHardcore);
    tag->putBoolean(L"allowCommands", mAllowCommands);
    tag->putBoolean(L"initialized", mInitialized);

    if (mDifficulty != 0)
    {
        tag->putByte(L"Difficulty", static_cast<unsigned char>(mDifficulty->getId()));
    }

    if (mCloudHeight != 128)
    {
        tag->putInt(L"cloudHeight", mCloudHeight);
    }

    tag->putBoolean(L"DifficultyLocked", mDifficultyLocked);

    CompoundTag* dimensionData = new CompoundTag();
    if (!mDimensionData.empty())
    {
        for (boost::unordered_map<const DimensionType*, CompoundTag*>::iterator it = mDimensionData.begin();
             it != mDimensionData.end(); ++it)
        {
            dimensionData->put(it->first->getName(), it->second->copy());
        }
    }
    tag->put(L"DimensionData", dimensionData);

    tag->putBoolean(L"newSeaLevel", mUseNewSeaLevel);
    tag->putBoolean(L"hasBeenInCreative", mHasBeenInCreative);
    tag->putBoolean(L"hasStronghold", mHasStronghold);
    tag->putInt(L"StrongholdX", mSpawnXStronghold);
    tag->putInt(L"StrongholdY", mSpawnYStronghold);
    tag->putInt(L"StrongholdZ", mSpawnZStronghold);
    tag->putBoolean(L"hasStrongholdEndPortal", mHasStrongholdEndPortal);
    tag->putInt(L"StrongholdEndPortalX", mXStrongholdEndPortal);
    tag->putInt(L"StrongholdEndPortalZ", mZStrongholdEndPortal);
    tag->putInt(L"XZSize", mXZSize);
    tag->putInt(L"HellScale", mHellScale);
    tag->putBoolean(L"ModernEnd", mModernEnd);
    tag->putInt(L"BiomeScale", mBiomeScale);
    tag->putInt(L"BiomeCentreXChunk", mBiomeCentreXChunk);
    tag->putInt(L"BiomeCentreZChunk", mBiomeCentreZChunk);
}

long long LevelData::getSeed()
{
    return mSeed;
}
int LevelData::getXSpawn()
{
    return mXSpawn;
}
int LevelData::getYSpawn()
{
    return mYSpawn;
}
int LevelData::getZSpawn()
{
    return mZSpawn;
}
int LevelData::getXStronghold()
{
    return mSpawnXStronghold;
}
int LevelData::getZStronghold()
{
    return mSpawnZStronghold;
}
int LevelData::getXStrongholdEndPortal()
{
    return mXStrongholdEndPortal;
}
int LevelData::getZStrongholdEndPortal()
{
    return mZStrongholdEndPortal;
}
long long LevelData::getGameTime()
{
    return mGameTime;
}
long long LevelData::getDayTime()
{
    return mDayTime;
}
long long LevelData::getSizeOnDisk()
{
    return mSizeOnDisk;
}
CompoundTag* LevelData::getLoadedPlayerTag()
{
    return 0;
}
void LevelData::setSeed(long long value)
{
    mSeed = value;
}
void LevelData::setXSpawn(int value)
{
    mXSpawn = value;
}
void LevelData::setYSpawn(int value)
{
    mYSpawn = value;
}
void LevelData::setZSpawn(int value)
{
    mZSpawn = value;
}
void LevelData::setHasStronghold()
{
    mHasStronghold = true;
}
bool LevelData::getHasStronghold()
{
    return mHasStronghold;
}
void LevelData::setXStronghold(int value)
{
    mSpawnXStronghold = value;
}
void LevelData::setZStronghold(int value)
{
    mSpawnZStronghold = value;
}
void LevelData::setHasStrongholdEndPortal()
{
    mHasStrongholdEndPortal = true;
}
bool LevelData::getHasStrongholdEndPortal()
{
    return mHasStrongholdEndPortal;
}
void LevelData::setXStrongholdEndPortal(int value)
{
    mXStrongholdEndPortal = value;
}
void LevelData::setZStrongholdEndPortal(int value)
{
    mZStrongholdEndPortal = value;
}
void LevelData::setGameTime(long long value)
{
    mGameTime = value;
}
void LevelData::setDayTime(long long value)
{
    mDayTime = value;
}
void LevelData::setSizeOnDisk(long long value)
{
    mSizeOnDisk = value;
}
void LevelData::setLoadedPlayerTag(CompoundTag* tag)
{
}
void LevelData::setSpawn(const BlockPos& pos)
{
    mXSpawn = pos.x;
    mYSpawn = pos.y;
    mZSpawn = pos.z;
}
std::wstring LevelData::getLevelName()
{
    return mLevelName;
}
void LevelData::setLevelName(const std::wstring& name)
{
    mLevelName = name;
}
int LevelData::getVersion()
{
    return mVersion;
}
void LevelData::setVersion(int value)
{
    mVersion = value;
}
long long LevelData::getLastPlayed()
{
    return mLastPlayed;
}
int LevelData::getClearWeatherTime()
{
    return mClearWeatherTime;
}
void LevelData::setClearWeatherTime(int value)
{
    mClearWeatherTime = value;
}
bool LevelData::isThundering()
{
    return mThundering;
}
void LevelData::setThundering(bool value)
{
    mThundering = value;
}
int LevelData::getThunderTime()
{
    return mThunderTime;
}
void LevelData::setThunderTime(int value)
{
    mThunderTime = value;
}
bool LevelData::isRaining()
{
    return mRaining;
}
void LevelData::setRaining(bool value)
{
    mRaining = value;
}
int LevelData::getRainTime()
{
    return mRainTime;
}
void LevelData::setRainTime(int value)
{
    mRainTime = value;
}
const GameType* LevelData::getGameType()
{
    return mGameType;
}
bool LevelData::isGenerateMapFeatures()
{
    return mGenerateMapFeatures;
}
void LevelData::setGenerateMapFeatures(bool value)
{
    mGenerateMapFeatures = value;
}
bool LevelData::getSpawnBonusChest()
{
    return mSpawnBonusChest;
}
void LevelData::setGameType(const GameType* type)
{
    mGameType = type;
    bool hasBeenInCreative = true;
    if (!mHasBeenInCreative && type != GameType::CREATIVE && gApp.GetGameHostOption(eGameHostOption_CheatsEnabled) == 0)
    {
        hasBeenInCreative = false;
    }
    mHasBeenInCreative = hasBeenInCreative;
}
bool LevelData::useNewSeaLevel()
{
    return mUseNewSeaLevel;
}
bool LevelData::getHasBeenInCreative()
{
    return mHasBeenInCreative;
}
void LevelData::setHasBeenInCreative(bool value)
{
    mHasBeenInCreative = value;
}
LevelType* LevelData::getGeneratorType()
{
    return mGeneratorType;
}
void LevelData::setGeneratorType(LevelType* type)
{
    mHasBeenInCreative = mHasBeenInCreative || type == LevelType::FLAT;
    mGeneratorType = type;
}
SuperflatConfig* LevelData::getGeneratorOptions()
{
    return mGeneratorOptions;
}
void LevelData::setGeneratorOptions(SuperflatConfig* options)
{
    mGeneratorOptions = options;
}
bool LevelData::isHardcore()
{
    return mHardcore;
}
void LevelData::setHardcore(bool value)
{
    mHardcore = value;
}
bool LevelData::getAllowCommands()
{
    return mAllowCommands;
}
void LevelData::setAllowCommands(bool value)
{
    mAllowCommands = value;
}
bool LevelData::isInitialized()
{
    return mInitialized;
}
void LevelData::setInitialized(bool value)
{
    mInitialized = value;
}
GameRules* LevelData::getGameRules()
{
    return &mGameRules;
}
const Difficulty* LevelData::getDifficulty()
{
    return mDifficulty;
}
void LevelData::setDifficulty(const Difficulty* difficulty)
{
    mDifficulty = difficulty;
    gApp.SetGameHostOption(eGameHostOption_Difficulty, difficulty->getId());
}
int LevelData::getCloudHeight()
{
    return mCloudHeight;
}
void LevelData::setCloudHeight(int value)
{
    mCloudHeight = value;
}
bool LevelData::isDifficultyLocked()
{
    return mDifficultyLocked;
}
void LevelData::setDifficultyLocked(bool value)
{
    mDifficultyLocked = value;
}
CompoundTag* LevelData::getDimensionData(const DimensionType* dimension)
{
    boost::unordered_map<const DimensionType*, CompoundTag*>::iterator it = mDimensionData.find(dimension);
    if (it == mDimensionData.end())
        return new CompoundTag();
    return it->second;
}
void LevelData::setDimensionData(const DimensionType* dimension, CompoundTag* tag)
{
    CompoundTag* existing = mDimensionData[dimension];
    if (existing != 0)
    {
        delete existing;
    }
    mDimensionData[dimension] = tag;
}
int LevelData::getXZSize()
{
    return mXZSize;
}
int LevelData::getHellScale()
{
    return mHellScale;
}
void LevelData::setModernEnd()
{
    mModernEnd = true;
}
bool LevelData::getModernEnd()
{
    return mModernEnd;
}
unsigned char LevelData::getBiomeScale()
{
    return mBiomeScale;
}
int LevelData::getBiomeCentreXChunk()
{
    return mBiomeCentreXChunk;
}
int LevelData::getBiomeCentreZChunk()
{
    return mBiomeCentreZChunk;
}

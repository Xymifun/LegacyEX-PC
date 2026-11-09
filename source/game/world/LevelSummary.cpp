#include "game/world/LevelSummary.hpp"

#include "game/world/LevelData.hpp"

LevelSummary::LevelSummary(LevelData* levelData, const std::wstring& levelId, const std::wstring& levelName,
                           long long sizeOnDisk, bool requiresConversion)
{
    mLevelId = levelId;
    mLevelName = levelName;
    mLastPlayed = levelData->getLastPlayed();
    mSizeOnDisk = sizeOnDisk;
    mGameMode = levelData->getGameType();
    mRequiresConversion = requiresConversion;
    mHardcore = levelData->isHardcore();
    mCheats = levelData->getAllowCommands();
}

std::wstring LevelSummary::getLevelId()
{
    return mLevelId;
}

std::wstring LevelSummary::getLevelName()
{
    return mLevelName;
}

long long LevelSummary::getSizeOnDisk()
{
    return mSizeOnDisk;
}

long long LevelSummary::getLastPlayed()
{
    return mLastPlayed;
}

bool LevelSummary::isRequiresConversion()
{
    return mRequiresConversion;
}

const GameType* LevelSummary::getGameMode()
{
    return mGameMode;
}

bool LevelSummary::isHardcore()
{
    return mHardcore;
}

bool LevelSummary::hasCheats()
{
    return mCheats;
}

int LevelSummary::compareTo(LevelSummary* other)
{
    if (mHardcore && !other->mHardcore)
        return -1;
    if (!mHardcore && other->mHardcore)
        return 1;
    if (mCheats && !other->mCheats)
        return -1;
    if (!mCheats && other->mCheats)
        return 1;
    if (mLastPlayed < other->mLastPlayed)
        return 1;
    if (mLastPlayed > other->mLastPlayed)
        return -1;
    return mLevelId.compare(other->mLevelId);
}

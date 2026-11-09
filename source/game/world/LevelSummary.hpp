#ifndef GAME_WORLD_LEVELSUMMARY_HPP
#define GAME_WORLD_LEVELSUMMARY_HPP

#include <string>

class GameType;
class LevelData;

class LevelSummary
{
public:
    LevelSummary(LevelData* levelData, const std::wstring& levelId, const std::wstring& levelName, long long sizeOnDisk,
                 bool requiresConversion);

    std::wstring getLevelId();
    std::wstring getLevelName();

    long long getSizeOnDisk();
    long long getLastPlayed();

    bool isRequiresConversion();

    const GameType* getGameMode();
    bool isHardcore();
    bool hasCheats();

    int compareTo(LevelSummary* other);

private:
    std::wstring mLevelId;
    std::wstring mLevelName;
    long long mLastPlayed;
    long long mSizeOnDisk;
    bool mRequiresConversion;
    const GameType* mGameMode;
    bool mHardcore;
    bool mCheats;
};

#endif

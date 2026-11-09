#ifndef GAME_CORE_CACHIEVEMENTS_HPP
#define GAME_CORE_CACHIEVEMENTS_HPP

#include <set>

#include "game/world/generation/PlayerUID.hpp"

class CProfile;

class CAchievements
{
public:
    CAchievements(int iQuad);

    bool IsIdle() const;
    bool StartRead();
    void printAchievements() const;
    bool IsDirty() const;
    bool SetAwardedBit(int achievementId, bool awarded);
    bool GetAwardedBit(int achievementId) const;
    bool hasPendingAwards() const;
    void addPendingAward(int achievementId);
    void awardPending(std::set<int>& awardsNotRegistered);
    unsigned int GetState() const;
    const char* GetFilename() const;
    void WriteTo(void* data, int size);
    void ReadFrom(void* data, int size);
    bool StartWrite();

private:
    friend class CProfile;

    bool mIsSystemPlayer;
    bool mHasAchievementsFile;
    unsigned char mSlotId;
    PlayerUID mPlayerUID;
    bool mDirty;
    unsigned char mAwarded[16];
    std::set<int> mPendingAwards;
    unsigned int mState;
    char mFilename[128];
};

#endif

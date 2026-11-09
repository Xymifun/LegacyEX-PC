#include "game/core/CAchievements.hpp"

#include <cstdio>
#include <cstring>
#include <string>

#include "platform/wiiu/CProfile.hpp"

int getSlotId(int iQuad);
PlayerUID getPlayerUID(int iQuad, bool includeOffline);

CAchievements::CAchievements(int iQuad)
    : mIsSystemPlayer(getSlotId(iQuad) != 0),
      mSlotId((unsigned char)getSlotId(iQuad)),
      mPlayerUID(getPlayerUID(iQuad, false)),
      mDirty(false)
{
    std::memset(mAwarded, 0, sizeof(mAwarded));
    mState = 0;
    std::memset(mFilename, 0, sizeof(mFilename));
    mHasAchievementsFile = false;

    if (mIsSystemPlayer)
    {
        std::wstring uid = mPlayerUID.toString();
        gProfileInstance->DebugPrintf("<CAchievements> Creating achievements for '%ls', iQuad=%i, slot=%i.\n",
                                      uid.c_str(), iQuad, mSlotId);
        std::sprintf(mFilename, "ach_%ls", uid.c_str());
        StartRead();
    }
    else
    {
        PlayerUID playerUID = getPlayerUID(iQuad, false);
        const char* guest = (playerUID.flags & 0x20000000u) != 0 ? "True" : "False";
        gProfileInstance->DebugPrintf("<CAchievements> Non-system player quad %i, guest=%s.\n", iQuad, guest);
    }
}

bool CAchievements::IsIdle() const
{
    return mState == 0;
}

bool CAchievements::StartRead()
{
    if (IsIdle())
    {
        if (!mIsSystemPlayer)
        {
            return true;
        }
        mState = 1;
        gProfileInstance->AddAwardManagerTask(this);
        return true;
    }
    return false;
}

void CAchievements::printAchievements() const
{
    gProfileInstance->DebugPrintf("<CAwardManager> Achievements for %ls.", mPlayerUID.toString().c_str());
    for (int i = 0; i < 128; i += 8)
    {
        gProfileInstance->DebugPrintf("\n0x%03x| ", i);
        for (int j = 0; j < 8; ++j)
        {
            int n = i + j;
            gProfileInstance->DebugPrintf((mAwarded[n >> 3] & (1 << (n & 7))) ? "1" : "0");
        }
    }
    gProfileInstance->DebugPrintf("\n");
}

bool CAchievements::IsDirty() const
{
    return mDirty;
}

bool CAchievements::SetAwardedBit(int achievementId, bool awarded)
{
    int registrationId = gProfileInstance->GetRegistrationId(achievementId);
    if (registrationId >= 0)
    {
        unsigned char& value = mAwarded[registrationId >> 3];
        unsigned char mask = (unsigned char)(1 << (registrationId & 7));
        bool wasAwarded = (mask & value) != 0;
        if (awarded)
        {
            value |= mask;
        }
        else
        {
            value &= ~mask;
        }
        bool dirty = true;
        if (!mDirty && wasAwarded == awarded)
        {
            dirty = false;
        }
        mDirty = dirty;
        return wasAwarded;
    }
    return false;
}

bool CAchievements::GetAwardedBit(int achievementId) const
{
    int registrationId = gProfileInstance->GetRegistrationId(achievementId);
    bool awarded = false;
    if (registrationId >= 0)
    {
        unsigned char mask = (unsigned char)(1 << (registrationId & 7));
        awarded = (mask & mAwarded[registrationId >> 3]) != 0;
    }
    return awarded;
}

bool CAchievements::hasPendingAwards() const
{
    return !mPendingAwards.empty();
}

void CAchievements::addPendingAward(int achievementId)
{
    mPendingAwards.insert(achievementId);
}

void CAchievements::awardPending(std::set<int>& awardsNotRegistered)
{
    if (!hasPendingAwards())
    {
        return;
    }

    std::set<int> pendingAwards = mPendingAwards;
    mPendingAwards.clear();
    for (std::set<int>::iterator it = pendingAwards.begin(); it != pendingAwards.end(); ++it)
    {
        if (!SetAwardedBit(*it, true))
        {
            awardsNotRegistered.insert(*it);
        }
    }
}

unsigned int CAchievements::GetState() const
{
    return mState;
}

const char* CAchievements::GetFilename() const
{
    return mFilename;
}

void CAchievements::WriteTo(void* data, int size)
{
    if (data != NULL)
    {
        std::memcpy(data, mAwarded, 16);
        gProfileInstance->DebugPrintf("<CAwardManager> Achievement file written.\n");
        printAchievements();
    }
    mState = 0;
    mDirty = false;
}

void CAchievements::ReadFrom(void* data, int size)
{
    if (data != NULL)
    {
        std::memcpy(mAwarded, data, 16);
        gProfileInstance->DebugPrintf("<CAwardManager> Achievement file read.\n");
        printAchievements();
    }
    mState = 0;
    mDirty = false;
}

bool CAchievements::StartWrite()
{
    bool result = IsIdle();
    if (result && mDirty)
    {
        if (!mIsSystemPlayer)
        {
            mDirty = false;
            return true;
        }
        mState = 2;
        gProfileInstance->AddAwardManagerTask(this);
        return true;
    }
    return result;
}

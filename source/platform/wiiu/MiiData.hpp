#ifndef PLATFORM_WIIU_MIIDATA_HPP
#define PLATFORM_WIIU_MIIDATA_HPP

#include <vector>

#include "game/util/ACTUuid.hpp"

class CProfile;

struct MiiDataEntry
{
    int mIconTexture;
    void* mIconMemory;
    unsigned short mName[11];
    unsigned char mCreatorId[10];
    bool mIsAccountMii;
    bool mIsGuestMii;
    ACTUuid mUuid;
};

class MiiData
{
public:
    void LoadAllData();
    void FreeAllData();

private:
    friend class CProfile;
    void CreateSysUserData(int slot);
    void CreateGuestData(int index);
    void CreateMiiMakerData(int index);

    std::vector<MiiDataEntry> mSysUserMiis;
    std::vector<MiiDataEntry> mGuestMiis;
    std::vector<MiiDataEntry> mMiiMakerMiis;
};

#endif

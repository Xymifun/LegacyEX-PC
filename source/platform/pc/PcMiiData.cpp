#include "platform/wiiu/MiiData.hpp"

#include "platform/pc/PcAccount.hpp"

#include <stdio.h>
#include <string.h>
#include <string>

namespace
{
const int GUEST_COUNT = 3;
const int NAME_LENGTH = 10;

void SetName(MiiDataEntry& e, const std::string& name)
{
    memset(e.mName, 0, sizeof(e.mName));
    for (size_t i = 0; i < name.size() && i < (size_t)NAME_LENGTH; ++i)
        e.mName[i] = (unsigned char)name[i];
}

MiiDataEntry MakeEntry(const std::string& name, bool account, bool guest)
{
    MiiDataEntry e;
    e.mIconTexture = -1;
    e.mIconMemory = 0;
    memset(e.mCreatorId, 0, sizeof(e.mCreatorId));
    memset(e.mUuid.data, 0, sizeof(e.mUuid.data));
    e.mIsAccountMii = account;
    e.mIsGuestMii = guest;
    SetName(e, name);
    return e;
}
}

void MiiData::CreateSysUserData(int)
{
    MiiDataEntry e = MakeEntry(PcPlayerName(), true, false);
    PcAccountUuid(e.mUuid.data);
    mSysUserMiis.push_back(e);
}

void MiiData::CreateGuestData(int i)
{
    MiiDataEntry e = MakeEntry(PcGuestName(i + 1), false, true);
    e.mCreatorId[0] = 0x80;
    e.mCreatorId[9] = (unsigned char)(i + 2);
    PcGuestAccountUuid(i + 1, e.mUuid.data);
    mGuestMiis.push_back(e);
}

void MiiData::CreateMiiMakerData(int)
{
}

void MiiData::LoadAllData()
{
    CreateSysUserData(1);
    for (int i = 0; i < GUEST_COUNT; ++i)
        CreateGuestData(i);
}

void MiiData::FreeAllData()
{
    mSysUserMiis.clear();
    mGuestMiis.clear();
    mMiiMakerMiis.clear();
}

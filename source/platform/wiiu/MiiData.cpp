#include "platform/wiiu/MiiData.hpp"

#include "game/client/render/Renderer.hpp"

#include <string.h>
#include <nn/act/client_cpp.h>
#include <nn/ffl.h>

static FFLResult sAdditionalInfoResult;

void MiiData::CreateSysUserData(int slot)
{
    FFLStoreData storeData;
    FFLAdditionalInfo additionalInfo;
    MiiDataEntry entry;

    nn::act::GetMiiEx(&storeData, slot);

    sAdditionalInfoResult = FFLGetAdditionalInfo(&additionalInfo, FFL_DATA_SOURCE_STORE_DATA, &storeData, 0, TRUE);

    *reinterpret_cast<FFLCreateID*>(entry.mCreatorId) = additionalInfo.creatorID;
    entry.mIsAccountMii = true;
    entry.mIsGuestMii = false;

    nn::act::GetUuidEx(reinterpret_cast<char*>(&entry.mUuid), slot);

    memcpy(entry.mName, additionalInfo.name, sizeof(entry.mName));

    entry.mIconMemory = (void*)gRendererInstance->CreateMiiIconTexture(FFL_DATA_SOURCE_STORE_DATA, &storeData, 0,
                                                                       &entry.mIconTexture);

    mSysUserMiis.push_back(entry);
}

void MiiData::CreateGuestData(int index)
{
    FFLAdditionalInfo additionalInfo;
    MiiDataEntry entry;

    FFLGetAdditionalInfo(&additionalInfo, FFL_DATA_SOURCE_DEFAULT, NULL, index, TRUE);

    *reinterpret_cast<FFLCreateID*>(entry.mCreatorId) = additionalInfo.creatorID;
    entry.mIsAccountMii = false;
    entry.mIsGuestMii = true;

    memcpy(entry.mName, additionalInfo.name, sizeof(entry.mName));

    entry.mIconMemory = (void*)gRendererInstance->CreateMiiIconTexture(FFL_DATA_SOURCE_DEFAULT, NULL, index,
                                                                       &entry.mIconTexture);

    mGuestMiis.push_back(entry);
}

void MiiData::CreateMiiMakerData(int index)
{
    FFLAdditionalInfo additionalInfo;
    MiiDataEntry entry;

    FFLGetAdditionalInfo(&additionalInfo, FFL_DATA_SOURCE_OFFICIAL, NULL, index, TRUE);

    *reinterpret_cast<FFLCreateID*>(entry.mCreatorId) = additionalInfo.creatorID;
    entry.mIsAccountMii = false;
    entry.mIsGuestMii = false;

    memcpy(entry.mName, additionalInfo.name, sizeof(entry.mName));

    entry.mIconMemory = (void*)gRendererInstance->CreateMiiIconTexture(FFL_DATA_SOURCE_OFFICIAL, NULL, index,
                                                                       &entry.mIconTexture);

    mMiiMakerMiis.push_back(entry);
}

void MiiData::LoadAllData()
{
    if (!gRendererInstance->InitFFL())
    {
        return;
    }

    int accountCount = nn::act::GetNumOfAccounts();

    for (int i = 0; i < accountCount; ++i)
    {
        int slot = i + 1;
        if (nn::act::IsSlotOccupied(slot))
        {
            CreateSysUserData(slot);
        }
    }

    int officialDataCount = FFLGetAvailableOfficialDataNum();
    int foundCount = 0;

    for (int index = 0; index < 3000; ++index)
    {
        if (FFLIsAvailableOfficialData(index))
        {
            if (FFLIsRegularOfficialData(index))
            {
                CreateMiiMakerData(index);
            }

            ++foundCount;

            if (foundCount >= officialDataCount)
            {
                break;
            }
        }
    }

    for (int guestIndex = 0; guestIndex < 6; ++guestIndex)
    {
        CreateGuestData(guestIndex);
    }

    gRendererInstance->ShutdownFFL();
}

void MiiData::FreeAllData()
{
    for (int i = 0; i < mSysUserMiis.size(); ++i)
    {
        gRendererInstance->FreeMiiIconTexture(mSysUserMiis[i].mIconTexture, mSysUserMiis[i].mIconMemory);
    }

    mSysUserMiis.clear();

    for (int i = 0; i < mGuestMiis.size(); ++i)
    {
        gRendererInstance->FreeMiiIconTexture(mGuestMiis[i].mIconTexture, mGuestMiis[i].mIconMemory);
    }

    mGuestMiis.clear();

    for (int i = 0; i < mMiiMakerMiis.size(); ++i)
    {
        gRendererInstance->FreeMiiIconTexture(mMiiMakerMiis[i].mIconTexture, mMiiMakerMiis[i].mIconMemory);
    }

    mMiiMakerMiis.clear();
}

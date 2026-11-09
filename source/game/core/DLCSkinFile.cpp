#include "game/core/DLCSkinFile.hpp"

#include <cstdio>
#include <cstring>
#include <cwchar>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ChinaDlcFilter.hpp"
#include "game/core/DLCLocalisationFile.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/StringUtils.hpp"
#include "game/data/StringTable.hpp"
#include "vendor/SKIN_BOX.hpp"

extern CConsoleMinecraftApp gApp;

DLCSkinFile::DLCSkinFile(const std::wstring& path)
    : DLCFile(DLCManager::eDLCType_Skin, path),
      mSkinAdjustments(),
      mAdditionalBoxes()
{
    mDisplayName = L"";
    mDisplayNameID = L"";
    mThemeName = L"";
    mLocKeySet = false;
    mLocKey = L"";
    mThemeNameID = L"";
    mCapePath = L"";
    mFree = false;
}

bool DLCSkinFile::isFiltered()
{
    return ChinaDlcFilter::instance()->isSkinFiltered(m_path);
}

#ifdef MINECRAFT_PC
static const unsigned int kPcSkin64x64AnimFlags = 0x40000 | 0x80000;

static unsigned int PcPngWord(const unsigned char* data)
{
    return ((unsigned int)data[0] << 24) | ((unsigned int)data[1] << 16) | ((unsigned int)data[2] << 8) | data[3];
}
#endif

void DLCSkinFile::addData(unsigned char* data, unsigned int size)
{
#ifdef MINECRAFT_PC
    if (size >= 24 && memcmp(data, "\x89PNG\r\n\x1a\n", 8) == 0 && memcmp(data + 12, "IHDR", 4) == 0
        && PcPngWord(data + 16) == 2 * PcPngWord(data + 20)
        && (mSkinAdjustments.mAnimFlags & kPcSkin64x64AnimFlags) != 0)
    {
        printf("[pc] dlc: %ls is %ux%u but its ANIM 0x%X marks a 64x64 skin; drawn as %ux%u\n", m_path.c_str(),
               PcPngWord(data + 16), PcPngWord(data + 20), mSkinAdjustments.mAnimFlags, PcPngWord(data + 16),
               PcPngWord(data + 20));
        mSkinAdjustments.mAnimFlags &= ~kPcSkin64x64AnimFlags;
        gApp.SetSkinAdjustments(CMinecraftApp::getSkinIdFromPath(m_path), mSkinAdjustments);
    }
#endif
    gApp.AddMemoryTextureFile(m_path, data, size);
}

void DLCSkinFile::addParameter(DLCManager::EDLCParameterType parameter, const std::wstring& value)
{
    switch (parameter)
    {
    case DLCManager::eDLCParameterType_DisplayNameID:
        mDisplayNameID = value;
        break;
    case DLCManager::eDLCParameterType_ThemeNameID:
        mThemeNameID = value;
        break;
    case DLCManager::eDLCParameterType_LocKey:
        mLocKey = value;
        mLocKeySet = true;
        break;
    case DLCManager::eDLCParameterType_DisplayName:
        mDisplayName = value;
        break;
    case DLCManager::eDLCParameterType_ThemeName:
        mThemeName = value;
        break;
    case DLCManager::eDLCParameterType_Free:
        mFree = true;
        break;
    case DLCManager::eDLCParameterType_CreditID:
        m_pPack->AddCreditID(value);
        break;
    case DLCManager::eDLCParameterType_CapePath:
        mCapePath = value;
        break;
    case DLCManager::eDLCParameterType_Box:
    {
        wchar_t partName[10];
        SKIN_BOX* box = new SKIN_BOX;
        memset(box, 0, sizeof(SKIN_BOX));
        swscanf(value.c_str(), L"%10ls%f%f%f%f%f%f%f%f%d%d%f", partName, &box->x, &box->y, &box->z, &box->width,
                &box->height, &box->depth, &box->xTexOffs, &box->yTexOffs, &box->armourFlags, &box->mirror,
                &box->delta);
        if (wcscmp(partName, L"HEAD") == 0)
            box->id = 1;
        else if (wcscmp(partName, L"BODY") == 0)
            box->id = 2;
        else if (wcscmp(partName, L"ARM0") == 0)
            box->id = 3;
        else if (wcscmp(partName, L"ARM1") == 0)
            box->id = 4;
        else if (wcscmp(partName, L"LEG0") == 0)
            box->id = 5;
        else if (wcscmp(partName, L"LEG1") == 0)
            box->id = 6;
        else if (wcscmp(partName, L"HEADWEAR") == 0)
            box->id = 7;
        else if (wcscmp(partName, L"JACKET") == 0)
            box->id = 8;
        else if (wcscmp(partName, L"SLEEVE0") == 0)
            box->id = 9;
        else if (wcscmp(partName, L"SLEEVE1") == 0)
            box->id = 10;
        else if (wcscmp(partName, L"PANTS0") == 0)
            box->id = 11;
        else if (wcscmp(partName, L"PANTS1") == 0)
            box->id = 12;
        else if (wcscmp(partName, L"WAIST") == 0)
            box->id = 13;
        else if (wcscmp(partName, L"LEGGING0") == 0)
            box->id = 14;
        else if (wcscmp(partName, L"LEGGING1") == 0)
            box->id = 15;
        else if (wcscmp(partName, L"SOCK0") == 0)
            box->id = 16;
        else if (wcscmp(partName, L"SOCK1") == 0)
            box->id = 17;
        else if (wcscmp(partName, L"BOOT0") == 0)
            box->id = 18;
        else if (wcscmp(partName, L"BOOT1") == 0)
            box->id = 19;
        else if (wcscmp(partName, L"ARMARMOR1") == 0)
            box->id = 20;
        else if (wcscmp(partName, L"ARMARMOR0") == 0)
            box->id = 21;
        else if (wcscmp(partName, L"BODYARMOR") == 0)
            box->id = 22;
        else if (wcscmp(partName, L"BELT") == 0)
            box->id = 23;
        mAdditionalBoxes.push_back(box);
    }
    break;
    case DLCManager::eDLCParameterType_Offset:
    {
        float offset;
        wchar_t partName[10];
        swscanf(value.c_str(), L"%10ls Y %f", partName, &offset);
        if (wcscmp(partName, L"HEAD") == 0)
            mSkinAdjustments.mHeadY = offset;
        else if (wcscmp(partName, L"BODY") == 0)
            mSkinAdjustments.mBodyY = offset;
        else if (wcscmp(partName, L"ARM0") == 0)
            mSkinAdjustments.mArm0Y = offset;
        else if (wcscmp(partName, L"ARM1") == 0)
            mSkinAdjustments.mArm1Y = offset;
        else if (wcscmp(partName, L"LEG0") == 0)
            mSkinAdjustments.mLeg0Y = offset;
        else if (wcscmp(partName, L"LEG1") == 0)
            mSkinAdjustments.mLeg1Y = offset;
        else if (wcscmp(partName, L"TOOL0") == 0)
            mSkinAdjustments.mTool0Y = offset;
        else if (wcscmp(partName, L"TOOL1") == 0)
            mSkinAdjustments.mTool1Y = offset;
        else if (wcscmp(partName, L"HELMET") == 0)
            mSkinAdjustments.mHelmetY = offset;
        else if (wcscmp(partName, L"SHOULDER0") == 0)
            mSkinAdjustments.mShoulder0Y = offset;
        else if (wcscmp(partName, L"SHOULDER1") == 0)
            mSkinAdjustments.mShoulder1Y = offset;
        else if (wcscmp(partName, L"CHEST") == 0)
            mSkinAdjustments.mChestY = offset;
        else if (wcscmp(partName, L"WAIST") == 0)
            mSkinAdjustments.mWaistY = offset;
        else if (wcscmp(partName, L"PANTS0") == 0)
            mSkinAdjustments.mPants0Y = offset;
        else if (wcscmp(partName, L"PANTS1") == 0)
            mSkinAdjustments.mPants1Y = offset;
        else if (wcscmp(partName, L"BOOT0") == 0)
            mSkinAdjustments.mBoot0Y = offset;
        else if (wcscmp(partName, L"BOOT1") == 0)
            mSkinAdjustments.mBoot1Y = offset;
    }
    break;
    case DLCManager::eDLCParameterType_Anim:
        swscanf(value.c_str(), L"%X", &mSkinAdjustments.mAnimFlags);
        {
            unsigned int skinId = CMinecraftApp::getSkinIdFromPath(m_path);
            gApp.SetSkinAdjustments(skinId, mSkinAdjustments);
        }
        break;
    case DLCManager::eDLCParameterType_GameFlags:
        swscanf(value.c_str(), L"%X", &mSkinAdjustments.mGameFlags);
        {
            unsigned int skinId = CMinecraftApp::getSkinIdFromPath(m_path);
            gApp.SetSkinAdjustments(skinId, mSkinAdjustments);
        }
        break;
    case DLCManager::eDLCParameterType_XMLVersion:
    {
        int xmlVersion;
        swscanf(value.c_str(), L"%i", &xmlVersion);
        mSkinAdjustments.mXMLVersion = xmlVersion;
    }
    break;
    default:
        break;
    }
}

std::wstring DLCSkinFile::getParameterAsString(DLCManager::EDLCParameterType parameter, const std::wstring&)
{
    switch (parameter)
    {
    case DLCManager::eDLCParameterType_DisplayName:
        if (mLocKeySet)
        {
            std::vector<std::wstring> parts = stringSplit(lookupString(mLocKey, mDisplayName), L'-');
            return parts.front();
        }
        return lookupString(mDisplayNameID, mDisplayName);
    case DLCManager::eDLCParameterType_ThemeName:
        if (mLocKeySet)
        {
            std::wstring themeKey = lookupString(mLocKey, mThemeName);
            if (themeKey.find(L"-") != std::wstring::npos)
            {
                std::vector<std::wstring> parts = stringSplit(themeKey, L'-');
                return parts.back();
            }
        }
        return lookupString(mThemeNameID, mThemeName);
    case DLCManager::eDLCParameterType_CapePath:
        return mCapePath;
    default:
        return L"";
    }
}

bool DLCSkinFile::getParameterAsBool(DLCManager::EDLCParameterType parameter)
{
    if (parameter != DLCManager::eDLCParameterType_Free)
        return false;
    return mFree;
}

DLCLocalisationFile* DLCSkinFile::getLocalisationFile()
{
    DLCLocalisationFile* localisation = NULL;
    localisation = static_cast<DLCLocalisationFile*>(
        m_pPack->getFile(DLCManager::eDLCType_Localisation, L"localisation.loc", true, true));
    if (localisation == NULL)
    {
        localisation = static_cast<DLCLocalisationFile*>(
            m_pPack->getFile(DLCManager::eDLCType_Localisation, L"languages.loc", true, true));
    }
    return localisation;
}

std::wstring DLCSkinFile::lookupString(const std::wstring& key, const std::wstring& fallback)
{
    DLCLocalisationFile* localisation = getLocalisationFile();
    if (!key.empty() && localisation != NULL)
    {
        StringTable* table = localisation->getStringTable();
        if (table != NULL)
            return std::wstring(table->getString(key));
    }
    return fallback;
}

std::vector<SKIN_BOX*>* DLCSkinFile::getAdditionalBoxes()
{
    return &mAdditionalBoxes;
}

int DLCSkinFile::getAdditionalBoxesCount()
{
    return (int)mAdditionalBoxes.size();
}

_SkinAdjustments DLCSkinFile::getSkinAdjustments()
{
    return mSkinAdjustments;
}

bool DLCSkinFile::isFree()
{
    return mFree;
}

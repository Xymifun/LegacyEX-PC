#include "platform/pc/PcTutorials.hpp"

#include "platform/pc/PcSettings.hpp"

namespace
{
const wchar_t* const sNames[kPcTutorialVersionCount] = {
    L"Pre-Release",
    L"TU1",
    L"TU3",
    L"TU5",
    L"TU7",
    L"TU9",
    L"TU12",
    L"TU14",
    L"TU19",
    L"TU31",
    L"TU46",
    L"Latest"};
const wchar_t* const FOLDERS[kPcTutorialVersionCount] = {
    L"PreRelease",
    L"TU1",
    L"TU3",
    L"TU5",
    L"TU7",
    L"TU9",
    L"TU12",
    L"TU14",
    L"TU19",
    L"TU31",
    L"TU46",
    0};
LevelGenerationOptions* OPTIONS[kPcTutorialVersionCount];

int VersionOf(LevelGenerationOptions* opts)
{
    for (int i = 0; i < kPcTutorialVersionCount; ++i)
    {
        if (opts != 0 && OPTIONS[i] == opts)
            return i;
    }
    return -1;
}
}

const wchar_t* PcTutorialVersionName(int version)
{
    return version >= 0 && version < kPcTutorialVersionCount ? sNames[version] : sNames[kPcTutorialVersionDefault];
}

const wchar_t* PcTutorialVersionFolder(int version)
{
    return version >= 0 && version < kPcTutorialVersionDefault ? FOLDERS[version] : 0;
}

void PcTutorialRegister(int version, LevelGenerationOptions* opts)
{
    if (version >= 0 && version < kPcTutorialVersionDefault)
        OPTIONS[version] = opts;
}

bool PcTutorialVersionAvailable(int version)
{
    return version == kPcTutorialVersionDefault
           || (version >= 0 && version < kPcTutorialVersionCount && OPTIONS[version] != 0);
}

bool PcTutorialVisible(LevelGenerationOptions* opts)
{
    const int selected = PcSettingsTutorialVersion();
    const int version = VersionOf(opts);
    if (version >= 0)
        return version == selected;
    return selected == kPcTutorialVersionDefault || !PcTutorialVersionAvailable(selected);
}

std::wstring PcTutorialBaseSavePath(LevelGenerationOptions* opts)
{
    const int version = VersionOf(opts);
    if (version < 0)
        return std::wstring();
    return std::wstring(L"PC/Tutorials/") + FOLDERS[version] + L"/Tutorial.mcs";
}

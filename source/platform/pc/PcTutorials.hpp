#ifndef PLATFORM_PC_PCTUTORIALS_HPP
#define PLATFORM_PC_PCTUTORIALS_HPP

#include <string>

class LevelGenerationOptions;

const wchar_t* PcTutorialVersionName(int version);
const wchar_t* PcTutorialVersionFolder(int version);
void PcTutorialRegister(int version, LevelGenerationOptions* options);
bool PcTutorialVersionAvailable(int version);
bool PcTutorialVisible(LevelGenerationOptions* options);
std::wstring PcTutorialBaseSavePath(LevelGenerationOptions* options);

#endif

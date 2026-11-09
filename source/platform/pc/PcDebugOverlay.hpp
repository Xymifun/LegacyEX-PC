#ifndef PLATFORM_PC_PCDEBUGOVERLAY_HPP
#define PLATFORM_PC_PCDEBUGOVERLAY_HPP

#include <string>
#include <vector>

class Minecraft;

void PcDebugOverlayToggle();
void PcDebugOverlayFrame();
bool PcDebugOverlayLines(Minecraft* minecraft, std::vector<std::wstring>* left, std::vector<std::wstring>* right);

#endif

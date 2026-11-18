#ifndef PLATFORM_PC_PCMULTIPLAYERMENU_HPP
#define PLATFORM_PC_PCMULTIPLAYERMENU_HPP

#include <string>

void PcOpenMultiplayerHosting(int pad);
std::wstring PcMultiplayerText();
void PcMultiplayerAskAddress(int pad);
void PcMultiplayerToggleIp();
bool PcMultiplayerShowingIp();
void PcShowAlertSoon(int titleId, int textId, int pad);
void PcMultiplayerMenuTick();

#endif

#ifndef PLATFORM_PC_PCNATIVEMODS_HPP
#define PLATFORM_PC_PCNATIVEMODS_HPP

#include <vector>

void PcNativeModsSetEnabled(bool enabled);
void PcNativeModsLoad();

void PcNativeModsFrame();
void PcNativeModsHostStart();
void PcNativeModsHostTick();
void PcNativeModsHostStop();

bool PcNativeModsHideMenus();

struct PcScriptHudItem;
bool PcNativeModsWantHud();
void PcNativeModsHud(std::vector<PcScriptHudItem>* items);

struct PcNativeViews;
bool PcNativeModsWantViews();
void PcNativeModsViewFrame(PcNativeViews* views);
void PcNativeModsViewDone(PcNativeViews* views);

#endif

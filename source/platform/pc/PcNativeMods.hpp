#ifndef PLATFORM_PC_PCNATIVEMODS_HPP
#define PLATFORM_PC_PCNATIVEMODS_HPP

#include <stddef.h>
#include <vector>

void PcNativeModsSetEnabled(bool enabled);
void PcNativeModsRegister();
void PcNativeModsLoad();
const char* PcNativeModsCurrentId();
size_t PcNativeModsCurrent();
void PcNativeModsSetCurrent(size_t mod);

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

#ifndef PLATFORM_PC_PCNATIVEMODS_HPP
#define PLATFORM_PC_PCNATIVEMODS_HPP

#include <stddef.h>
#include <string>
#include <vector>

void PcNativeModsSetEnabled(bool enabled);
void PcNativeModsRegister();
void PcNativeModsLoad();
const char* PcNativeModsIdFor(void* from);
std::string PcNativeModsDirectoryOf(const char* id);
const char* PcNativeModsCurrentId();
bool PcNativeModsHudHidden(int part);
size_t PcNativeModsCurrent();
void PcNativeModsSetCurrent(size_t mod);

void PcNativeModsFrame();
void PcNativeModsHostStart();
void PcNativeModsHostTick();
void PcNativeModsHostStop();

void* PcNativeModsFindFunction(const char* name, std::string* found, std::string* err);

bool PcNativeModsHideMenus();

struct PcScriptHudItem;
bool PcNativeModsWantHud();
void PcNativeModsHud(std::vector<PcScriptHudItem>* items);

struct PcNativeViews;
bool PcNativeModsWantViews();
void PcNativeModsViewFrame(PcNativeViews* views);
void PcNativeModsViewDone(PcNativeViews* views);
bool PcNativeModsWantViewDraw();
void PcNativeModsViewDraw(PcNativeViews* views);

#endif

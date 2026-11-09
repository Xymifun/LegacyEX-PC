#ifndef PLATFORM_PC_PCWORLDLAUNCHER_HPP
#define PLATFORM_PC_PCWORLDLAUNCHER_HPP

#include <string>

class Minecraft;

void PcRequestWorld(const std::string& name);
void PcRequestWorldSeed(long long seed);
void PcRequestWorldCreative();
void PcRequestWorldVisibility(int visibility);
bool PcRequestWorldSize(const std::string& size);
void PcRequestWorldRoam(int blocks);
void PcAcceptEula();
void PcRequestHost();
void PcRequestJoin(const std::string& hostName);
void PcRequestInteractiveJoin(const std::string& address);
void PcRequestQuitOnDisconnect();
void PcRequestLeaveAt(double seconds);
bool PcWorldLauncherQuitRequested();
bool PcWorldRequested();
void PcWorldLauncherTick(Minecraft* minecraft);
bool PcWorldLauncherFinish();

#endif

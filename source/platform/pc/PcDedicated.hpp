#ifndef PLATFORM_PC_PCDEDICATED_HPP
#define PLATFORM_PC_PCDEDICATED_HPP

bool PcIsDedicatedServer();
void PcSetDedicatedServer(bool dedicated);
void PcSetDedicatedSaveMinutes(int minutes);
void PcDedicatedTick();
bool PcDedicatedStopRequested();

class INetworkPlayer;
bool PcIsHiddenHost(INetworkPlayer* player);
bool PcIsServerOperator(INetworkPlayer* player);
bool PcPhantomsEnabled();
void PcSetPhantomsEnabled(bool enabled);

#endif

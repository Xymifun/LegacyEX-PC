#ifndef PLATFORM_PC_PCSETTINGS_HPP
#define PLATFORM_PC_PCSETTINGS_HPP

#include <string>

const int kPcFovMinimum = 30;
const int kPcFovMaximum = 110;
const int kPcFramerateSteps = 10;
const int kPcRenderScaleSteps = 6;
const int kPcRenderDistanceMinimum = 2;
const int kPcRenderDistanceMaximum = 32;
const int kPcRenderDistanceDefault = 10;
const int kPcSimulationDistanceMinimum = 5;
const int kPcSimulationDistanceMaximum = 16;
const int kPcSimulationDistanceDefault = 9;
const int kPcServerChunkBudget = 12000;
const int kPcGoldenAgeBlocks = 1;
const int kPcGoldenAgeItems = 2;
const int kPcGoldenAgeModels = 4;
const int kPcGoldenAgeSounds = 8;
const int kPcGoldenAgeWater = 16;
const int kPcGoldenAgeLava = 32;
const int kPcGoldenAgeAll = 63;
const int kPcTitleLogoCount = 7;
const int kPcTutorialVersionCount = 12;
const int kPcTutorialVersionDefault = 11;
const int kPcPanoramaCount = 9;
const int kPcPanoramaDefault = 8;

void PcSettingsLoad();
void PcSettingsSave();
void PcSettingsReset();
const int kPcVoiceModeActivation = 0;
const int kPcVoiceModePushToTalk = 1;
const int kPcVoiceModeCount = 2;

const int kPcProximityRangeMinimum = 8;
const int kPcProximityRangeMaximum = 128;
const int kPcProximityRangeStep = 8;
const int kPcProximityRangeDefault = 32;

const int kPcHostProximityUsersChoose = 0;
const int kPcHostProximityForce = 1;
const int kPcHostProximityDisable = 2;
const int kPcHostProximityCount = 3;

int PcSettingsHostProximity();
void PcSettingsSetHostProximity(int mode);
int PcSettingsHostProximityRange();
void PcSettingsSetHostProximityRange(int blocks);
bool PcSettingsProximityChat();
void PcSettingsSetProximityChat(bool on);
int PcSettingsProximityRange();
void PcSettingsSetProximityRange(int blocks);
const std::string& PcSettingsVoiceInput();
int PcSettingsVoiceMode();
void PcSettingsSetVoiceMode(int mode);
void PcSettingsSetVoiceInput(const std::string& id);
int PcSettingsMicVolume();
void PcSettingsSetMicVolume(int volume);
int PcSettingsVoiceVolume();
void PcSettingsSetVoiceVolume(int volume);
int PcSettingsGoldenAge();
void PcSettingsSetGoldenAge(int groups);
int PcSettingsTitleLogo();
void PcSettingsSetTitleLogo(int logo);
int PcSettingsTutorialVersion();
void PcSettingsSetTutorialVersion(int version);
int PcSettingsPanorama();
void PcSettingsSetPanorama(int panorama);
int PcSettingsFov();
void PcSettingsSetFov(int fov);
int PcSettingsFramerateStep();
void PcSettingsSetFramerateStep(int step);
int PcSettingsFramerateLimit(int step);
bool PcSettingsVsync();
int PcSettingsRenderScaleStep();
void PcSettingsSetRenderScaleStep(int step);
int PcSettingsRenderScale(int step);
int PcSettingsRenderScalePercent();
int PcSettingsRenderDistance();
void PcSettingsSetRenderDistance(int chunks);
int PcSettingsSimulationDistance();
void PcSettingsSetSimulationDistance(int chunks);
int PcSettingsServerDistance();
void PcSettingsSetServerDistance(int chunks);
int PcSettingsServerRadiusCap(int players);
float PcSettingsFogDistance();
bool PcSettingsGlideGhostHidden();
void PcSettingsToggleGlideGhost();
void PcSettingsApplyDisplay();
void PcSettingsLimitFrame();

#endif

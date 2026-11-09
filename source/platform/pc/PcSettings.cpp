#include "platform/pc/PcSettings.hpp"

#include "platform/pc/PcDisplay.hpp"
#include "platform/pc/PcOs.hpp"
#include "platform/pc/PcPaths.hpp"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <string>

namespace
{
const int DEFAULT_FOV = 70;
const int FRAMERATE_LIMITS[kPcFramerateSteps] = {-1, 30, 60, 90, 120, 144, 165, 240, 300, 0};
const int kRenderScales[kPcRenderScaleSteps] = {50, 60, 70, 80, 90, 100};
const unsigned long long NANOSECONDS_PER_SECOND = 1000000000ULL;
const unsigned long long SPIN_NANOSECONDS = 2000000ULL;

int s_fov = DEFAULT_FOV;
int framerateStep = 0;
int s_renderScaleStep = kPcRenderScaleSteps - 1;
int renderDistance = kPcRenderDistanceDefault;
int serverDistance = kPcRenderDistanceMaximum;
int simulationDistance = kPcSimulationDistanceDefault;
bool s_glideGhostHidden = false;
int s_goldenAge = 0;
int titleLogo = 0;
int tutorialVersion = kPcTutorialVersionDefault;
int s_panorama = kPcPanoramaDefault;
std::string voiceInput;
int micVolume = 100;
int voiceVolume = 100;
int voiceMode = 0;
bool proximityChat = false;
int hostProximity = kPcHostProximityUsersChoose;
int hostProximityRange = 0;
int proximityRange = kPcProximityRangeDefault;
bool s_vsyncApplied = true;
unsigned long long nextFrame = 0;

std::string SettingsPath()
{
    return PcSaveRoot() + "/pcsettings.txt";
}

int Clamp(int v, int lo, int hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}
}

void PcSettingsLoad()
{
    FILE* fp = fopen(SettingsPath().c_str(), "r");
    if (fp == 0)
        return;
    char line[1024];
    bool waterAndLavaSaved = false;
    while (fgets(line, sizeof(line), fp) != 0)
    {
        int v = 0;
        if (strncmp(line, "voiceinput=", 11) == 0)
        {
            voiceInput = line + 11;
            while (!voiceInput.empty()
                   && (voiceInput[voiceInput.size() - 1] == '\n' || voiceInput[voiceInput.size() - 1] == '\r'))
                voiceInput.erase(voiceInput.size() - 1);
        }
        else if (sscanf(line, "micvolume=%d", &v) == 1)
            micVolume = Clamp(v, 0, 200);
        else if (sscanf(line, "voicevolume=%d", &v) == 1)
            voiceVolume = Clamp(v, 0, 200);
        else if (sscanf(line, "hostproximity=%d", &v) == 1)
            hostProximity = Clamp(v, 0, kPcHostProximityCount - 1);
        else if (sscanf(line, "hostproximityrange=%d", &v) == 1)
            hostProximityRange = v == 0 ? 0 : Clamp(v, kPcProximityRangeMinimum, kPcProximityRangeMaximum);
        else if (sscanf(line, "proximitychat=%d", &v) == 1)
            proximityChat = v != 0;
        else if (sscanf(line, "proximityrange=%d", &v) == 1)
            proximityRange = Clamp(v, kPcProximityRangeMinimum, kPcProximityRangeMaximum);
        else if (sscanf(line, "voicemode=%d", &v) == 1)
            voiceMode = Clamp(v, 0, kPcVoiceModeCount - 1);
        else if (sscanf(line, "goldenagegroups=%d", &v) == 1)
            waterAndLavaSaved = v >= 6;
        else if (sscanf(line, "fov=%d", &v) == 1)
            s_fov = Clamp(v, kPcFovMinimum, kPcFovMaximum);
        else if (sscanf(line, "framerate=%d", &v) == 1)
            framerateStep = Clamp(v, 0, kPcFramerateSteps - 1);
        else if (sscanf(line, "glideghost=%d", &v) == 1)
            s_glideGhostHidden = v == 0;
        else if (sscanf(line, "renderdistance=%d", &v) == 1)
            renderDistance = Clamp(v, kPcRenderDistanceMinimum, kPcRenderDistanceMaximum);
        else if (sscanf(line, "serverdistance=%d", &v) == 1)
            serverDistance = Clamp(v, kPcRenderDistanceMinimum, kPcRenderDistanceMaximum);
        else if (sscanf(line, "goldenage=%d", &v) == 1)
            s_goldenAge = v & kPcGoldenAgeAll;
        else if (sscanf(line, "titlelogo=%d", &v) == 1)
            titleLogo = Clamp(v, 0, kPcTitleLogoCount - 1);
        else if (sscanf(line, "tutorial=%d", &v) == 1)
            tutorialVersion = Clamp(v, 0, kPcTutorialVersionCount - 1);
        else if (sscanf(line, "menupanorama=%d", &v) == 1)
            s_panorama = Clamp(v, 0, kPcPanoramaCount - 1);
        else if (sscanf(line, "simulation=%d", &v) == 1)
            simulationDistance = Clamp(v, kPcSimulationDistanceMinimum, kPcSimulationDistanceMaximum);
        else if (sscanf(line, "renderscale=%d", &v) == 1)
        {
            s_renderScaleStep = 0;
            while (s_renderScaleStep < kPcRenderScaleSteps - 1 && kRenderScales[s_renderScaleStep] < v)
                ++s_renderScaleStep;
        }
    }
    fclose(fp);
    if (!waterAndLavaSaved && (s_goldenAge & kPcGoldenAgeBlocks) != 0)
        s_goldenAge |= kPcGoldenAgeWater | kPcGoldenAgeLava;
}

void PcSettingsSave()
{
    FILE* fp = fopen(SettingsPath().c_str(), "w");
    if (fp == 0)
        return;
    fprintf(fp,
            "fov=%d\nframerate=%d\nrenderscale=%d\nrenderdistance=%d\nsimulation=%d\nglideghost=%d\nserverdistance=%d\n"
            "goldenage=%d\ngoldenagegroups=6\ntitlelogo=%d\ntutorial=%d\nmenupanorama=%d\n",
            s_fov, framerateStep, kRenderScales[s_renderScaleStep], renderDistance, simulationDistance,
            s_glideGhostHidden ? 0 : 1, serverDistance, s_goldenAge, titleLogo, tutorialVersion,
            s_panorama);
    fprintf(fp,
            "voiceinput=%s\nmicvolume=%d\nvoicemode=%d\nproximitychat=%d\nproximityrange=%d\nhostproximity=%d\n"
            "hostproximityrange=%d\nvoicevolume=%d\n",
            voiceInput.c_str(), micVolume, voiceMode, proximityChat ? 1 : 0, proximityRange, hostProximity,
            hostProximityRange, voiceVolume);
    fclose(fp);
}

void PcSettingsReset()
{
    s_fov = DEFAULT_FOV;
    framerateStep = 0;
    s_renderScaleStep = kPcRenderScaleSteps - 1;
    renderDistance = kPcRenderDistanceDefault;
    simulationDistance = kPcSimulationDistanceDefault;
    s_goldenAge = 0;
    titleLogo = 0;
    tutorialVersion = kPcTutorialVersionDefault;
    s_panorama = kPcPanoramaDefault;
    voiceInput.clear();
    micVolume = 100;
    voiceVolume = 100;
    voiceMode = 0;
    proximityChat = false;
    proximityRange = kPcProximityRangeDefault;
    PcSettingsApplyDisplay();
    PcSettingsSave();
}

const std::string& PcSettingsVoiceInput()
{
    return voiceInput;
}

void PcSettingsSetVoiceInput(const std::string& id)
{
    voiceInput = id;
}

int PcSettingsHostProximity()
{
    return hostProximity;
}

void PcSettingsSetHostProximity(int mode)
{
    hostProximity = Clamp(mode, 0, kPcHostProximityCount - 1);
}

int PcSettingsHostProximityRange()
{
    return hostProximityRange;
}

void PcSettingsSetHostProximityRange(int n)
{
    hostProximityRange = n <= 0 ? 0 : Clamp(n, kPcProximityRangeMinimum, kPcProximityRangeMaximum);
}

bool PcSettingsProximityChat()
{
    return proximityChat;
}

void PcSettingsSetProximityChat(bool on)
{
    proximityChat = on;
}

int PcSettingsProximityRange()
{
    return proximityRange;
}

void PcSettingsSetProximityRange(int n)
{
    proximityRange = Clamp(n, kPcProximityRangeMinimum, kPcProximityRangeMaximum);
}

int PcSettingsVoiceMode()
{
    return voiceMode;
}

void PcSettingsSetVoiceMode(int mode)
{
    voiceMode = Clamp(mode, 0, kPcVoiceModeCount - 1);
}

int PcSettingsMicVolume()
{
    return micVolume;
}

void PcSettingsSetMicVolume(int vol)
{
    micVolume = Clamp(vol, 0, 200);
}

int PcSettingsVoiceVolume()
{
    return voiceVolume;
}

void PcSettingsSetVoiceVolume(int vol)
{
    voiceVolume = Clamp(vol, 0, 200);
}

int PcSettingsGoldenAge()
{
    return s_goldenAge;
}

void PcSettingsSetGoldenAge(int groups)
{
    s_goldenAge = groups & kPcGoldenAgeAll;
}

int PcSettingsTitleLogo()
{
    return titleLogo;
}

void PcSettingsSetTitleLogo(int logo)
{
    titleLogo = Clamp(logo, 0, kPcTitleLogoCount - 1);
}

int PcSettingsTutorialVersion()
{
    return tutorialVersion;
}

void PcSettingsSetTutorialVersion(int version)
{
    tutorialVersion = Clamp(version, 0, kPcTutorialVersionCount - 1);
}

int PcSettingsPanorama()
{
    return s_panorama;
}

void PcSettingsSetPanorama(int panorama)
{
    s_panorama = Clamp(panorama, 0, kPcPanoramaCount - 1);
}

int PcSettingsFov()
{
    return s_fov;
}

void PcSettingsSetFov(int fov)
{
    s_fov = Clamp(fov, kPcFovMinimum, kPcFovMaximum);
}

int PcSettingsFramerateStep()
{
    return framerateStep;
}

void PcSettingsSetFramerateStep(int s)
{
    framerateStep = Clamp(s, 0, kPcFramerateSteps - 1);
    PcSettingsApplyDisplay();
}

int PcSettingsFramerateLimit(int s)
{
    return FRAMERATE_LIMITS[Clamp(s, 0, kPcFramerateSteps - 1)];
}

bool PcSettingsVsync()
{
    return PcSettingsFramerateLimit(framerateStep) < 0;
}

int PcSettingsRenderScaleStep()
{
    return s_renderScaleStep;
}

void PcSettingsSetRenderScaleStep(int step)
{
    s_renderScaleStep = Clamp(step, 0, kPcRenderScaleSteps - 1);
}

int PcSettingsRenderScale(int step)
{
    return kRenderScales[Clamp(step, 0, kPcRenderScaleSteps - 1)];
}

int PcSettingsRenderScalePercent()
{
    return kRenderScales[s_renderScaleStep];
}

int PcSettingsRenderDistance()
{
    return renderDistance;
}

void PcSettingsSetRenderDistance(int n)
{
    renderDistance = Clamp(n, kPcRenderDistanceMinimum, kPcRenderDistanceMaximum);
}

int PcSettingsServerDistance()
{
    return serverDistance;
}

void PcSettingsSetServerDistance(int n)
{
    serverDistance = Clamp(n, kPcRenderDistanceMinimum, kPcRenderDistanceMaximum);
}

int PcSettingsServerRadiusCap(int players)
{
    if (players < 1)
        players = 1;
    const double side = sqrt((double)kPcServerChunkBudget / players);
    const int budgetRadius = (int)((side - 1.0) / 2.0);
    return Clamp(budgetRadius < serverDistance ? budgetRadius : serverDistance, kPcRenderDistanceMinimum,
                 kPcRenderDistanceMaximum);
}

int PcSettingsSimulationDistance()
{
    return simulationDistance;
}

void PcSettingsSetSimulationDistance(int n)
{
    simulationDistance = Clamp(n, kPcSimulationDistanceMinimum, kPcSimulationDistanceMaximum);
}

bool PcSettingsGlideGhostHidden()
{
    return s_glideGhostHidden;
}

void PcSettingsToggleGlideGhost()
{
    s_glideGhostHidden = !s_glideGhostHidden;
    PcSettingsSave();
}

float PcSettingsFogDistance()
{
    if (renderDistance < kPcRenderDistanceDefault)
        return 25.6f * (float)renderDistance;
    if (renderDistance <= 16)
        return 256.0f;
    return 16.0f * (float)renderDistance;
}

void PcSettingsApplyDisplay()
{
    if (PcGetDisplayMode() != ePcDisplay_Window)
        return;
    const bool vsync = PcSettingsVsync();
    if (vsync == s_vsyncApplied)
        return;
    s_vsyncApplied = vsync;
    PcOsWindowSetSwapInterval(vsync ? 1 : 0);
}

void PcSettingsLimitFrame()
{
    const int limit = PcSettingsFramerateLimit(framerateStep);
    if (limit <= 0 || PcGetDisplayMode() != ePcDisplay_Window)
    {
        nextFrame = 0;
        return;
    }
    const unsigned long long interval = NANOSECONDS_PER_SECOND / (unsigned long long)limit;
    unsigned long long now = PcOsMonotonicNanoseconds();
    if (nextFrame == 0 || now > nextFrame + interval)
        nextFrame = now;
    while (now + SPIN_NANOSECONDS < nextFrame)
    {
        PcOsSleepMilliseconds(1);
        now = PcOsMonotonicNanoseconds();
    }
    while (now < nextFrame)
        now = PcOsMonotonicNanoseconds();
    nextFrame += interval;
}

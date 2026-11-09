#ifndef PLATFORM_PC_PCVOICE_HPP
#define PLATFORM_PC_PCVOICE_HPP

#include <string>
#include <vector>

const unsigned int kPcVoiceRate = 48000;
const int kPcMicVolumeMaximum = 200;
const int kPcMicVolumeDefault = 100;

struct PcVoicePosition
{
    int smallId;
    int dimension;
    double x;
    double y;
    double z;
    float yaw;
};

const unsigned char kPcVoiceFullGain = 255;
const unsigned char kPcVoiceCentrePan = 127;

void PcVoiceServerTick();
void PcVoiceSetPositions(const std::vector<PcVoicePosition>& positions);
bool PcVoiceRoute(int speakerId, int listenerId, bool proximity, int range, unsigned char* gain, unsigned char* pan);
void PcVoiceSetHostOverride(int mode, int range);
void PcVoiceClearHostOverride();
bool PcVoiceHostOverride(int* mode, int* range);
bool PcVoiceEffectiveProximity(int* range);

void PcVoiceSetFileInput(const char* path);
void PcVoiceRequestTestAtStart();
void PcVoiceForceChat();
void PcVoiceTick();

bool PcVoiceTakeOutgoing(std::vector<unsigned char>* packet);
void PcVoiceReceive(int speakerId, unsigned char gain, unsigned char pan, const unsigned char* data,
                    unsigned int size);
bool PcVoiceSpeakerTalking(int speakerId);
bool PcVoiceSending();
bool PcVoiceLocalTalking();
void PcVoiceSetMuted(int speakerId, bool muted);
bool PcVoiceMuted(int speakerId);
void PcVoiceForgetSpeaker(int speakerId);
void PcVoiceStatistics(unsigned int* sent, unsigned int* received);

void PcVoiceRefreshDevices();
int PcVoiceDeviceCount();
std::wstring PcVoiceDeviceName(int index);
int PcVoiceSelectedDevice();
void PcVoiceSelectDevice(int index);

void PcVoiceSetTest(bool on);
bool PcVoiceTesting();
bool PcVoiceInputOpen();
int PcVoiceInputLevel();

void PcVoiceMix(float* stereo, unsigned int frames);

#endif

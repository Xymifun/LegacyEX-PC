#ifndef GAME_UTIL_LEGACYUTILITYFUNCTIONS_HPP
#define GAME_UTIL_LEGACYUTILITYFUNCTIONS_HPP

#include <stdint.h>

struct _SAMPLE;
struct _DIG_DRIVER;

extern "C" void outresfloat(float fResult);
extern "C" void outresint64(uint32_t low, uint32_t high);
extern "C" void outresuint(uint32_t value);
char ourtoupper(char value);
int OnlyNum(const char* value);
extern "C" unsigned int find_ASI_provider(const char* attributeName, const char* fileName);
extern "C" int AIL_stricmp(const char* first, const char* second);
extern "C" int AIL_strnicmp(const char* first, const char* second, int count);
extern "C" void AIL_set_sample_speaker_scale_factors(_SAMPLE* pSample, const int* pSpeakers, const float* pScales,
                                                     int iCount);
extern "C" void AIL_sample_speaker_scale_factors(_SAMPLE* pSample, int* pSpeakers, float* pScales, int iCount);
extern "C" void AIL_sample_channel_levels(_SAMPLE* pSample, int* pSources, int* pDestinations, float* pLevels,
                                          int iCount);
extern "C" void AIL_set_sample_channel_levels(_SAMPLE* pSample, const int* pSources, const int* pDestinations,
                                              const float* pLevels, int iCount);
extern "C" void AIL_set_sample_reverb_levels(_SAMPLE* pSample, float fFirst, float fSecond);
extern "C" void AIL_set_sample_low_pass_cut_off(_SAMPLE* pSample, int iChannel, float fCutOff);
extern "C" void AIL_set_sample_51_volume_pan(_SAMPLE* pSample, float fVolume, float fPan, float fFade, float fCenter,
                                             float fLowFrequency);
extern "C" int AIL_sample_processor(_SAMPLE* pSample, int iStage);
extern "C" int AIL_set_sample_processor(_SAMPLE* pSample, int iStage, int iProvider);
extern "C" int AIL_install_bus_compressor(_DIG_DRIVER* pDriver, int iBus, int iStage, int iInputBus);
extern "C" int AIL_open_filter(unsigned int uProvider, _DIG_DRIVER* pDriver);
extern "C" int AIL_sample_channel_count(_SAMPLE* pSample, unsigned int* puMask);
extern "C" int AIL_sample_playback_rate(_SAMPLE* pSample);
extern "C" void AIL_set_sample_playback_rate(_SAMPLE* pSample, int iRate);

#endif

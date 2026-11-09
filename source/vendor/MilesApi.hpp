#ifndef SOURCE_VENDOR_MILES_API_HPP
#define SOURCE_VENDOR_MILES_API_HPP

#include "vendor/MilesAsync.hpp"
#include "vendor/MilesIO.hpp"
#include "vendor/MilesRR.hpp"
#include "vendor/MilesStream.hpp"

struct _SAMPLE;
struct _DIG_DRIVER;
struct _AILSOUNDINFO;
struct SoundInstanceInfo;
class EventSystem;
struct _RadSoundSystem;
struct FLT_INSTANCE;
struct FalloffGraphPoint;
struct SoundBank;

typedef void* (*AIL_MEM_ALLOC_CALLBACK)(unsigned int uSize);
typedef void (*AIL_MEM_FREE_CALLBACK)(void* pBlock);
typedef int (*RADSS_OPEN_DRIVER_CALLBACK)(_RadSoundSystem* pSystem, unsigned int uFirst, unsigned int uSecond,
                                          unsigned int uThird, unsigned int uFourth, unsigned int uFifth);
typedef float (*AIL_FALLOFF_CALLBACK)(_SAMPLE* pSample, float fDistance, float fRolloffFactor, float fMinimumDistance,
                                      float fMaximumDistance);

extern int MSS_api_started;
extern rrMutex MSS_api_mutex;
extern MilesProfileSite MSS_api_lock_wait_site;
extern MilesProfileSite MSS_api_lock_site;
extern MilesProfileSite MSS_api_unlock_site;
extern MilesProfileSite MSS_api_mutex_name_site;

extern AIL_MEM_ALLOC_CALLBACK AIL_mem_alloc_handler;
extern AIL_MEM_FREE_CALLBACK AIL_mem_free_handler;

extern char AIL_error[256];
extern int AIL_preference[37];

extern int AIL_debug_enabled;
extern int AIL_debug_nested;
extern int AIL_debug_depth;

extern _DIG_DRIVER* MSS_primary_digital_driver;

extern "C" void AIL_debug_log(const char* pFormat, ...);
extern "C" void* AIL_mem_alloc_lock_info(int iSize, const char* pOwnerName, int iOwnerLine);
extern "C" void AIL_mem_free_lock(void* pBlock);
extern "C" void outresuhex(unsigned int uResult);
extern "C" void outresint(int iResult);

extern "C" void InMilesMutex();
extern "C" void OutMilesMutex();
extern "C" void AIL_lock_mutex();
extern "C" void AIL_unlock_mutex();
extern "C" void AIL_API_startup();
extern "C" int AIL_API_background();
extern "C" void AIL_API_set_error(const char* pError);
extern "C" void* AIL_API_mem_alloc_lock(unsigned int uSize);
extern "C" void AIL_API_mem_free_lock(void* pBlock);
extern "C" void* MSS_alloc_info(unsigned int uSize, unsigned int uUser, const char* pOwnerName, int iOwnerLine);
extern "C" void MSS_free_info(void* pBlock);

extern "C" RADSS_OPEN_DRIVER_CALLBACK RADSS_AXInstallDriver1(int iDevice, int iSecond);
extern "C" RADSS_OPEN_DRIVER_CALLBACK AIL_get_default_output();
extern "C" RADSS_OPEN_DRIVER_CALLBACK AIL_output_ax(int iDevice);
extern "C" void AIL_set_file_callbacks(AIL_FILE_OPEN_CALLBACK pOpen, AIL_FILE_CLOSE_CALLBACK pClose,
                                       AIL_FILE_SEEK_CALLBACK pSeek, AIL_FILE_READ_CALLBACK pRead);
extern "C" void AIL_set_sample_bus(_SAMPLE* pSample, unsigned int uBus);

extern "C" _DIG_DRIVER* AIL_open_generic_digital_driver(unsigned int uFrequency, int iBits, unsigned int uSpeakerConfig,
                                                        unsigned int uFlags, RADSS_OPEN_DRIVER_CALLBACK pOutput);
extern "C" void AIL_close_digital_driver(_DIG_DRIVER* pDriver);
extern "C" _SAMPLE* AIL_allocate_sample_handle(_DIG_DRIVER* pDriver);
extern "C" _SAMPLE* AIL_allocate_bus(_DIG_DRIVER* pDriver);
extern "C" _SAMPLE* AIL_bus_sample_handle(_DIG_DRIVER* pDriver, int iBus);
extern "C" void AIL_release_sample_handle(_SAMPLE* pSample);
extern "C" int AIL_init_sample(_SAMPLE* pSample, unsigned int uFormat);
extern "C" int AIL_set_sample_file(_SAMPLE* pSample, const void* pFile, int iBlock);
extern "C" int AIL_set_named_sample_file(_SAMPLE* pSample, const char* pFileName, const void* pFile, int iSize,
                                         int iBlock);
extern "C" int AIL_set_digital_driver_processor(_DIG_DRIVER* pDriver, int iStage, int iProvider);
extern "C" void AIL_set_sample_address(_SAMPLE* pSample, const void* pStart, unsigned int uLength);
extern "C" void AIL_set_sample_adpcm_block_size(_SAMPLE* pSample, unsigned int uBlockSize);
extern "C" void AIL_start_sample(_SAMPLE* pSample);
extern "C" void AIL_stop_sample(_SAMPLE* pSample);
extern "C" void AIL_resume_sample(_SAMPLE* pSample);
extern "C" void AIL_set_sample_playback_rate_factor(_SAMPLE* pSample, float fFactor);
extern "C" void AIL_set_sample_playback_delay(_SAMPLE* pSample, int iDelay);
extern "C" void AIL_set_speaker_configuration(_DIG_DRIVER* pDriver, const float* pSpeakerPositions, int iChannels,
                                              float fAzimuth);
extern "C" void AIL_set_speaker_reverb_levels(_DIG_DRIVER* pDriver, const float* pWetLevels, const float* pDryLevels,
                                              const int* pSpeakers, int iCount);
extern "C" void AIL_set_sample_volume_levels(_SAMPLE* pSample, float fLeftLevel, float fRightLevel);
extern "C" void AIL_set_sample_loop_count(_SAMPLE* pSample, int iLoopCount);
extern "C" void AIL_set_sample_loop_block(_SAMPLE* pSample, int iLoopStart, int iLoopEnd);
extern "C" int AIL_sample_status(_SAMPLE* pSample);
extern "C" float AIL_sample_playback_rate_factor(_SAMPLE* pSample);
extern "C" void AIL_sample_volume_levels(_SAMPLE* pSample, float* pLeftLevel, float* pRightLevel);
extern "C" void AIL_set_digital_master_volume_level(_DIG_DRIVER* pDriver, float fLevel);
extern "C" void AIL_set_digital_master_reverb_levels(_DIG_DRIVER* pDriver, int iBus, float fWetLevel, float fDryLevel);
extern "C" void AIL_set_digital_master_reverb(_DIG_DRIVER* pDriver, int iBus, float fReverbTime, float fReverbPredelay,
                                              float fReverbDamping);
extern "C" int AIL_minimum_sample_buffer_size(_DIG_DRIVER* pDriver, int iPlaybackRate, int iFormat);
extern "C" int AIL_sample_buffer_available(_SAMPLE* pSample);
extern "C" int AIL_set_sample_buffer_count(_SAMPLE* pSample, int iCount);
extern "C" int AIL_load_sample_buffer(_SAMPLE* pSample, int iBuffer, const void* pData, unsigned int uLength);
extern "C" int AIL_sample_granularity(_SAMPLE* pSample);
extern "C" void AIL_set_sample_position(_SAMPLE* pSample, unsigned int uPosition);
extern "C" unsigned int AIL_sample_position(_SAMPLE* pSample);
extern "C" AIL_FALLOFF_CALLBACK AIL_register_falloff_function_callback(_SAMPLE* pSample,
                                                                       AIL_FALLOFF_CALLBACK pCallback);
extern "C" _STREAM* AIL_open_stream(_DIG_DRIVER* pDriver, const char* pFileName, int iStreamMem);
extern "C" void AIL_close_stream(_STREAM* pStream);
extern "C" _SAMPLE* AIL_stream_sample_handle(_STREAM* pStream);
extern "C" void AIL_start_stream(_STREAM* pStream);
extern "C" void AIL_pause_stream(_STREAM* pStream, int iPause);
extern "C" void AIL_set_stream_loop_block(_STREAM* pStream, int iLoopStart, int iLoopEnd);
extern "C" void AIL_set_stream_loop_count(_STREAM* pStream, int iLoopCount);
extern "C" int AIL_stream_status(_STREAM* pStream);
extern "C" void AIL_set_stream_position(_STREAM* pStream, int iPosition);
extern "C" int AIL_stream_position(_STREAM* pStream);
extern "C" void AIL_set_sample_ms_position(_SAMPLE* pSample, unsigned int uMilliseconds);
extern "C" void AIL_set_stream_ms_position(_STREAM* pStream, unsigned int uMilliseconds);
extern "C" int AIL_WAV_info(const void* pData, _AILSOUNDINFO* pInfo);
extern "C" int MilesEnumerateSoundInstances(EventSystem* pEventSystem, unsigned int* puNext, unsigned int uTypeMask,
                                            const char* pLabels, int iEventIndex, int iEventSerial,
                                            SoundInstanceInfo* pInfo);
extern "C" int MilesBeginEventQueueProcessing();
extern "C" int MilesCompleteEventQueueProcessing();
extern "C" unsigned int MilesStartupEventSystem(_DIG_DRIVER* pDriver, int iEventMemory, int iFlags, int iStreamMemory);
extern "C" SoundBank* MilesAddSoundBank(const char* pFileName, const char* pBankName);
extern "C" unsigned long long MilesEnqueueEventByName(const char* pEventName);
extern "C" _DIG_DRIVER* AIL_primary_digital_driver(_DIG_DRIVER* pDriver);
extern "C" void AIL_set_sample_3D_distances(_SAMPLE* pSample, float fMaxDistance, float fMinDistance,
                                            int iAutomaticWetAttenuation);
extern "C" void AIL_set_room_type(_DIG_DRIVER* pDriver, int iBus, int iRoomType);
extern "C" void AIL_set_3D_rolloff_factor(_DIG_DRIVER* pDriver, float fRolloffFactor);
extern "C" void AIL_set_3D_doppler_factor(_DIG_DRIVER* pDriver, float fDopplerFactor);
extern "C" void AIL_set_sample_3D_position(_SAMPLE* pSample, float fX, float fY, float fZ);
extern "C" void AIL_set_listener_3D_position(_DIG_DRIVER* pDriver, float fX, float fY, float fZ);
extern "C" void AIL_set_sample_3D_volume_falloff(_SAMPLE* pSample, const FalloffGraphPoint* pPoints, int iPointCount);
extern "C" void AIL_set_sample_3D_lowpass_falloff(_SAMPLE* pSample, const FalloffGraphPoint* pPoints, int iPointCount);
extern "C" void AIL_set_sample_3D_exclusion_falloff(_SAMPLE* pSample, const FalloffGraphPoint* pPoints,
                                                    int iPointCount);
extern "C" void AIL_set_listener_3D_orientation(_DIG_DRIVER* pDriver, float fForwardX, float fForwardY, float fForwardZ,
                                                float fUpX, float fUpY, float fUpZ);
extern "C" void AIL_close_filter(FLT_INSTANCE* pFilter);

extern "C" _DIG_DRIVER* AIL_API_open_generic_digital_driver(unsigned int uFrequency, int iBits,
                                                            unsigned int uSpeakerConfig, unsigned int uFlags,
                                                            RADSS_OPEN_DRIVER_CALLBACK pOutput);
extern "C" void AIL_API_close_digital_driver(_DIG_DRIVER* pDriver);
extern "C" _SAMPLE* AIL_API_allocate_sample_handle(_DIG_DRIVER* pDriver);
extern "C" _SAMPLE* AIL_API_allocate_bus(_DIG_DRIVER* pDriver);
extern "C" _SAMPLE* AIL_API_bus_sample_handle(_DIG_DRIVER* pDriver, int iBus);
extern "C" void AIL_API_release_sample_handle(_SAMPLE* pSample);
extern "C" int AIL_API_init_sample(_SAMPLE* pSample, unsigned int uFormat);
extern "C" int AIL_API_set_sample_file(_SAMPLE* pSample, const void* pFile, int iBlock);
extern "C" int AIL_API_set_named_sample_file(_SAMPLE* pSample, const char* pFileName, const void* pFile, int iSize,
                                             int iBlock);
extern "C" int AIL_API_set_digital_driver_processor(_DIG_DRIVER* pDriver, int iStage, int iProvider);
extern "C" void AIL_API_set_sample_address(_SAMPLE* pSample, const void* pStart, unsigned int uLength);
extern "C" void AIL_API_set_sample_adpcm_block_size(_SAMPLE* pSample, unsigned int uBlockSize);
extern "C" void AIL_API_start_sample(_SAMPLE* pSample);
extern "C" void AIL_API_stop_sample(_SAMPLE* pSample);
extern "C" void AIL_API_resume_sample(_SAMPLE* pSample);
extern "C" void AIL_API_set_sample_playback_rate_factor(_SAMPLE* pSample, float fFactor);
extern "C" void AIL_API_set_sample_playback_delay(_SAMPLE* pSample, int iDelay);
extern "C" int AIL_API_set_sample_is_3D(_SAMPLE* pSample, int iEnabled);
extern "C" void AIL_API_set_speaker_configuration(_DIG_DRIVER* pDriver, const float* pSpeakerPositions, int iChannels,
                                                  float fAzimuth);
extern "C" void AIL_API_set_speaker_reverb_levels(_DIG_DRIVER* pDriver, const float* pWetLevels,
                                                  const float* pDryLevels, const int* pSpeakers, int iCount);
extern "C" void AIL_API_set_sample_volume_levels(_SAMPLE* pSample, float fLeftLevel, float fRightLevel);
extern "C" void AIL_API_set_sample_loop_count(_SAMPLE* pSample, int iLoopCount);
extern "C" void AIL_API_set_sample_loop_block(_SAMPLE* pSample, int iLoopStart, int iLoopEnd);
extern "C" int AIL_API_sample_status(_SAMPLE* pSample);
extern "C" float AIL_API_sample_playback_rate_factor(_SAMPLE* pSample);
extern "C" void AIL_API_sample_volume_levels(_SAMPLE* pSample, float* pLeftLevel, float* pRightLevel);
extern "C" void AIL_API_set_digital_master_volume_level(_DIG_DRIVER* pDriver, float fLevel);
extern "C" void AIL_API_set_digital_master_reverb_levels(_DIG_DRIVER* pDriver, int iBus, float fWetLevel,
                                                         float fDryLevel);
extern "C" void AIL_API_set_digital_master_reverb(_DIG_DRIVER* pDriver, int iBus, float fReverbTime,
                                                  float fReverbPredelay, float fReverbDamping);
extern "C" int AIL_API_minimum_sample_buffer_size(_DIG_DRIVER* pDriver, int iPlaybackRate, int iFormat);
extern "C" int AIL_API_sample_buffer_available(_SAMPLE* pSample);
extern "C" int AIL_API_set_sample_buffer_count(_SAMPLE* pSample, int iCount);
extern "C" int AIL_API_load_sample_buffer(_SAMPLE* pSample, int iBuffer, const void* pData, unsigned int uLength);
extern "C" int AIL_API_sample_granularity(_SAMPLE* pSample);
extern "C" void AIL_API_set_sample_position(_SAMPLE* pSample, unsigned int uPosition);
extern "C" unsigned int AIL_API_sample_position(_SAMPLE* pSample);
extern "C" AIL_FALLOFF_CALLBACK AIL_API_register_falloff_function_callback(_SAMPLE* pSample,
                                                                           AIL_FALLOFF_CALLBACK pCallback);
extern "C" _STREAM* AIL_API_open_stream(_DIG_DRIVER* pDriver, const char* pFileName, int iStreamMem);
extern "C" void AIL_API_close_stream(_STREAM* pStream);
extern "C" void AIL_API_start_stream(_STREAM* pStream);
extern "C" void AIL_API_pause_stream(_STREAM* pStream, int iPause);
extern "C" void AIL_API_set_stream_loop_block(_STREAM* pStream, int iLoopStart, int iLoopEnd);
extern "C" void AIL_API_set_stream_loop_count(_STREAM* pStream, int iLoopCount);
extern "C" int AIL_API_stream_status(_STREAM* pStream);
extern "C" void AIL_API_set_stream_position(_STREAM* pStream, int iPosition);
extern "C" int AIL_API_stream_position(_STREAM* pStream);
extern "C" void AIL_API_set_sample_ms_position(_SAMPLE* pSample, unsigned int uMilliseconds);
extern "C" void AIL_API_set_stream_ms_position(_STREAM* pStream, unsigned int uMilliseconds);
extern "C" int AIL_API_WAV_info(const void* pData, _AILSOUNDINFO* pInfo);
extern "C" int MilesEnumerateSoundInstancesInternal(EventSystem* pEventSystem, unsigned int* puNext,
                                                    unsigned int uTypeMask, const char* pLabels, int iEventIndex,
                                                    int iEventSerial, SoundInstanceInfo* pInfo);
extern "C" int MilesBeginEventQueueProcessingInternal();
extern "C" int MilesCompleteEventQueueProcessingInternal();
extern "C" unsigned int MilesStartupEventSystemInternal(_DIG_DRIVER* pDriver, int iEventMemory, int iFlags,
                                                        int iStreamMemory);
extern "C" SoundBank* MilesAddSoundBankInternal(const char* pFileName, const char* pBankName);
extern "C" void AIL_API_set_sample_3D_distances(_SAMPLE* pSample, float fMaxDistance, float fMinDistance,
                                                int iAutomaticWetAttenuation);
extern "C" void AIL_API_set_room_type(_DIG_DRIVER* pDriver, int iBus, int iRoomType);
extern "C" void AIL_API_set_3D_rolloff_factor(_DIG_DRIVER* pDriver, float fRolloffFactor);
extern "C" void AIL_API_set_3D_doppler_factor(_DIG_DRIVER* pDriver, float fDopplerFactor);
extern "C" void AIL_API_set_sample_3D_position(_SAMPLE* pSample, float fX, float fY, float fZ);
extern "C" void AIL_API_set_listener_3D_position(_DIG_DRIVER* pDriver, float fX, float fY, float fZ);
extern "C" void AIL_API_set_sample_3D_volume_falloff(_SAMPLE* pSample, const FalloffGraphPoint* pPoints,
                                                     int iPointCount);
extern "C" void AIL_API_set_sample_3D_lowpass_falloff(_SAMPLE* pSample, const FalloffGraphPoint* pPoints,
                                                      int iPointCount);
extern "C" void AIL_API_set_sample_3D_exclusion_falloff(_SAMPLE* pSample, const FalloffGraphPoint* pPoints,
                                                        int iPointCount);
extern "C" void AIL_API_set_listener_3D_orientation(_DIG_DRIVER* pDriver, float fForwardX, float fForwardY,
                                                    float fForwardZ, float fUpX, float fUpY, float fUpZ);
extern "C" void AIL_API_close_filter(FLT_INSTANCE* pFilter);

#endif

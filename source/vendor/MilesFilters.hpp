#ifndef SOURCE_VENDOR_MILES_FILTERS_HPP
#define SOURCE_VENDOR_MILES_FILTERS_HPP

struct _DIG_DRIVER;
struct _SAMPLE;
struct FLT_INSTANCE;
struct FalloffGraphPoint;

extern FLT_INSTANCE* FLT_list;

extern "C" void FLT_init_list();
extern "C" void FLT_disconnect_driver(_DIG_DRIVER* pDriver);
extern "C" void FLT_call_premix_processors(_DIG_DRIVER* pDriver);
extern "C" int FLT_call_postmix_processors(_DIG_DRIVER* pDriver, short* pBuffer);
extern "C" void AIL_API_close_filter(FLT_INSTANCE* pFilter);

extern "C" void AIL_API_set_room_type(_DIG_DRIVER* pDriver, int iBus, int iRoomType);
extern "C" void AIL_API_set_3D_rolloff_factor(_DIG_DRIVER* pDriver, float fFactor);
extern "C" void AIL_API_set_3D_doppler_factor(_DIG_DRIVER* pDriver, float fFactor);
extern "C" void AIL_API_set_listener_3D_position(_DIG_DRIVER* pDriver, float fX, float fY, float fZ);
extern "C" void AIL_API_set_listener_3D_orientation(_DIG_DRIVER* pDriver, float fFaceX, float fFaceY, float fFaceZ,
                                                    float fUpX, float fUpY, float fUpZ);
extern "C" void AIL_API_set_sample_3D_volume_falloff(_SAMPLE* pSample, const FalloffGraphPoint* pPoints, int iCount);
extern "C" void AIL_API_set_sample_3D_lowpass_falloff(_SAMPLE* pSample, const FalloffGraphPoint* pPoints, int iCount);
extern "C" void AIL_API_set_sample_3D_exclusion_falloff(_SAMPLE* pSample, const FalloffGraphPoint* pPoints, int iCount);
extern "C" void AIL_API_set_sample_3D_position(_SAMPLE* pSample, float fX, float fY, float fZ);
extern "C" void AIL_API_set_sample_3D_distances(_SAMPLE* pSample, float fMaximum, float fMinimum,
                                                int iAutomaticWetAttenuation);

#endif

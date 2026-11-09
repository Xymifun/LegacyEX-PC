#ifndef SOURCE_VENDOR_MILES_MIXER_HPP
#define SOURCE_VENDOR_MILES_MIXER_HPP

#include "vendor/MilesRib.hpp"

struct EVENT_STEP_INFO;

const int MILES_EVENT_HEADER_SIZE = 0x100;
const int MILES_EVENT_BLEND_POINTS = 10;

struct MilesEventString
{
    const char* pText;
    int iLength;
};

struct MilesEventStartStep
{
    unsigned int uType;
    MilesEventString text0;
    MilesEventString text1;
    MilesEventString text2;
    MilesEventString text5;
    MilesEventString text3;
    MilesEventString text4;
    MilesEventString text8;
    MilesEventString text6;
    MilesEventString text7;
    int iValue;
    float fValue[5];
    unsigned short uValue0;
    unsigned short uValue1;
    unsigned char bValue0;
    unsigned char bValue2;
    unsigned char bValue3;
    unsigned char bValue4;
    unsigned char bValue5;
    unsigned char bValue1;
};

struct MilesEventSecondStep
{
    unsigned int uType;
    MilesEventString text0;
    MilesEventString text1;
    MilesEventString text2;
    MilesEventString text3;
    MilesEventString text4;
    float fValue;
    unsigned char bValue0;
    unsigned char bValue1;
    unsigned char bValue2;
};

struct MilesEventThirdStep
{
    unsigned int uType;
    MilesEventString text0;
    unsigned char bValue;
};

struct MilesEventSimpleStep
{
    unsigned int uType;
    MilesEventString text0;
};

struct MilesEventListStep
{
    unsigned int uType;
    MilesEventString name;
    char** ppItems;
    int iCount;
};

struct MilesEventSeventhStep
{
    unsigned int uType;
    MilesEventString text0;
    MilesEventString text1;
};

struct MilesEventEighthStep
{
    unsigned int uType;
    MilesEventString text0;
    MilesEventString text1;
    MilesEventString text2;
    unsigned char bValue;
};

struct MilesEventRampStep
{
    unsigned int uType;
    MilesEventString text0;
    MilesEventString text1;
    MilesEventString text2;
    float fValue;
    unsigned char bValue0;
    unsigned char bValue1;
    unsigned char bValue2;
};

struct MilesEventBlendStep
{
    unsigned int uType;
    MilesEventString name;
    float fRangeStart[MILES_EVENT_BLEND_POINTS];
    float fFadeInEnd[MILES_EVENT_BLEND_POINTS];
    float fFadeOutStart[MILES_EVENT_BLEND_POINTS];
    float fRangeEnd[MILES_EVENT_BLEND_POINTS];
    float fPitchAtStart[MILES_EVENT_BLEND_POINTS];
    float fPitchAtEnd[MILES_EVENT_BLEND_POINTS];
    unsigned char uPointCount;
};

struct MilesEventWaveStep
{
    unsigned int uType;
    MilesEventString text0;
    MilesEventString text1;
    MilesEventString text2;
    MilesEventString text3;
    int iValue0;
    int iValue1;
    int iValue2;
    int iValue3;
    int iValue4;
};

struct MilesEventLastStep
{
    unsigned int uType;
    MilesEventString text0;
    float fValue0;
    float fValue1;
    int iValue0;
    int iValue1;
    float fValue2;
    float fValue3;
    float fValue4;
};

union MilesEventStep
{
    unsigned int uType;
    MilesEventStartStep start;
    MilesEventSecondStep second;
    MilesEventThirdStep third;
    MilesEventSimpleStep fourth;
    MilesEventListStep list;
    MilesEventSeventhStep seventh;
    MilesEventEighthStep eighth;
    MilesEventRampStep ramp;
    MilesEventBlendStep blend;
    MilesEventSimpleStep simple;
    MilesEventWaveStep wave;
    MilesEventLastStep last;
    unsigned char header[MILES_EVENT_HEADER_SIZE];
};

int MSS_mixer_property(unsigned int uProperty, void* pBefore, const void* pNewValue, void* pAfter);

extern "C" void MSS_mixer_startup();
extern "C" void MSS_mixer_shutdown();
extern "C" void MSS_mixer_flush(void* pDestination, int iLength);

extern "C" void MSS_mixer_merge(void*, unsigned int*, unsigned short*, void**, unsigned short*, int*, int*,
                                unsigned int, int, int, unsigned int);
extern "C" void MSS_mixer_copy(void* pSource, unsigned int uLength, unsigned short* pDestination, unsigned int uFlags,
                               int iSuppressSwap);
extern "C" void MSS_mixer_mc_copy(void*, int, void*, unsigned int, unsigned int, int);
extern "C" void MSS_mixer_adpcm_decode(unsigned int*, unsigned int*, int, int, unsigned int, int*);

extern "C" int MIX_RIB_MAIN(unsigned int uProvider, int iUpDown,
                            RIB_ALLOC_PROVIDER_HANDLE_CALLBACK pAllocProviderHandle,
                            RIB_REGISTER_INTERFACE_CALLBACK pRegisterInterface,
                            RIB_UNREGISTER_INTERFACE_CALLBACK pUnregisterInterface);

extern "C" const unsigned char* AIL_next_event_step(const unsigned char* pEvent, EVENT_STEP_INFO** ppStep,
                                                    EVENT_STEP_INFO* pBuffer, unsigned int uBufferSize);
extern "C" int get_system_speaker_configuration(unsigned int* puConfiguration);

#endif

#include "game/util/LegacyUtilityFunctions.hpp"
#include "vendor/MilesApi.hpp"
#include "vendor/MilesMem.hpp"
#include "vendor/MilesRib.hpp"

extern "C" unsigned int RIB_find_file_provider(const char* pInterfaceName, const char* pAttributeName,
                                               const char* pFileName);

static const char ASI_CODEC_INTERFACE[] = "ASI codec";
static const char PROVIDER_PROPERTY_ENTRY[] = "PROVIDER_property";
static const char OUT_OF_MEMORY_ERROR[] = "Out of memory.";
static const unsigned int RIB_FUNCTION = 0;

extern "C" void outresfloat(float fResult)
{
    AIL_debug_log("!-Result = %f", fResult);
}

extern "C" void outresint64(uint32_t low, uint32_t high)
{
    AIL_debug_log("!-Result = %llu", high, low, high);
}

extern "C" void outresuint(uint32_t value)
{
    AIL_debug_log("!-Result = %ld", value);
}

char ourtoupper(char value)
{
    if (value >= 'a' && value <= 'z')
    {
        return value - ('a' - 'A');
    }
    return value;
}

int OnlyNum(const char* value)
{
    const char* start = value;
    while (*value != '\0')
    {
        if ((value != start || *value != '-')
            && (static_cast<unsigned int>(static_cast<char>(*value + static_cast<unsigned char>(-'0'))) >= 10
                && *value != '.'))
        {
            return 0;
        }
        ++value;
    }
    return 1;
}

extern "C" int AIL_stricmp(const char* first, const char* second)
{
    for (;;)
    {
        int firstChar = *first;
        firstChar = ourtoupper(firstChar);
        int secondChar = *second;
        secondChar = ourtoupper(secondChar);
        if (firstChar < secondChar)
        {
            return -1;
        }
        if (firstChar > secondChar)
        {
            return 1;
        }
        if (firstChar == 0)
        {
            break;
        }
        ++first;
        ++second;
    }
    return 0;
}

extern "C" int AIL_strnicmp(const char* first, const char* second, int count)
{
    for (; count != 0; --count)
    {
        int firstChar = *first;
        firstChar = ourtoupper(firstChar);
        int secondChar = *second;
        secondChar = ourtoupper(secondChar);
        if (firstChar < secondChar)
        {
            return -1;
        }
        if (firstChar > secondChar)
        {
            return 1;
        }
        if (firstChar == 0)
        {
            break;
        }
        ++first;
        ++second;
    }
    return 0;
}

extern "C" unsigned int find_ASI_provider(const char* attributeName, const char* fileName)
{
    unsigned int provider = RIB_find_file_provider(ASI_CODEC_INTERFACE, attributeName, fileName);
    if (provider != 0)
    {
        void* propertyToken = NULL;
        RIB_INTERFACE_ENTRY entry;
        entry.uType = RIB_FUNCTION;
        entry.pEntryName = PROVIDER_PROPERTY_ENTRY;
        entry.pToken = &propertyToken;
        entry.uSubType = 0;
        if (RIB_request_interface(provider, ASI_CODEC_INTERFACE, 1, &entry) != 0)
        {
            AIL_set_error(OUT_OF_MEMORY_ERROR);
            provider = 0;
        }
    }
    return provider;
}

extern "C" void AIL_set_sample_speaker_scale_factors(_SAMPLE* pSample, const int* pSpeakers, const float* pScales,
                                                     int iCount)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_speaker_scale_factors(0x%lX,0x%lX,0x%lX, %d)", pSample, pSpeakers, pScales,
                      iCount);
    }

    InMilesMutex();
    AIL_API_set_sample_speaker_scale_factors(pSample, pSpeakers, pScales, iCount);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_sample_speaker_scale_factors(_SAMPLE* pSample, int* pSpeakers, float* pScales, int iCount)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_sample_speaker_scale_factors(0x%lX,0x%lX,0x%lX, %d)", pSample, pSpeakers, pScales, iCount);
    }

    InMilesMutex();
    AIL_API_sample_speaker_scale_factors(pSample, pSpeakers, pScales, iCount);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_sample_channel_levels(_SAMPLE* pSample, int* pSources, int* pDestinations, float* pLevels,
                                          int iCount)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_sample_channel_levels(0x%lX,0x%lX,0x%lX,0x%lX,%d)", pSample, pSources, pDestinations,
                      pLevels, iCount);
    }

    InMilesMutex();
    AIL_API_sample_channel_levels(pSample, pSources, pDestinations, pLevels, iCount);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_sample_channel_levels(_SAMPLE* pSample, const int* pSources, const int* pDestinations,
                                              const float* pLevels, int iCount)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_channel_levels(0x%lX,0x%lX,0x%lX,0x%lX,%d)", pSample, pSources, pDestinations,
                      pLevels, iCount);
    }

    InMilesMutex();
    AIL_API_set_sample_channel_levels(pSample, pSources, pDestinations, pLevels, iCount);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_sample_reverb_levels(_SAMPLE* pSample, float fFirst, float fSecond)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_reverb_levels(0x%lX,%f,%f)", pSample, fFirst, fSecond);
    }

    InMilesMutex();
    AIL_API_set_sample_reverb_levels(pSample, fFirst, fSecond);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_sample_low_pass_cut_off(_SAMPLE* pSample, int iChannel, float fCutOff)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_low_pass_cut_off(0x%lX,%d,%f)", pSample, iChannel, fCutOff);
    }

    InMilesMutex();
    AIL_API_set_sample_low_pass_cut_off(pSample, iChannel, fCutOff);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_sample_51_volume_pan(_SAMPLE* pSample, float fVolume, float fPan, float fFade, float fCenter,
                                             float fLowFrequency)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_51_volume_pan(0x%lX,%f,%f,%f,%f,%f)", pSample, fVolume, fPan, fFade, fCenter,
                      fLowFrequency);
    }

    InMilesMutex();
    AIL_API_set_sample_51_volume_pan(pSample, fVolume, fPan, fFade, fCenter, fLowFrequency);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" int AIL_sample_processor(_SAMPLE* pSample, int iStage)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_sample_processor(0x%lX,%ld)", pSample, iStage);
    }

    InMilesMutex();
    iResult = AIL_API_sample_processor(pSample, iStage);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" int AIL_set_sample_processor(_SAMPLE* pSample, int iStage, int iProvider)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_processor(0x%lX,%ld,%ld)", pSample, iStage, iProvider);
    }

    InMilesMutex();
    iResult = AIL_API_set_sample_processor(pSample, iStage, iProvider);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" int AIL_install_bus_compressor(_DIG_DRIVER* pDriver, int iBus, int iStage, int iMode)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_install_bus_compressor(0x%lX,%d,%d,%d)", pDriver, iBus, iStage, iMode);
    }

    InMilesMutex();
    iResult = AIL_API_install_bus_compressor(pDriver, iBus, iStage, iMode);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" int AIL_open_filter(unsigned int uProvider, _DIG_DRIVER* pDriver)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_open_filter(%ld,0x%lX)", uProvider, pDriver);
    }

    InMilesMutex();
    iResult = AIL_API_open_filter(uProvider, pDriver);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" int AIL_sample_channel_count(_SAMPLE* pSample, unsigned int* puMask)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_sample_channel_count(0x%lX,0x%lX)", pSample, puMask);
    }

    iResult = AIL_API_sample_channel_count(pSample, puMask);

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" int AIL_sample_playback_rate(_SAMPLE* pSample)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_sample_playback_rate(0x%lX)", pSample);
    }

    iResult = AIL_API_sample_playback_rate(pSample);

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" void AIL_set_sample_playback_rate(_SAMPLE* pSample, int iRate)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_playback_rate(0x%lX,%ld)", pSample, iRate);
    }

    InMilesMutex();
    AIL_API_set_sample_playback_rate(pSample, iRate);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

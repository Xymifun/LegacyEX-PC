#ifndef SOURCE_VENDOR_MILES_GENERIC_HPP
#define SOURCE_VENDOR_MILES_GENERIC_HPP

#include "game/client/audio/FalloffGraph.hpp"
#include "vendor/MilesApi.hpp"
#include "vendor/MilesRadSS.hpp"

enum MSS_SPEAKER_CONFIGURATION
{
    MSS_MC_MONO = 0x01,
    MSS_MC_STEREO = 0x02,
    MSS_MC_USE_SYSTEM_CONFIG = 0x10,
    MSS_MC_HEADPHONES = 0x20,
    MSS_MC_DOLBY_SURROUND = 0x30,
    MSS_MC_SRS_CIRCLE_SURROUND = 0x40,
    MSS_MC_40_DTS = 0x48,
    MSS_MC_40_DISCRETE = 0x50,
    MSS_MC_51_DTS = 0x58,
    MSS_MC_51_DISCRETE = 0x60,
    MSS_MC_61_DISCRETE = 0x70,
    MSS_MC_71_DISCRETE = 0x80,
    MSS_MC_81_DISCRETE = 0x90,
    MSS_MC_DIRECTSOUND3D = 0xA0,
    MSS_MC_EAX2 = 0xC0,
    MSS_MC_EAX3 = 0xD0,
    MSS_MC_EAX4 = 0xE0
};

#ifdef MINECRAFT_PC
const int DIG_DRIVER_SIZE = sizeof(_DIG_DRIVER);
#else
const int DIG_DRIVER_SIZE = 4392;
#endif
const unsigned int DIG_DRIVER_MAGIC = ('H' << 24) | ('D' << 16) | ('I' << 8) | 'G';
const unsigned int SAMPLE_MAGIC = ('H' << 24) | ('S' << 16) | ('A' << 8) | 'M';

struct MilesSpeakerPosition
{
    float fX;
    float fY;
    float fZ;
};

extern _DIG_DRIVER* MSS_first_digital_driver;
extern "C" void AIL_set_error(const char* pError);
extern "C" void AIL_API_close_digital_driver(_DIG_DRIVER* pDriver);
extern "C" int AIL_get_preference(int iPreference);
extern "C" unsigned int AIL_us_count();
extern "C" void SS_start_DIG_driver_playback(_SAMPLE* pSample);
extern "C" void SS_fill(_DIG_DRIVER* pDriver, short* pDestination);
extern "C" int AIL_allocate_reverb_buffers(_DIG_DRIVER* pDriver, int iBus);
void SS_serve(unsigned int uDriver);

#endif

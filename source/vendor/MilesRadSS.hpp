#ifndef SOURCE_VENDOR_MILES_RADSS_HPP
#define SOURCE_VENDOR_MILES_RADSS_HPP

#include <sndcore2/voice.h>

#include "vendor/MilesApi.hpp"


struct _RadSoundSystem;

enum RADSS_PROPERTY
{
    RADSS_PROPERTY_AX_QUERY = 5000
};

typedef void (*RADSS_SET_MEMORY_FUNCTION)(_RadSoundSystem* pSystem, void* pMemory);
typedef int (*RADSS_GET_READY_BYTES_FUNCTION)(_RadSoundSystem* pSystem);
typedef int (*RADSS_SET_PAUSED_FUNCTION)(_RadSoundSystem* pSystem, int iPaused);
typedef int (*RADSS_SET_ON_OFF_FUNCTION)(_RadSoundSystem* pSystem, int iOn);
typedef int (*RADSS_CLOSE_FUNCTION)(_RadSoundSystem* pSystem);
typedef int (*RADSS_LOCK_FUNCTION)(_RadSoundSystem* pSystem, void** ppBuffer, unsigned int uBytes);
typedef int (*RADSS_UNLOCK_FUNCTION)(_RadSoundSystem* pSystem);
typedef unsigned int (*RADSS_GET_PROPERTY_FUNCTION)(_RadSoundSystem* pSystem, RADSS_PROPERTY property,
                                                    unsigned int* puValue);
typedef void (*RADSS_VOLUME_FUNCTION)(_RadSoundSystem* pSystem, float fFirst, float fSecond);
typedef void (*RADSS_CHANNEL_ROUTE_FUNCTION)(_RadSoundSystem* pSystem, short* pRouting, int iCount);

static const int RADSS_AX_VOICE_COUNT = 8;
static const int RADSS_AX_ROUTING_ENTRIES = 64;
static const unsigned int RADSS_AX_ROUTING_BYTES = 0x80;

struct RadSS_WiiUPrivate
{
    AXVoice* pVoices[RADSS_AX_VOICE_COUNT];
    void* pChannelBuffers[RADSS_AX_VOICE_COUNT];
    void* pStaging;
    short* pRouting;
    unsigned int uLockBytes;
    unsigned int uLastTick;
    unsigned int uFreshTicks;
    unsigned int uPollTicks;
    unsigned int uReadyLimit;
    unsigned int uChunkBytes;
    unsigned int uBufferBytes;
    unsigned int uChunkStride;
    unsigned int uStagingBytes;
    unsigned int uPlayOffset;
    unsigned int uWriteOffset;
    unsigned short usLatency;
    short sVolumeFirst;
    short sVolumeSecond;
    unsigned char ucChunkCount;
    unsigned char ucFlags;
};

struct _RadSoundSystem
{
    const char* pError;
    RADSS_SET_MEMORY_FUNCTION pSetMemory;
    RADSS_GET_READY_BYTES_FUNCTION pGetReadyBytes;
    RADSS_SET_PAUSED_FUNCTION pSetPaused;
    RADSS_SET_ON_OFF_FUNCTION pSetOnOff;
    RADSS_CLOSE_FUNCTION pClose;
    RADSS_LOCK_FUNCTION pLock;
    RADSS_UNLOCK_FUNCTION pUnlock;
    RADSS_GET_PROPERTY_FUNCTION pGetProperty;
    RADSS_VOLUME_FUNCTION pVolume;
    RADSS_CHANNEL_ROUTE_FUNCTION pChannelRoute;
    void* pMemory;
    unsigned int uMemoryBytes;
    unsigned char ucReserved34;
    unsigned char ucChannels;
    unsigned char ucPaused;
    unsigned char ucEnabled;
    unsigned short usRate;
    unsigned short usUnderruns;
    unsigned int uReserved3C;
    RadSS_WiiUPrivate priv;
};

#ifdef MINECRAFT_PC
static const unsigned int RADSS_GENERIC_SIZE = sizeof(_RadSoundSystem);
#else
static const unsigned int RADSS_GENERIC_SIZE = 0xB8;
#endif


#endif

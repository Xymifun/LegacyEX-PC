#include "vendor/MilesRadSS.hpp"

#include <stddef.h>
#include <string.h>

#include <coreinit/cache.h>
#include <coreinit/systeminfo.h>
#include <coreinit/time.h>
#include <sndcore2/core.h>

static const short RADSS_UNITY_VOLUME = 0x7FFF;
static const float RADSS_VOLUME_SCALE = 32767.0f;
static const float RADSS_REFERENCE_RATE = 32000.0f;

static const int RADSS_ROUTING_STRIDE = 8;

static const unsigned char RADSS_FLAG_MODE_MASK = 3;
static const unsigned char RADSS_FLAG_ROUTED = 4;
static const unsigned char RADSS_FLAG_PLAYING = 8;

static const unsigned int RADSS_MODE_STEREO = 0;
static const unsigned int RADSS_MODE_SURROUND = 1;
static const unsigned int RADSS_MODE_MATRIX = 3;

static const unsigned int RADSS_VOICE_PRIORITY = 0x1F;
static const unsigned int RADSS_TV_MIX_COUNT = 6;
static const unsigned int RADSS_DRC_MIX_COUNT = 4;
static const int RADSS_VOLUME_SHIFT = 15;
static const unsigned short RADSS_VOICE_VOLUME = 0x8000;

static const unsigned int RADSS_MAXIMUM_CHUNK_MILLISECONDS = 24;
static const unsigned int RADSS_MINIMUM_CHUNKS = 8;
static const unsigned int RADSS_SAMPLE_ALIGNMENT_MASK = 0xFFFFFFF0;
static const unsigned int RADSS_BYTE_ALIGNMENT_MASK = 0xFFFFFFE0;
static const unsigned int RADSS_MILLISECONDS_PER_SECOND = 1000;
static const unsigned int RADSS_FRESH_TICK_DIVISOR = 4;
static const unsigned int RADSS_READY_LIMIT_NUMERATOR = 9;
static const unsigned int RADSS_READY_LIMIT_DENOMINATOR = 10;
static const unsigned int RADSS_MINIMUM_READY_BYTES = 0x400;

static const unsigned int RADSS_AX_QUERY_RESULT = 9;

static const char RADSS_INVALID_LOCK_SIZE[] = "Invalid lock size.";
static const char RADSS_AX_NOT_INITIALIZED[] = "AX library isn't initialized.";
static const char RADSS_NO_VOICES[] = "Couldn't acquire voices.";

static unsigned int RadSS_defaultMode = 0;

static int RADSS_AXOpenDriver(_RadSoundSystem* pSystem, unsigned int uLatencyMilliseconds, unsigned int uRate,
                              unsigned int uChannels, unsigned int uLockBytes, unsigned int uMode);

static void ClearBuffer(RadSS_WiiUPrivate* pPrivate, int iChannels, unsigned int uOffset, unsigned int uCount)
{
    unsigned int uBufferBytes;
    unsigned int uChunk;
    unsigned int uEnd;
    int iChannel;

    uBufferBytes = pPrivate->uBufferBytes;

    while (uCount != 0)
    {
        uChunk = uCount;
        uEnd = uOffset + uCount;

        if (uBufferBytes < uEnd)
        {
            uChunk = uBufferBytes - uOffset;
            uEnd = uBufferBytes;
        }

        for (iChannel = 0; iChannel < iChannels; iChannel++)
        {
            memset(static_cast<unsigned char*>(pPrivate->pChannelBuffers[iChannel]) + uOffset, 0, uChunk);
            DCFlushRangeNoSync(static_cast<unsigned char*>(pPrivate->pChannelBuffers[iChannel]) + uOffset, uChunk);
        }

        uCount = uCount - uChunk;
        uOffset = uEnd;

        if (uEnd == uBufferBytes)
        {
            uOffset = 0;
        }
    }
}

static void InitRouting(int iMode, int iChannels, short* pRouting)
{
    if (iChannels == 1)
    {
        if (iMode == RADSS_MODE_STEREO || iMode == RADSS_MODE_MATRIX)
        {
            pRouting[0 * RADSS_ROUTING_STRIDE] = RADSS_UNITY_VOLUME;
            pRouting[1 * RADSS_ROUTING_STRIDE] = RADSS_UNITY_VOLUME;
            pRouting[5 * RADSS_ROUTING_STRIDE] = RADSS_UNITY_VOLUME;
            pRouting[3 * RADSS_ROUTING_STRIDE] = RADSS_UNITY_VOLUME;
            pRouting[2 * RADSS_ROUTING_STRIDE] = RADSS_UNITY_VOLUME;
            pRouting[4 * RADSS_ROUTING_STRIDE] = RADSS_UNITY_VOLUME;
        }

        if (iMode == RADSS_MODE_MATRIX || iMode == RADSS_MODE_SURROUND)
        {
            pRouting[6 * RADSS_ROUTING_STRIDE] = RADSS_UNITY_VOLUME;
            pRouting[7 * RADSS_ROUTING_STRIDE] = RADSS_UNITY_VOLUME;
        }

        return;
    }

    if (iMode == RADSS_MODE_MATRIX && iChannels == 4)
    {
        pRouting[1 * RADSS_ROUTING_STRIDE + 1] = RADSS_UNITY_VOLUME;
        pRouting[6 * RADSS_ROUTING_STRIDE + 2] = RADSS_UNITY_VOLUME;
        pRouting[0 * RADSS_ROUTING_STRIDE + 0] = RADSS_UNITY_VOLUME;
        pRouting[7 * RADSS_ROUTING_STRIDE + 3] = RADSS_UNITY_VOLUME;
        return;
    }

    if (iMode == RADSS_MODE_MATRIX || iMode == RADSS_MODE_STEREO)
    {
        pRouting[1 * RADSS_ROUTING_STRIDE + 1] = RADSS_UNITY_VOLUME;
        pRouting[5 * RADSS_ROUTING_STRIDE + 5] = RADSS_UNITY_VOLUME;
        pRouting[3 * RADSS_ROUTING_STRIDE + 3] = RADSS_UNITY_VOLUME;
        pRouting[4 * RADSS_ROUTING_STRIDE + 4] = RADSS_UNITY_VOLUME;
        pRouting[0 * RADSS_ROUTING_STRIDE + 0] = RADSS_UNITY_VOLUME;
        pRouting[2 * RADSS_ROUTING_STRIDE + 2] = RADSS_UNITY_VOLUME;
    }

    if (iMode == RADSS_MODE_SURROUND && iChannels == 2)
    {
        pRouting[7 * RADSS_ROUTING_STRIDE + 1] = RADSS_UNITY_VOLUME;
        pRouting[6 * RADSS_ROUTING_STRIDE + 0] = RADSS_UNITY_VOLUME;
        return;
    }

    if (iChannels == 7)
    {
        pRouting[6 * RADSS_ROUTING_STRIDE + 6] = RADSS_UNITY_VOLUME;
        pRouting[7 * RADSS_ROUTING_STRIDE + 6] = RADSS_UNITY_VOLUME;
    }
    else if (iChannels == 8)
    {
        pRouting[6 * RADSS_ROUTING_STRIDE + 6] = RADSS_UNITY_VOLUME;
        pRouting[7 * RADSS_ROUTING_STRIDE + 7] = RADSS_UNITY_VOLUME;
    }
}

static int RADSS_AXCloseDriver(_RadSoundSystem* pSystem)
{
    RadSS_WiiUPrivate* pPrivate = &pSystem->priv;
    int iVoice;

    for (iVoice = 0; iVoice < pSystem->ucChannels; iVoice++)
    {
        if (pPrivate->pVoices[iVoice] != NULL)
        {
            pPrivate->pVoices[iVoice]->callback = NULL;
            pPrivate->pVoices[iVoice]->callbackEx = NULL;
            AXSetVoiceState(pPrivate->pVoices[iVoice], AX_VOICE_STATE_STOPPED);
            AXFreeVoice(pPrivate->pVoices[iVoice]);
            pPrivate->pVoices[iVoice] = NULL;
        }
    }

    return 1;
}

static int RADSS_AXGetReadyBytes(_RadSoundSystem* pSystem)
{
    AXVoiceOffsets offsets;
    unsigned int uPosition;
    unsigned int uStart;
    unsigned int uNow;
    unsigned int uReady;
    int bFresh;

    if (pSystem->ucEnabled == 0 || pSystem->ucPaused != 0)
    {
        return 0;
    }

    uPosition = pSystem->priv.uPlayOffset;
    uNow = OSGetTick();
    uStart = pSystem->priv.uLastTick;
    bFresh = (uNow - uStart) <= pSystem->priv.uFreshTicks;

    for (;;)
    {
        if (bFresh == 0)
        {
            AXGetVoiceOffsets(pSystem->priv.pVoices[0], &offsets);
            uPosition = offsets.currentOffset << 1;

            if (pSystem->priv.uBufferBytes < uPosition)
            {
                uPosition = pSystem->priv.uPlayOffset;
            }
            else
            {
                pSystem->priv.uPlayOffset = uPosition;
            }

            uNow = OSGetTick();
            pSystem->priv.uLastTick = uNow;
            bFresh = (uNow - uStart) <= pSystem->priv.uPollTicks;
        }

        if (bFresh != 0)
        {
            if (uPosition < pSystem->priv.uWriteOffset)
            {
                uPosition = uPosition + pSystem->priv.uBufferBytes;
            }

            uPosition = uPosition - pSystem->priv.uWriteOffset;
            uReady = 0;

            if (uPosition >= RADSS_MINIMUM_READY_BYTES)
            {
                uReady = (uPosition - RADSS_MINIMUM_READY_BYTES) & 0xFFFFFFFE;
            }

            if (uReady < pSystem->priv.uReadyLimit)
            {
                return uReady * pSystem->ucChannels;
            }
        }

        if ((pSystem->priv.ucFlags & RADSS_FLAG_PLAYING) != 0)
        {
            pSystem->usUnderruns = pSystem->usUnderruns + 1;
        }

        ClearBuffer(&pSystem->priv, pSystem->ucChannels, pSystem->priv.uWriteOffset, pSystem->priv.uChunkBytes);

        uPosition = pSystem->priv.uWriteOffset + pSystem->priv.uChunkBytes;
        uStart = pSystem->priv.uLastTick;
        pSystem->priv.uWriteOffset = uPosition;

        if (pSystem->priv.uBufferBytes <= uPosition)
        {
            pSystem->priv.uWriteOffset = uPosition - pSystem->priv.uBufferBytes;
        }

        bFresh = 0;
    }
}

static void RADSS_AXSetMem(_RadSoundSystem* pSystem, void* pMemory)
{
    AXVoiceOffsets offsets;
    int iVoice;

    pSystem->pMemory = pMemory;

    for (iVoice = 0; iVoice < pSystem->ucChannels; iVoice++)
    {
        if (pSystem->priv.pVoices[iVoice] != NULL)
        {
            pSystem->priv.pChannelBuffers[iVoice] = pMemory;

            offsets.dataType = AX_VOICE_FORMAT_LPCM16;
            offsets.loopingEnabled = AX_VOICE_LOOP_ENABLED;
            offsets.loopOffset = 0;
            offsets.currentOffset = 0;
            offsets.endOffset = (pSystem->priv.uBufferBytes >> 1) - 1;
            offsets.data = pSystem->priv.pChannelBuffers[iVoice];

            pMemory = static_cast<unsigned char*>(pMemory) + pSystem->priv.uBufferBytes;

            AXSetVoiceOffsets(pSystem->priv.pVoices[iVoice], &offsets);
        }
    }

    pSystem->priv.pStaging = pMemory;

    ClearBuffer(&pSystem->priv, pSystem->ucChannels, 0, pSystem->priv.uBufferBytes);

    pSystem->priv.pRouting = reinterpret_cast<short*>(static_cast<unsigned char*>(pMemory)
                                                      + pSystem->priv.uStagingBytes);
    pSystem->priv.uWriteOffset = pSystem->priv.uChunkBytes;
}

static void RADSS_AXChannelRoute(_RadSoundSystem* pSystem, short* pRouting, int iCount)
{
    AXVoiceDeviceMixData tvMix[RADSS_TV_MIX_COUNT] = {};
    AXVoiceDeviceMixData drcMix[RADSS_DRC_MIX_COUNT] = {};
    unsigned int uVolumeFirst;
    unsigned int uVolumeSecond;
    unsigned int uVolumeCenter;
    int iChannels;
    int iVoice;
    int iRow;

    iChannels = pSystem->ucChannels;

    if (iCount == RADSS_AX_ROUTING_ENTRIES)
    {
        memcpy(pSystem->priv.pRouting, pRouting, RADSS_AX_ROUTING_BYTES);
    }
    else
    {
        if (iCount != iChannels * RADSS_ROUTING_STRIDE)
        {
            return;
        }

        memset(pSystem->priv.pRouting, 0, RADSS_AX_ROUTING_BYTES);

        for (iVoice = 0; iVoice < RADSS_AX_VOICE_COUNT && iVoice < iChannels; iVoice++)
        {
            for (iRow = 0; iRow < RADSS_ROUTING_STRIDE; iRow++)
            {
                pSystem->priv.pRouting[iRow * RADSS_ROUTING_STRIDE + iVoice] = pRouting[iRow * iChannels + iVoice];
            }
        }
    }

    if ((pSystem->priv.ucFlags & RADSS_FLAG_ROUTED) == 0)
    {
        pSystem->priv.sVolumeSecond = RADSS_UNITY_VOLUME;
        pSystem->priv.sVolumeFirst = RADSS_UNITY_VOLUME;
    }

    uVolumeSecond = pSystem->priv.sVolumeSecond;
    uVolumeFirst = pSystem->priv.sVolumeFirst;
    pSystem->priv.ucFlags = pSystem->priv.ucFlags | RADSS_FLAG_ROUTED;

    uVolumeCenter = uVolumeSecond;

    if (uVolumeSecond < uVolumeFirst)
    {
        uVolumeCenter = uVolumeFirst;
    }

    for (iVoice = 0; iVoice < iChannels; iVoice++)
    {
        tvMix[0].bus[0].volume = (pSystem->priv.pRouting[0 * RADSS_ROUTING_STRIDE + iVoice] * uVolumeFirst)
                                 >> RADSS_VOLUME_SHIFT;
        tvMix[1].bus[0].volume = (pSystem->priv.pRouting[1 * RADSS_ROUTING_STRIDE + iVoice] * uVolumeSecond)
                                 >> RADSS_VOLUME_SHIFT;
        tvMix[4].bus[0].volume = (pSystem->priv.pRouting[2 * RADSS_ROUTING_STRIDE + iVoice] * uVolumeCenter)
                                 >> RADSS_VOLUME_SHIFT;
        tvMix[5].bus[0].volume = (pSystem->priv.pRouting[3 * RADSS_ROUTING_STRIDE + iVoice] * uVolumeCenter)
                                 >> RADSS_VOLUME_SHIFT;
        tvMix[2].bus[0].volume = (pSystem->priv.pRouting[4 * RADSS_ROUTING_STRIDE + iVoice] * uVolumeFirst)
                                 >> RADSS_VOLUME_SHIFT;
        tvMix[3].bus[0].volume = (pSystem->priv.pRouting[5 * RADSS_ROUTING_STRIDE + iVoice] * uVolumeSecond)
                                 >> RADSS_VOLUME_SHIFT;
        drcMix[0].bus[0].volume = (pSystem->priv.pRouting[6 * RADSS_ROUTING_STRIDE + iVoice] * uVolumeFirst)
                                  >> RADSS_VOLUME_SHIFT;
        drcMix[1].bus[0].volume = (pSystem->priv.pRouting[7 * RADSS_ROUTING_STRIDE + iVoice] * uVolumeSecond)
                                  >> RADSS_VOLUME_SHIFT;

        AXSetVoiceDeviceMix(pSystem->priv.pVoices[iVoice], AX_DEVICE_TYPE_TV, 0, tvMix);
        AXSetVoiceDeviceMix(pSystem->priv.pVoices[iVoice], AX_DEVICE_TYPE_DRC, 0, drcMix);
    }
}

static void RADSS_AXVolume(_RadSoundSystem* pSystem, float fFirst, float fSecond)
{
    unsigned char ucFlags;

    ucFlags = pSystem->priv.ucFlags;

    pSystem->priv.sVolumeSecond = (short)(fSecond * RADSS_VOLUME_SCALE);
    pSystem->priv.sVolumeFirst = (short)(fFirst * RADSS_VOLUME_SCALE);

    if ((ucFlags & RADSS_FLAG_ROUTED) == 0)
    {
        memset(pSystem->priv.pRouting, 0, RADSS_AX_ROUTING_BYTES);
        InitRouting(pSystem->priv.ucFlags & RADSS_FLAG_MODE_MASK, pSystem->ucChannels, pSystem->priv.pRouting);
        ucFlags = pSystem->priv.ucFlags;
    }

    pSystem->priv.ucFlags = ucFlags | RADSS_FLAG_ROUTED;
    RADSS_AXChannelRoute(pSystem, pSystem->priv.pRouting, RADSS_AX_ROUTING_ENTRIES);
}

static int RADSS_AXSetOnOff(_RadSoundSystem* pSystem, int iOn)
{
    void* pMemory;
    unsigned char ucFlags;
    short sVolumeFirst;
    short sVolumeSecond;

    if (iOn == 0)
    {
        RADSS_AXCloseDriver(pSystem);
    }
    else
    {
        ucFlags = pSystem->priv.ucFlags;
        sVolumeFirst = pSystem->priv.sVolumeFirst;
        pMemory = pSystem->pMemory;
        sVolumeSecond = pSystem->priv.sVolumeSecond;

        if (RADSS_AXOpenDriver(pSystem, pSystem->priv.usLatency, pSystem->usRate, pSystem->ucChannels,
                               pSystem->priv.uStagingBytes, ucFlags & RADSS_FLAG_MODE_MASK)
            == 0)
        {
            return 0;
        }

        RADSS_AXSetMem(pSystem, pMemory);

        if ((ucFlags & RADSS_FLAG_ROUTED) != 0)
        {
            RADSS_AXChannelRoute(pSystem, pSystem->priv.pRouting, RADSS_AX_ROUTING_ENTRIES);
            RADSS_AXVolume(pSystem, (float)sVolumeFirst / RADSS_VOLUME_SCALE,
                           (float)sVolumeSecond / RADSS_VOLUME_SCALE);
        }
    }

    return 1;
}

static int RADSS_AXSetPaused(_RadSoundSystem* pSystem, int iPaused)
{
    int iVoice;

    if ((unsigned int)iPaused != pSystem->ucPaused)
    {
        if ((pSystem->priv.ucFlags & RADSS_FLAG_PLAYING) != 0)
        {
            for (iVoice = 0; iVoice < pSystem->ucChannels; iVoice++)
            {
                if (pSystem->priv.pVoices[iVoice] != NULL)
                {
                    AXSetVoiceState(pSystem->priv.pVoices[iVoice],
                                    iPaused == 0 ? AX_VOICE_STATE_PLAYING : AX_VOICE_STATE_STOPPED);
                }
            }
        }

        pSystem->ucPaused = (unsigned char)iPaused;

        if (iPaused == 0)
        {
            pSystem->priv.uLastTick = OSGetTick() - pSystem->priv.uFreshTicks;
        }
    }

    return 1;
}

static int RADSS_AXLock(_RadSoundSystem* pSystem, void** ppBuffer, unsigned int uBytes)
{

    if (uBytes > pSystem->priv.uStagingBytes)
    {
        pSystem->pError = "Invalid lock size.";
        return 0;
    }

    *ppBuffer = pSystem->priv.pStaging;
    pSystem->priv.uLockBytes = uBytes;
    return 1;
}

static int RADSS_AXUnlock(_RadSoundSystem* pSystem)
{
    const short* pSource;
    short* pDestination;
    unsigned int uChannels;
    unsigned int uRemaining;
    unsigned int uOffset;
    unsigned int uChunk;
    unsigned int uSamples;
    unsigned int uSample;
    unsigned int uChannel;

    uChannels = pSystem->ucChannels;
    pSource = static_cast<const short*>(pSystem->priv.pStaging);
    uRemaining = pSystem->priv.uLockBytes / uChannels;
    uOffset = pSystem->priv.uWriteOffset;

    while (uRemaining != 0)
    {
        uChunk = uRemaining;

        if (pSystem->priv.uBufferBytes < uOffset + uRemaining)
        {
            uChunk = pSystem->priv.uBufferBytes - uOffset;
        }

        uSamples = uChunk >> 1;

        for (uChannel = 0; uChannel < uChannels; uChannel++)
        {
            pDestination = reinterpret_cast<short*>(static_cast<unsigned char*>(pSystem->priv.pChannelBuffers[uChannel])
                                                    + uOffset);

            for (uSample = 0; uSample < uSamples; uSample++)
            {
                pDestination[uSample] = pSource[uSample * uChannels + uChannel];
            }

            DCFlushRangeNoSync(pDestination, uChunk);
        }

        uOffset = uOffset + uChunk;
        uRemaining = uRemaining - uChunk;
        pSource = reinterpret_cast<const short*>(reinterpret_cast<const unsigned char*>(pSource)
                                                 + ((uChunk * uChannels) & 0xFFFFFFFE));

        if (uOffset == pSystem->priv.uBufferBytes)
        {
            uOffset = 0;
        }
    }

    pSystem->priv.uWriteOffset = uOffset;

    if ((pSystem->priv.ucFlags & RADSS_FLAG_PLAYING) == 0 && pSystem->ucPaused == 0 && pSystem->ucEnabled != 0)
    {
        for (uChannel = 0; uChannel < uChannels; uChannel++)
        {
            if (pSystem->priv.pVoices[uChannel] != NULL)
            {
                AXSetVoiceState(pSystem->priv.pVoices[uChannel], AX_VOICE_STATE_PLAYING);
            }
        }

        pSystem->priv.ucFlags = pSystem->priv.ucFlags | RADSS_FLAG_PLAYING;
    }

    return 1;
}

static unsigned int RADSS_AXGetProperty(_RadSoundSystem*, RADSS_PROPERTY property, unsigned int*)
{
    switch (property)
    {
    case RADSS_PROPERTY_AX_QUERY:
        return RADSS_AX_QUERY_RESULT;
    }

    return 0;
}

static int RADSS_AXOpenDriver(_RadSoundSystem* pSystem, unsigned int uLatencyMilliseconds, unsigned int uRate,
                              unsigned int uChannels, unsigned int uLockBytes, unsigned int uMode)
{
    short routing[RADSS_AX_ROUTING_ENTRIES] = {};
    AXVoiceDeviceMixData tvMix[RADSS_TV_MIX_COUNT] = {};
    AXVoiceDeviceMixData drcMix[RADSS_DRC_MIX_COUNT] = {};
    AXVoiceVeData voiceVe;
    unsigned int uChunks;
    unsigned int uChunkMilliseconds;
    unsigned int uSamples;
    unsigned int uChunkSamples;
    unsigned int uBufferBytes;
    unsigned int uVoice;
    int iChunkBytes;

    memset(pSystem, 0, RADSS_GENERIC_SIZE);

    if (uMode != 0)
    {
        RadSS_defaultMode = uMode;
    }

    pSystem->pClose = RADSS_AXCloseDriver;
    pSystem->pGetReadyBytes = RADSS_AXGetReadyBytes;
    pSystem->pSetMemory = RADSS_AXSetMem;
    pSystem->pSetOnOff = RADSS_AXSetOnOff;
    pSystem->pSetPaused = RADSS_AXSetPaused;
    pSystem->pLock = RADSS_AXLock;
    pSystem->pUnlock = RADSS_AXUnlock;
    pSystem->pGetProperty = RADSS_AXGetProperty;
    pSystem->pChannelRoute = RADSS_AXChannelRoute;
    pSystem->ucChannels = (unsigned char)uChannels;
    pSystem->usUnderruns = 0;
    pSystem->usRate = (unsigned short)uRate;
    pSystem->ucEnabled = 1;
    pSystem->ucReserved34 = 0;
    pSystem->ucPaused = 1;
    pSystem->pVolume = RADSS_AXVolume;

    if (AXIsInit() == 0)
    {
        pSystem->pError = RADSS_AX_NOT_INITIALIZED;
        return 0;
    }

    uChunks = uLatencyMilliseconds / RADSS_MAXIMUM_CHUNK_MILLISECONDS;
    pSystem->priv.uStagingBytes = uLockBytes;
    pSystem->priv.usLatency = (unsigned short)uLatencyMilliseconds;

    if (uChunks * RADSS_MAXIMUM_CHUNK_MILLISECONDS < uLatencyMilliseconds)
    {
        uChunks = uChunks + 1;
    }

    if (uChunks < RADSS_MINIMUM_CHUNKS)
    {
        uChunks = RADSS_MINIMUM_CHUNKS;
    }

    uChunkMilliseconds = uLatencyMilliseconds / uChunks;
    uSamples = (uRate * uChunks) / RADSS_MILLISECONDS_PER_SECOND + 15;
    iChunkBytes = (uSamples & RADSS_SAMPLE_ALIGNMENT_MASK) * uChannels * 2;
    uChunkSamples = (uSamples * 2) & RADSS_BYTE_ALIGNMENT_MASK;
    pSystem->priv.uChunkStride = iChunkBytes;

    if (uChunkMilliseconds * uChunks < uLatencyMilliseconds)
    {
        uChunkMilliseconds = uChunkMilliseconds + 1;
    }

    if (uChunkMilliseconds > RADSS_MAXIMUM_CHUNK_MILLISECONDS)
    {
        uChunkMilliseconds = RADSS_MAXIMUM_CHUNK_MILLISECONDS;
    }

    pSystem->priv.ucChunkCount = (unsigned char)uChunkMilliseconds;

    pSystem->priv.uFreshTicks = (unsigned long long)uChunks * (OSGetSystemInfo()->busClockSpeed >> 2)
                                    / RADSS_MILLISECONDS_PER_SECOND / RADSS_FRESH_TICK_DIVISOR
                                + 1;
    pSystem->priv.uPollTicks = (unsigned long long)(uChunkMilliseconds * uChunks)
                               * (OSGetSystemInfo()->busClockSpeed >> 2) / RADSS_MILLISECONDS_PER_SECOND;

    uBufferBytes = uChunkMilliseconds * uChunkSamples;
    pSystem->priv.uChunkBytes = uChunkSamples << 1;
    pSystem->priv.uBufferBytes = uBufferBytes;
    pSystem->priv.uReadyLimit = (uBufferBytes * RADSS_READY_LIMIT_NUMERATOR) / RADSS_READY_LIMIT_DENOMINATOR;
    pSystem->uMemoryBytes = uChunkMilliseconds * iChunkBytes + uLockBytes + RADSS_AX_ROUTING_BYTES;
    pSystem->priv.ucFlags = (pSystem->priv.ucFlags & ~RADSS_FLAG_MODE_MASK) | (unsigned char)RadSS_defaultMode;

    InitRouting(RadSS_defaultMode, uChannels, routing);

    for (uVoice = 0; uVoice < uChannels; uVoice++)
    {
        pSystem->priv.pVoices[uVoice] = AXAcquireVoice(RADSS_VOICE_PRIORITY, NULL, NULL);

        if (pSystem->priv.pVoices[uVoice] == NULL)
        {
            pSystem->pError = RADSS_NO_VOICES;
            RADSS_AXCloseDriver(pSystem);
            return 0;
        }

        pSystem->priv.pVoices[uVoice]->callback = NULL;
        pSystem->priv.pVoices[uVoice]->callbackEx = NULL;
        AXSetVoiceSrcType(pSystem->priv.pVoices[uVoice], AX_VOICE_SRC_TYPE_LINEAR);
        AXSetVoiceSrcRatio(pSystem->priv.pVoices[uVoice], (float)uRate / RADSS_REFERENCE_RATE);

        memset(tvMix, 0, sizeof(tvMix));
        memset(drcMix, 0, sizeof(drcMix));

        drcMix[0].bus[0].volume = routing[6 * RADSS_ROUTING_STRIDE + uVoice];
        tvMix[4].bus[0].volume = routing[2 * RADSS_ROUTING_STRIDE + uVoice];
        tvMix[0].bus[0].volume = routing[0 * RADSS_ROUTING_STRIDE + uVoice];
        tvMix[2].bus[0].volume = routing[4 * RADSS_ROUTING_STRIDE + uVoice];
        drcMix[1].bus[0].volume = routing[7 * RADSS_ROUTING_STRIDE + uVoice];
        tvMix[1].bus[0].volume = routing[1 * RADSS_ROUTING_STRIDE + uVoice];
        tvMix[3].bus[0].volume = routing[5 * RADSS_ROUTING_STRIDE + uVoice];
        tvMix[5].bus[0].volume = routing[3 * RADSS_ROUTING_STRIDE + uVoice];

        AXSetVoiceDeviceMix(pSystem->priv.pVoices[uVoice], AX_DEVICE_TYPE_TV, 0, tvMix);
        AXSetVoiceDeviceMix(pSystem->priv.pVoices[uVoice], AX_DEVICE_TYPE_DRC, 0, drcMix);

        voiceVe.volume = RADSS_VOICE_VOLUME;
        voiceVe.delta = 0;
        AXSetVoiceVe(pSystem->priv.pVoices[uVoice], &voiceVe);
    }

    return 1;
}

extern "C" RADSS_OPEN_DRIVER_CALLBACK RADSS_AXInstallDriver1(int iDevice, int)
{
    RadSS_defaultMode = iDevice;
    return RADSS_AXOpenDriver;
}

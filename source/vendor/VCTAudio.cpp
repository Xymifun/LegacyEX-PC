#include "vendor/VCTAudio.hpp"

#include "nn/pia/chat/AudioStream.hpp"
#include "nn/pia/chat/CodecFacade.hpp"
#include "nn/pia/chat/Decoder.hpp"
#include "vendor/VCTDropRecovery.hpp"
#include "vendor/VCTQuality.hpp"
#include "vendor/VCTQueue.hpp"
#include "nn/pia/chat/Adpcm.hpp"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <coreinit/fastmutex.h>
#include <coreinit/systeminfo.h>
#include <coreinit/time.h>

namespace
{

const unsigned int VCT_STREAM_MAX = 15;
const unsigned int VCT_CONFERENCE_CID_MAX = 16;
const int VCT_PCM_SAMPLE_MAX = 32767;
const int VCT_PCM_SAMPLE_MIN = -32768;
const unsigned int VCT_AUDIO_SIGNATURE = 0x5f564354;
const unsigned int VCT_AUDIO_PACKET_TYPE = 0x40;
const unsigned int VCT_RESET_PACKET_TYPE = 0x41;
const unsigned int VCT_EMPTY_PACKET_TYPE = 0x42;
const unsigned int VCT_PACKET_FLAG_RESET = 0x80;
const unsigned int VCT_PACKET_CODEC_MASK = 0x0f;
const unsigned int VCT_PACKET_FLAG_QUALITY_ALERT = 0x20;
const unsigned int VCT_QUALITY_ALERT_CLEARING = 2;
const unsigned int VCT_MODE_CONFERENCE = 2;
const unsigned int VCT_MODE_PEER = 1;
const unsigned int VCT_SESSION_STREAMING = 2;
const unsigned int VCT_SESSION_CONFERENCE_STREAMING = 3;
const unsigned int VCT_ADPCM_STATE_SIZE = 4;
const unsigned int VCT_VAD_STATUS_SILENT = 0;
const unsigned int VCT_VAD_STATUS_SPEECH_START = 1;
const unsigned int VCT_VAD_STATUS_SPEECH_END = 3;
const int VCT_SEND_TARGET_INDEX_MAX = 15;
const unsigned int VCT_PLAY_GUARD_MICROSECONDS = 100;
const unsigned int VCT_PACKET_HEADER_SIZE = 12;
const unsigned int VCT_PACKET_BYTES_PER_MILLISECOND = 8;
const unsigned int VCT_AUDIO_BYTES_PER_MILLISECOND = 16;
const unsigned int VCT_FRAME_LENGTH_MIN = 0x40;
const unsigned int VCT_FRAME_LENGTH_MAX = 0xc0;
const unsigned int VCT_DROP_FADE_SAMPLES = 0x120;
const unsigned int VCT_SPARE_PACKET_DATA_SIZE = 400;
const unsigned long VCT_AUDIO_BUFFER_ALIGNMENT_MASK = 0x1f;
const unsigned int VCT_SEQUENCE_SEED_MASK = 0xffff;
const unsigned char VCT_DROP_RECOVER_MODE_OFF = 0;
const unsigned char VCT_DROP_RECOVER_MODE_ON = 4;

const int VCT_RESULT_OK = 0;
const int VCT_RESULT_FAILED = -1;
const int VCT_RESULT_INVALID_STATE = -2;
const int VCT_RESULT_FULL = -6;

const int VCT_RESULT_INVALID_ARGUMENT = 0x10c07;
const int VCT_RESULT_NOT_INITIALIZED = 0x10c08;
const int VCT_RESULT_NO_RECEIVE_DATA = 0x2c09;
const int VCT_RESULT_NO_SEND_TARGET = 0x2c0a;

const unsigned char g_vctPacketSizeTable[7][2] = {{8, 0}, {8, 0}, {2, 4}, {3, 4}, {4, 4}, {2, 8}, {4, 8}};

long long g_vctAudioFrameTicks = 0;
long long g_vctPlayTickTolerance = 0;
unsigned int g_vctNoiseSkipTime = 0;
unsigned int g_vctStreamingSessionCount = 0;
unsigned int g_vctVadStatus = 0;
VCTCodec g_vctCodec = VCT_CODEC_4;
unsigned int g_vctAudioFrameLength = 0x90;
unsigned int g_vctAudioDataSize = 0x900;
unsigned int g_vctPacketSize = 0;
unsigned int g_vctMaxPacketSize = 0;
long long g_vctNoiseSkipStartTick = 0;
VCTCodec g_vctSpareCodec = VCT_CODEC_2;
unsigned int g_vctSparePacketSize = 0;
unsigned char g_vctVadResetPending = 0;
unsigned char g_vctVadEnabled = 1;
unsigned char g_vctVadDisableState = 0;
unsigned char g_vctSendPending = 0;
unsigned char g_vctAudioScaleSpecified = 0;
unsigned char g_vctHpfEnabled = 0;
unsigned char g_vctSparePacketEnabled = 1;
unsigned int g_vctTotalPlayTime = 0;
unsigned int g_vctTotalSpeechTime = 0;
int g_vctHpf1FeedForward = 0;
int g_vctHpf1Previous = 0;
int g_vctHpfFeedForward = 0;
int g_vctHpfPrevious = 0;
unsigned int g_vctConferenceClientCount = 0;

nn::pia::chat::Adpcm::State g_vctAdpcmState;
nn::pia::chat::Adpcm::State g_vctSpareAdpcmState;
unsigned short g_vctSendSequence = 0;
unsigned short g_vctUnknown63CA = 0;
unsigned short g_vctConferenceCids[VCT_CONFERENCE_CID_MAX] = {0};
const VCTSession* g_vctStreamingSessions[VCT_STREAM_MAX] = {0};
struct VCTSendContext
{
    VCTAudioPacket m_Packets[2];
    unsigned int m_AudioScale[2];
    unsigned int m_FlushCount;
    unsigned int m_PacketCount;
    VCTAudioPacket* m_pSendPacket;
    unsigned short m_Cids[VCT_CONFERENCE_CID_MAX];
    unsigned int m_Unknown26C;
    int m_Unknown270;
};

VCTSendContext g_vctSend;
nn::pia::chat::AudioStream g_vctAudioStreams[VCT_STREAM_MAX];
short g_vctReceiveMixBuffer[0x600];
VCTAudioPacket g_vctSendPacket;
VCTAudioPacket g_vctSparePacket;
unsigned char g_vctSparePacketData[VCT_SPARE_PACKET_DATA_SIZE];
struct VCTSessionLists
{
    VCTSession* m_Free;
    VCTSession* m_Active;
    unsigned int m_Unknown04[VCT_STREAM_MAX];
    unsigned int m_Unknown40[VCT_STREAM_MAX];
};

VCTSessionLists g_vctSessionLists = {0, 0};

}

unsigned char g_vctInitialized = 0;
VCTState g_vctState;
OSFastMutex g_vctDecoderMutex;
OSFastMutex g_vctAudioQueueMutex;
OSFastMutex g_vctPacketBufferMutex;

unsigned int VCT_GetNoiseSkipTime()
{
    return g_vctNoiseSkipTime;
}

unsigned int VCTi_GetAudioFrameLength()
{
    return g_vctAudioFrameLength;
}

unsigned int VCTi_GetAudioDataSize()
{
    return g_vctAudioDataSize;
}

void VCT_EnableVAD(bool bEnabled)
{
    g_vctVadEnabled = bEnabled ? 1 : 0;
    VCT_ResetVAD();
    if (!bEnabled)
    {
        g_vctVadDisableState = 1;
    }
}

bool vct_is_adpcm(VCTCodec codec)
{
    return codec == VCT_CODEC_2 || codec == VCT_CODEC_3 || codec == VCT_CODEC_4;
}

bool vct_is_opus(VCTCodec codec)
{
    return codec == VCT_CODEC_5 || codec == VCT_CODEC_6;
}

unsigned int HPFFilter(void* pAudio, unsigned int uiSize)
{
    short* pSamples = static_cast<short*>(pAudio);
    const unsigned int uiCount = uiSize >> 1;

    int iScaled = pSamples[0] * 3743;
    g_vctHpf1Previous = (iScaled + g_vctHpf1FeedForward + g_vctHpf1Previous * 3390) >> 12;
    g_vctHpf1FeedForward = -iScaled;
    unsigned long long ullEnergy = static_cast<unsigned long long>(
        static_cast<long long>(g_vctHpf1Previous * g_vctHpf1Previous));

    for (unsigned int uiIndex = 1; uiIndex < uiCount; ++uiIndex)
    {
        pSamples[uiIndex - 1] = static_cast<short>(g_vctHpf1Previous);
        iScaled = pSamples[uiIndex] * 3743;
        g_vctHpf1Previous = (iScaled + g_vctHpf1FeedForward + g_vctHpf1Previous * 3390) >> 12;
        ullEnergy += static_cast<unsigned long long>(static_cast<long long>(g_vctHpf1Previous * g_vctHpf1Previous));
        g_vctHpf1FeedForward = -iScaled;
    }
    pSamples[uiCount - 1] = static_cast<short>(g_vctHpf1Previous);

    return static_cast<unsigned int>(ullEnergy / uiCount);
}

unsigned int HPFFilter2(const void* pAudio, unsigned int uiSize)
{
    const short* pSamples = static_cast<const short*>(pAudio);
    unsigned long long ullEnergy = 0;
    const unsigned int uiCount = uiSize >> 1;

    for (unsigned int uiIndex = 0; uiIndex < uiCount; ++uiIndex)
    {
        const int iScaled = *pSamples * 3743;
        g_vctHpfPrevious = (iScaled + g_vctHpfFeedForward + g_vctHpfPrevious * 3390) >> 12;
        const int iSquare = g_vctHpfPrevious * g_vctHpfPrevious;
        g_vctHpfFeedForward = -iScaled;
        ++pSamples;
        ullEnergy += static_cast<unsigned long long>(static_cast<long long>(iSquare));
    }

    return static_cast<unsigned int>(ullEnergy / uiCount);
}

nn::pia::Result VCT_SendAudioEx(const void* pAudio, unsigned int uiAudioScale)
{
    const unsigned int uiDataSize = g_vctAudioDataSize;

    g_vctAudioScaleSpecified = 0;
    g_vctSend.m_AudioScale[(g_vctSend.m_PacketCount & 1) ^ 1] = uiAudioScale;

    if (!VCT_IsInitialized())
    {
        return VCT_RESULT_NOT_INITIALIZED;
    }
    if (uiDataSize != g_vctAudioDataSize)
    {
        return VCT_RESULT_INVALID_ARGUMENT;
    }
    if (g_vctStreamingSessionCount == 0)
    {
        return VCT_RESULT_NO_SEND_TARGET;
    }

    for (unsigned int uiSession = 0; uiSession < VCT_STREAM_MAX; ++uiSession)
    {
        const VCTSession* pSession = g_vctStreamingSessions[uiSession];
        if (pSession == 0 || pSession->m_State != VCT_SESSION_STREAMING)
        {
            continue;
        }

        VCTAudioPacket* pPacket = &g_vctSend.m_Packets[(g_vctSend.m_PacketCount & 1) ^ 1];
        pPacket->m_Signature = VCT_AUDIO_SIGNATURE;
        memcpy(pPacket->m_Data + (vct_is_adpcm(g_vctCodec) ? VCT_ADPCM_STATE_SIZE : 0), pAudio, uiDataSize);
        pPacket->m_Type = VCT_AUDIO_PACKET_TYPE;
        pPacket->m_Sequence = g_vctSendSequence;
        ++g_vctSendSequence;
        pPacket->m_Timestamp = static_cast<unsigned int>(OSGetTime() / VCT_TIMESTAMP_TICK_SCALE);
        g_vctSendPending = 1;
        ++g_vctSend.m_PacketCount;
        return VCT_RESULT_OK;
    }

    return VCT_RESULT_NO_SEND_TARGET;
}

void vct_mix_audio(const short* pLeft, const short* pRight, short* pOut, unsigned int uiSize)
{
    for (unsigned int uiSample = 0; uiSample < (uiSize >> 1); uiSample += 4)
    {
        {
            int iMixed = pLeft[uiSample] + pRight[uiSample];
            int iClamped;
            if (iMixed > VCT_PCM_SAMPLE_MAX)
            {
                iClamped = VCT_PCM_SAMPLE_MAX;
            }
            else if (iMixed < VCT_PCM_SAMPLE_MIN)
            {
                iClamped = VCT_PCM_SAMPLE_MIN;
            }
            else
            {
                iClamped = iMixed;
            }
            pOut[uiSample] = static_cast<short>(iClamped);
        }
        {
            int iMixed = pLeft[uiSample + 1] + pRight[uiSample + 1];
            int iClamped;
            if (iMixed > VCT_PCM_SAMPLE_MAX)
            {
                iClamped = VCT_PCM_SAMPLE_MAX;
            }
            else if (iMixed < VCT_PCM_SAMPLE_MIN)
            {
                iClamped = VCT_PCM_SAMPLE_MIN;
            }
            else
            {
                iClamped = iMixed;
            }
            pOut[uiSample + 1] = static_cast<short>(iClamped);
        }
        {
            int iMixed = pLeft[uiSample + 2] + pRight[uiSample + 2];
            int iClamped;
            if (iMixed > VCT_PCM_SAMPLE_MAX)
            {
                iClamped = VCT_PCM_SAMPLE_MAX;
            }
            else if (iMixed < VCT_PCM_SAMPLE_MIN)
            {
                iClamped = VCT_PCM_SAMPLE_MIN;
            }
            else
            {
                iClamped = iMixed;
            }
            pOut[uiSample + 2] = static_cast<short>(iClamped);
        }
        {
            int iMixed = pLeft[uiSample + 3] + pRight[uiSample + 3];
            int iClamped;
            if (iMixed > VCT_PCM_SAMPLE_MAX)
            {
                iClamped = VCT_PCM_SAMPLE_MAX;
            }
            else if (iMixed < VCT_PCM_SAMPLE_MIN)
            {
                iClamped = VCT_PCM_SAMPLE_MIN;
            }
            else
            {
                iClamped = iMixed;
            }
            pOut[uiSample + 3] = static_cast<short>(iClamped);
        }
    }
}

nn::pia::Result VCT_ReceiveAudio(void* pData, unsigned int uiSize, unsigned short* pCids)
{
    if (!VCT_IsInitialized())
    {
        return VCT_RESULT_NOT_INITIALIZED;
    }

    if (pCids != 0)
    {
        for (unsigned int uiSlot = 0; uiSlot < VCT_STREAM_MAX; ++uiSlot)
        {
            pCids[uiSlot] = 0;
        }
    }

    if (uiSize != VCTi_GetAudioDataSize())
    {
        memset(pData, 0, uiSize);
        return VCT_RESULT_INVALID_ARGUMENT;
    }

    if (g_vctStreamingSessionCount == 0)
    {
        memset(pData, 0, uiSize);
        return VCT_RESULT_NO_RECEIVE_DATA;
    }

    unsigned short usCid = 0;
    unsigned int uiStream = 0;
    unsigned int uiRemaining = VCT_STREAM_MAX;

    while (VCTi_ReceiveAudioData(pData, uiSize, uiStream, &usCid) == 0)
    {
        --uiRemaining;
        ++uiStream;
        if (uiRemaining == 0)
        {
            memset(pData, 0, uiSize);
            return VCT_RESULT_NO_RECEIVE_DATA;
        }
    }

    if (pCids != 0)
    {
        pCids[0] = usCid;
    }

    unsigned short* pNextCid = (pCids != 0) ? pCids + 1 : 0;
    for (unsigned int uiOther = uiStream + 1; uiOther < VCT_STREAM_MAX; ++uiOther)
    {
        if (VCTi_ReceiveAudioData(g_vctReceiveMixBuffer, uiSize, uiOther, &usCid) != 0)
        {
            if (pCids != 0)
            {
                *pNextCid = usCid;
                ++pNextCid;
            }
            vct_mix_audio(static_cast<const short*>(pData), g_vctReceiveMixBuffer, static_cast<short*>(pData), uiSize);
        }
    }

    g_vctTotalPlayTime += uiSize >> 4;
    return VCT_RESULT_OK;
}

VCTAudioPacket* vct_prepare_send_buffer()
{
    const unsigned int uiPending = g_vctSend.m_PacketCount - g_vctSend.m_FlushCount;
    if (uiPending > 1)
    {
        g_vctSend.m_FlushCount = g_vctSend.m_PacketCount - 1;
    }
    else if (uiPending == 0)
    {
        return 0;
    }

    memcpy(&g_vctSendPacket, (g_vctSend.m_FlushCount & 1) != 0 ? &g_vctSend.m_Packets[0] : &g_vctSend.m_Packets[1],
           sizeof(VCTAudioPacket));

    const VCTCodec codec = g_vctCodec;
    unsigned char* pAudio = g_vctSendPacket.m_Data;
    const bool bAdpcm = vct_is_adpcm(codec);
    if (bAdpcm)
    {
        pAudio = g_vctSendPacket.m_Data + VCT_ADPCM_STATE_SIZE;
    }

    unsigned int uiAudioScale;
    if (g_vctAudioScaleSpecified != 0)
    {
        uiAudioScale = g_vctSend.m_AudioScale[(g_vctSend.m_FlushCount & 1) ^ 1];
    }
    else
    {
        if (g_vctHpfEnabled != 0)
        {
            uiAudioScale = HPFFilter(pAudio, g_vctAudioDataSize);
        }
        if (g_vctHpfEnabled == 0)
        {
            uiAudioScale = HPFFilter2(pAudio, g_vctAudioDataSize);
        }
    }
    const unsigned int uiLevel = static_cast<unsigned int>(sqrt(static_cast<double>(uiAudioScale)));
    g_vctSendPacket.m_Codec = static_cast<unsigned char>(codec);

    if (g_vctVadEnabled != 0)
    {
        unsigned int uiVadStatus = VCTi_GetVADStatus(uiAudioScale, uiLevel, 0);
        g_vctVadStatus = uiVadStatus;
        if (g_vctNoiseSkipStartTick != 0)
        {
            if (OSGetTime() - g_vctNoiseSkipStartTick
                >= static_cast<long long>(OSMillisecondsToTicks(g_vctNoiseSkipTime)))
            {
                g_vctNoiseSkipStartTick = 0;
            }
            uiVadStatus = VCT_VAD_STATUS_SILENT;
        }

        switch (uiVadStatus)
        {
        case VCT_VAD_STATUS_SILENT:
            ++g_vctSend.m_FlushCount;
            if (!VCT_IsEnableVADInvalidation() || g_vctSendPending == 0)
            {
                return 0;
            }
            g_vctSendPending = 0;
            g_vctSendPacket.m_Type = VCT_EMPTY_PACKET_TYPE;
            g_vctSend.m_Unknown270 = 0;
            for (unsigned int uiEntry = 0; uiEntry < VCT_CONFERENCE_CID_MAX; ++uiEntry)
            {
                const unsigned short usCid = g_vctConferenceCids[uiEntry];
                if (usCid == g_vctState.m_Cid)
                {
                    g_vctSend.m_Cids[uiEntry] = 0;
                }
                else
                {
                    g_vctSend.m_Cids[uiEntry] = usCid;
                    if (usCid != 0)
                    {
                        ++g_vctSend.m_Unknown270;
                    }
                }
            }
            if (--g_vctSend.m_Unknown270 > VCT_SEND_TARGET_INDEX_MAX)
            {
                return 0;
            }
            g_vctSend.m_Unknown26C = 0;
            return &g_vctSendPacket;
        case VCT_VAD_STATUS_SPEECH_START:
            g_vctSendPacket.m_Codec |= VCT_PACKET_FLAG_RESET;
            break;
        case VCT_VAD_STATUS_SPEECH_END:
            g_vctSendPacket.m_Type = VCT_RESET_PACKET_TYPE;
            break;
        default:
            break;
        }
    }

    if (g_vctVadEnabled == 0 && g_vctVadDisableState != 0)
    {
        g_vctVadDisableState = 0;
        g_vctSendPacket.m_Codec |= VCT_PACKET_FLAG_RESET;
    }
    if (g_vctVadResetPending != 0)
    {
        g_vctVadResetPending = 0;
        g_vctSendPacket.m_Codec |= VCT_PACKET_FLAG_RESET;
    }

    nn::pia::chat::CodecFacade* pCodecFacade = nn::pia::chat::g_codecFacade;
    if (bAdpcm)
    {
        if ((g_vctSendPacket.m_Codec & VCT_PACKET_FLAG_RESET) != 0)
        {
            init_adpcm_state();
        }
        if (g_vctSparePacketEnabled != 0 && VCTi_GetQualityAlertStatus() == 0)
        {
            g_vctSendPacket.m_Codec |= VCT_PACKET_FLAG_SPARE;
            memcpy(&g_vctSparePacket, &g_vctSendPacket, sizeof(VCTAudioPacket));
            pCodecFacade->Encode(pAudio - VCT_ADPCM_STATE_SIZE, reinterpret_cast<const short*>(pAudio),
                                 g_vctAudioDataSize >> 1, &g_vctAdpcmState, codec);
            memcpy(reinterpret_cast<unsigned char*>(&g_vctSendPacket) + g_vctPacketSize, g_vctSparePacketData,
                   g_vctSparePacketSize);
            g_vctSparePacket.m_Codec = static_cast<unsigned char>(
                (g_vctSpareCodec & VCT_PACKET_CODEC_MASK) | (g_vctSparePacket.m_Codec & ~VCT_PACKET_CODEC_MASK));
            pCodecFacade->Encode(g_vctSparePacket.m_Data,
                                 reinterpret_cast<const short*>(g_vctSparePacket.m_Data + VCT_ADPCM_STATE_SIZE),
                                 g_vctAudioDataSize >> 1, &g_vctSpareAdpcmState, g_vctSpareCodec);
            memcpy(g_vctSparePacketData, &g_vctSparePacket, g_vctSparePacketSize);
        }
        else
        {
            pCodecFacade->Encode(pAudio - VCT_ADPCM_STATE_SIZE, reinterpret_cast<const short*>(pAudio),
                                 g_vctAudioDataSize >> 1, &g_vctAdpcmState, codec);
        }
    }
    else if (vct_is_opus(codec))
    {
        if (g_vctSparePacketEnabled != 0 && VCTi_GetQualityAlertStatus() == 0)
        {
            g_vctSendPacket.m_Codec |= VCT_PACKET_FLAG_SPARE;
            memcpy(&g_vctSparePacket, &g_vctSendPacket, sizeof(VCTAudioPacket));
            pCodecFacade->Encode(pAudio, reinterpret_cast<const short*>(pAudio), g_vctAudioDataSize >> 1, 0, codec);
            memcpy(reinterpret_cast<unsigned char*>(&g_vctSendPacket) + g_vctPacketSize, g_vctSparePacketData,
                   g_vctSparePacketSize);
            g_vctSparePacket.m_Codec = static_cast<unsigned char>(
                (g_vctSpareCodec & VCT_PACKET_CODEC_MASK) | (g_vctSparePacket.m_Codec & ~VCT_PACKET_CODEC_MASK));
            pCodecFacade->Encode(g_vctSparePacket.m_Data, reinterpret_cast<const short*>(g_vctSparePacket.m_Data),
                                 g_vctAudioDataSize >> 1, 0, g_vctSpareCodec);
            memcpy(g_vctSparePacketData, &g_vctSparePacket, g_vctSparePacketSize);
        }
        else
        {
            pCodecFacade->Encode(pAudio, reinterpret_cast<const short*>(pAudio), g_vctAudioDataSize >> 1, 0, codec);
        }
    }
    else if (codec == VCT_CODEC_0 || codec == VCT_CODEC_1)
    {
        pCodecFacade->Encode(pAudio, reinterpret_cast<const short*>(pAudio), g_vctAudioDataSize >> 1, 0, codec);
    }

    g_vctSend.m_Unknown270 = 0;
    for (unsigned int uiEntry = 0; uiEntry < VCT_CONFERENCE_CID_MAX; ++uiEntry)
    {
        const unsigned short usCid = g_vctConferenceCids[uiEntry];
        if (usCid == g_vctState.m_Cid)
        {
            g_vctSend.m_Cids[uiEntry] = 0;
        }
        else
        {
            g_vctSend.m_Cids[uiEntry] = usCid;
            if (usCid != 0)
            {
                ++g_vctSend.m_Unknown270;
            }
        }
    }
    if (--g_vctSend.m_Unknown270 > VCT_SEND_TARGET_INDEX_MAX)
    {
        return 0;
    }
    g_vctSend.m_Unknown26C = 0;
    return &g_vctSendPacket;
}

nn::pia::chat::AudioStream* VCTi_GetStreamFromCid(unsigned short usCid)
{
    for (unsigned int uiStream = 0; uiStream < VCT_STREAM_MAX; ++uiStream)
    {
        if (g_vctAudioStreams[uiStream].m_Cid == usCid)
        {
            return &g_vctAudioStreams[uiStream];
        }
    }
    return 0;
}

bool VCT_GetQualityAlert(unsigned short usCid)
{
    if (usCid == g_vctState.m_Cid)
    {
        return VCT_GetQualityAlertStatus();
    }

    const nn::pia::chat::AudioStream* pStream = VCTi_GetStreamFromCid(usCid);
    if (pStream != 0 && VCT_IsEnableQualityAlert() && VCT_GetQualityAlertThresholdRate() > 0U
        && pStream->m_Info.m_Unknown50 >= VCT_GetQualityAlertThresholdRate())
    {
        return true;
    }
    return false;
}

bool VCT_IsValidCodec(VCTCodec codec)
{
    switch (codec)
    {
    case VCT_CODEC_2:
    case VCT_CODEC_4:
    case VCT_CODEC_5:
    case VCT_CODEC_6:
        return true;
    default:
        return false;
    }
}

bool VCT_IsValidSpareCodec(VCTCodec codec)
{
    const unsigned int value = static_cast<unsigned int>(codec);
    return value == 2U || value == 5U;
}

unsigned int vct_handle_audio(unsigned short usCid, unsigned char* pData, unsigned int uiDataSize,
                              long long llReceiveTick)
{
    if (g_vctStreamingSessionCount == 0)
    {
        return 0;
    }

    nn::pia::chat::AudioStream* pStream = VCTi_GetStreamFromCid(usCid);
    if (pStream == 0)
    {
        return 0;
    }

    if (g_vctVadEnabled == 0)
    {
        pStream->UpdateSkew(llReceiveTick);
    }

    const VCTAudioPacket* pHeader = reinterpret_cast<const VCTAudioPacket*>(pData);
    const unsigned char ucType = pHeader->m_Type;
    if (ucType == 0x41)
    {
        pStream->m_SkewSampleCount = 1;
        pStream->m_audioState = pHeader->m_Sequence;
        pStream->m_BaseSendTick = 0;
        return 0;
    }
    if (ucType != 0x40)
    {
        return 0;
    }

    const unsigned char ucInitialFlags = pData[5];
    const unsigned char ucCodec = static_cast<unsigned char>(ucInitialFlags & 0x0f);
    if (!VCT_IsValidCodec(static_cast<VCTCodec>(ucCodec)))
    {
        return 0;
    }

    pStream->m_Info.m_Unknown00 = ucCodec;

    const unsigned int uiMainSize = vct_get_packet_size(ucCodec);
    const bool bHasSpare = (ucInitialFlags & VCT_PACKET_FLAG_SPARE) != 0;
    const unsigned char* pSpare = 0;
    unsigned int uiSpareSize = 0;
    unsigned int uiPacketSize = uiMainSize;
    const unsigned int uiMainPayload = uiMainSize - 12;

    if (bHasSpare)
    {
        pSpare = pData + uiMainSize;
        const unsigned char ucSpareCodec = static_cast<unsigned char>(pSpare[5] & 0x0f);
        pData[uiMainSize + 5] = ucSpareCodec;
        if (!VCT_IsValidSpareCodec(static_cast<VCTCodec>(ucSpareCodec)))
        {
            return 0;
        }
        uiSpareSize = vct_get_packet_size(ucSpareCodec);
        uiPacketSize += uiSpareSize;
    }

    if (uiDataSize != uiPacketSize)
    {
        return 0;
    }

    if ((ucInitialFlags & VCT_PACKET_FLAG_QUALITY_ALERT) != 0)
    {
        VCTi_SetQualityAlertStatus(1);
    }
    else if (VCTi_GetQualityAlertStatus() == 1)
    {
        VCTi_SetQualityAlertClearStartTick(OSGetTime());
        VCTi_SetQualityAlertStatus(2);
    }

    unsigned int uiConferenceCid = 0;
    while (uiConferenceCid < VCT_CONFERENCE_CID_MAX && g_vctConferenceCids[uiConferenceCid] != usCid)
    {
        ++uiConferenceCid;
    }
    if (uiConferenceCid == VCT_CONFERENCE_CID_MAX)
    {
        return 0;
    }

    _udp_packet* pPacket = vct_alloc_packet_buffer();
    if (pPacket == 0)
    {
        vct_flush_audio_queue(pStream->m_StreamIndex);
        nn::pia::chat::Decoder::Flush(pStream->m_StreamIndex);
        pPacket = vct_alloc_packet_buffer();
        if (pPacket == 0)
        {
            vct_flush_all_audio_queue();
            nn::pia::chat::Decoder::FlushAll();
            pPacket = vct_alloc_packet_buffer();
            if (pPacket == 0)
            {
                vct_trace_packet_buffer(0x4000000000ULL);
                return 0;
            }
        }
    }

    unsigned char* pBufferEnd = pPacket->m_Datagram + VCT_AUDIO_FRAME_SAMPLE_MAX * sizeof(short);
    unsigned char* pFrameData = pBufferEnd - uiMainPayload;
    memcpy(pFrameData, pData + 12, uiDataSize - 12);
    pPacket->m_Codec = ucCodec;
    pPacket->m_StreamIndex = static_cast<unsigned char>(pStream->m_StreamIndex);
    pPacket->m_pFrame = reinterpret_cast<VCTAudioFrame*>(pFrameData);
    const unsigned char ucStoredFlags = pData[5];
    pPacket->m_Flags = ucStoredFlags;
    pPacket->m_Unknown28 = uiMainPayload;
    if ((ucStoredFlags & VCT_PACKET_FLAG_SPARE) != 0)
    {
        memcpy(pBufferEnd, pSpare, uiSpareSize);
    }

    pPacket->m_ReceiveTick = llReceiveTick;
    pPacket->m_Cid = usCid;
    pPacket->m_IsSpare = pData[5] & 0x80;
    pPacket->m_SendTick = static_cast<long long>(pHeader->m_Timestamp);
    pPacket->m_Sequence = pHeader->m_Sequence;

    ++pStream->m_Info.m_Unknown4C;
    const unsigned int uiSamplingPacketNum = VCT_GetQualityAlertSamplingPacketNum();
    if (uiSamplingPacketNum != 0)
    {
        const unsigned int uiSamplingCompare = VCT_GetQualityAlertSamplingPacketNum();
        if (uiSamplingCompare <= pStream->m_Info.m_Unknown4C)
        {
            const unsigned int uiExpiredCount = pStream->m_Info.m_ExpiredCount;
            const unsigned int uiSamplingDivisor = VCT_GetQualityAlertSamplingPacketNum();
            pStream->m_Info.m_ExpiredCount = 0;
            pStream->m_Info.m_Unknown4C = 0;
            pStream->m_Info.m_Unknown50 = (uiExpiredCount * 100U) / uiSamplingDivisor;
        }
    }

    if (pStream->CalcJitterAndSkew(pPacket) == 0)
    {
        vct_free_packet_buffer(pPacket);
        return 0;
    }

    nn::pia::chat::Decoder::Insert(pPacket);
    return 1;
}

int vct_decode_audio_buffer()
{
    _udp_packet* pTop = nn::pia::chat::Decoder::Top();
    if (pTop == 0)
    {
        return 0;
    }

    nn::pia::chat::Decoder::Lock();
    const unsigned int uiStreamIndex = pTop->m_StreamIndex;
    const int iCount = nn::pia::chat::Decoder::CountQueue(uiStreamIndex);
    for (int iPacket = 0; iPacket < iCount; ++iPacket)
    {
        _udp_packet* pPacket = nn::pia::chat::Decoder::GetPacket(uiStreamIndex);
        nn::pia::chat::g_codecFacade->Decode(reinterpret_cast<const unsigned char*>(pPacket->m_pFrame),
                                             reinterpret_cast<short*>(pPacket->m_Datagram), pPacket->m_Unknown28,
                                             static_cast<VCTCodec>(pPacket->m_Codec));
        pPacket->m_pFrame = reinterpret_cast<VCTAudioFrame*>(pPacket->m_Datagram);
        nn::pia::chat::Decoder::RemovePacket(pPacket);
        if (vct_insert_audio_queue(pPacket, pPacket->m_StreamIndex) < 0)
        {
            vct_free_packet_buffer(pPacket);
        }
    }
    nn::pia::chat::Decoder::Unlock();
    return iCount;
}

int vct_flush_send_buffer()
{
    if (g_vctStreamingSessionCount != 0)
    {
        if (g_vctSend.m_pSendPacket == 0)
        {
            g_vctSend.m_pSendPacket = vct_prepare_send_buffer();
            if (g_vctSend.m_pSendPacket != 0)
            {
                return 0;
            }
        }
        else if (g_vctSend.m_pSendPacket->m_Type == VCT_EMPTY_PACKET_TYPE)
        {
            unsigned int uiCidMask = 0;
            for (unsigned int uiEntry = 0; uiEntry < VCT_CONFERENCE_CID_MAX; ++uiEntry)
            {
                const unsigned short usCid = g_vctSend.m_Cids[uiEntry];
                if (usCid == 0)
                {
                    continue;
                }

                const unsigned int uiBit = 1U << (usCid - 1);
                g_vctSend.m_Cids[uiEntry] = 0;
                for (unsigned int uiSession = 0; uiSession < VCT_STREAM_MAX; ++uiSession)
                {
                    const VCTSession* pSession = g_vctStreamingSessions[uiSession];
                    if (pSession != 0 && pSession->m_Cid == usCid)
                    {
                        uiCidMask |= uiBit;
                        break;
                    }
                }
            }
            VCTi_SendMulti(uiCidMask, 0, 0);
            ++g_vctSend.m_FlushCount;
            g_vctSend.m_pSendPacket = 0;
        }
        else
        {
            g_vctTotalSpeechTime += g_vctAudioFrameLength;
            unsigned int uiSendSize = g_vctPacketSize;
            if ((g_vctSend.m_pSendPacket->m_Codec & VCT_PACKET_FLAG_SPARE) != 0)
            {
                uiSendSize += g_vctSparePacketSize;
            }

            unsigned int uiNormalMask = 0;
            unsigned int uiAlertMask = 0;
            for (unsigned int uiEntry = 0; uiEntry < VCT_CONFERENCE_CID_MAX; ++uiEntry)
            {
                const unsigned short usCid = g_vctSend.m_Cids[uiEntry];
                if (usCid == 0)
                {
                    continue;
                }

                const unsigned int uiBit = 1U << (usCid - 1);
                g_vctSend.m_Cids[uiEntry] = 0;
                for (unsigned int uiSession = 0; uiSession < VCT_STREAM_MAX; ++uiSession)
                {
                    const VCTSession* pSession = g_vctStreamingSessions[uiSession];
                    if (pSession != 0 && pSession->m_Cid == usCid)
                    {
                        if (VCT_IsEnableQualityAlert() && VCT_GetQualityAlert(usCid))
                        {
                            uiAlertMask |= uiBit;
                        }
                        else
                        {
                            uiNormalMask |= uiBit;
                        }
                        break;
                    }
                }
            }

            if (uiNormalMask != 0)
            {
                g_vctSend.m_pSendPacket->m_Codec = static_cast<unsigned char>(g_vctSend.m_pSendPacket->m_Codec
                                                                              & ~VCT_PACKET_FLAG_QUALITY_ALERT);
                VCTi_SendMulti(uiNormalMask, g_vctSend.m_pSendPacket, uiSendSize);
            }
            if (uiAlertMask != 0)
            {
                g_vctSend.m_pSendPacket->m_Codec = static_cast<unsigned char>(g_vctSend.m_pSendPacket->m_Codec
                                                                              | VCT_PACKET_FLAG_QUALITY_ALERT);
                VCTi_SendMulti(uiAlertMask, g_vctSend.m_pSendPacket, uiSendSize);
            }
            ++g_vctSend.m_FlushCount;
            g_vctSend.m_pSendPacket = 0;
        }
    }
    return 1;
}

void VCT_EnableSparePacket(bool bEnabled)
{
    g_vctSparePacketEnabled = bEnabled;
    if (bEnabled)
    {
        g_vctSpareAdpcmState.sample = 0;
        g_vctSpareAdpcmState.index = 0;
    }
}

void VCT_EnableHPF(bool bEnabled)
{
    g_vctHpfEnabled = bEnabled;
}

unsigned int VCT_GetDropCount(unsigned short usCid)
{
    const nn::pia::chat::AudioStream* pStream = VCTi_GetStreamFromCid(usCid);
    if (pStream == 0)
    {
        return 0;
    }
    return pStream->m_Info.m_DropCount;
}

unsigned int VCT_GetDropFrequency(unsigned short usCid)
{
    const nn::pia::chat::AudioStream* pStream = VCTi_GetStreamFromCid(usCid);
    if (pStream == 0)
    {
        return 0;
    }

    const unsigned int uiDropCount = VCT_GetDropCount(usCid);
    const unsigned long long ullNow = static_cast<unsigned long long>(OSGetTime());
    const unsigned long long ullElapsedMilliticks = (ullNow - pStream->m_DropMeasureStartTick) * 1000;
    const unsigned int uiTimerClock = OSGetSystemInfo()->busClockSpeed >> 2;

    return uiDropCount * 60 / static_cast<unsigned int>(ullElapsedMilliticks / uiTimerClock / 1000);
}

void VCT_GetAudioInfoFromCid(unsigned short usCid, VCTAudioInfo* pAudioInfo)
{
    nn::pia::chat::AudioStream* pStream = VCTi_GetStreamFromCid(usCid);
    if (pStream == 0)
    {
        memset(pAudioInfo, 0, sizeof(VCTAudioInfo));
        return;
    }

    pStream->m_DropFrequency = VCT_GetDropFrequency(usCid);
    memcpy(pAudioInfo, &pStream->m_Info, sizeof(VCTAudioInfo));
}

void VCT_EnableDropRecover(bool bEnabled)
{
    VCT_SetDropRecoverMode(bEnabled ? VCT_DROP_RECOVER_MODE_ON : VCT_DROP_RECOVER_MODE_OFF);
}

void vct_check_quality_alert_time()
{
    if (VCTi_GetQualityAlertStatus() != VCT_QUALITY_ALERT_CLEARING)
    {
        return;
    }

    if (OSTicksToMilliseconds(OSGetTime() - VCTi_GetQualityAlertClearStartTick())
        >= VCT_GetQualityAlertDuration() * 1000)
    {
        VCTi_SetQualityAlertStatus(0);
        VCTi_SetQualityAlertClearStartTick(0);
        g_vctSpareAdpcmState.sample = 0;
        g_vctSpareAdpcmState.index = 0;
    }
}

VCTSession* vct_find_session(unsigned short usCid)
{
    VCTSession* pSession;
    for (pSession = g_vctSessionLists.m_Active; pSession != 0; pSession = pSession->m_Next)
    {
        if (pSession->m_Mode != 0 && pSession->m_Cid == usCid)
        {
            break;
        }
    }
    return pSession;
}

VCTSession* VCTi_CreateSession(unsigned short usCid)
{
    if (usCid != g_vctState.m_Cid)
    {
        VCTSession* pSession = g_vctSessionLists.m_Free;
        if (pSession != 0)
        {
            g_vctSessionLists.m_Free = pSession->m_Next;
            memset(pSession, 0, sizeof(VCTSession));
            pSession->m_State = 0;
            pSession->m_Mode = g_vctState.m_Mode;
            pSession->m_Cid = usCid;
            pSession->m_Next = g_vctSessionLists.m_Active;
            g_vctSessionLists.m_Active = pSession;
            return pSession;
        }
    }
    return 0;
}

int VCT_DeleteSession(VCTSession* pSession)
{
    if (pSession != 0)
    {
        if (g_vctState.m_Mode == VCT_MODE_PEER)
        {
            return 1;
        }

        VCTSession* pPrevious = 0;
        for (VCTSession* pCurrent = g_vctSessionLists.m_Active; pCurrent != 0; pCurrent = pCurrent->m_Next)
        {
            if (pCurrent == pSession)
            {
                pSession->m_Mode = 0;
                if (pPrevious)
                {
                    pPrevious->m_Next = pSession->m_Next;
                    pSession->m_Next = g_vctSessionLists.m_Free;
                    g_vctSessionLists.m_Free = pSession;
                    return 1;
                }
                g_vctSessionLists.m_Active = pCurrent->m_Next;
                pSession->m_Next = g_vctSessionLists.m_Free;
                g_vctSessionLists.m_Free = pSession;
                return 1;
            }
            pPrevious = pCurrent;
        }
    }

    return 0;
}

void VCT_ClearStreaming(const VCTSession*)
{
    for (unsigned int uiStream = 0; uiStream < VCT_STREAM_MAX; ++uiStream)
    {
        if (g_vctAudioStreams[uiStream].m_PreviousPacket != 0)
        {
            vct_free_packet_buffer(g_vctAudioStreams[uiStream].m_PreviousPacket);
            g_vctAudioStreams[uiStream].m_PreviousPacket = 0;
        }
        vct_flush_audio_queue(uiStream);
        nn::pia::chat::Decoder::Flush(uiStream);
    }
}

int VCT_AddConferenceClient(unsigned short usCid)
{
    if (g_vctState.m_Mode != VCT_MODE_CONFERENCE)
    {
        return VCT_RESULT_INVALID_STATE;
    }
    if (g_vctConferenceClientCount == VCT_STREAM_MAX)
    {
        return VCT_RESULT_FULL;
    }

    if (usCid != g_vctState.m_Cid)
    {
        if (vct_find_session(usCid) != 0)
        {
            return VCT_RESULT_OK;
        }

        VCTSession* pSession = VCTi_CreateSession(usCid);
        if (pSession != 0)
        {
            pSession->m_Cid = usCid;
            pSession->m_OwnerCid = usCid;
            pSession->m_ConferenceCids[0] = usCid;
            pSession->m_State = VCT_SESSION_STREAMING;
            for (unsigned int uiEntry = 1; uiEntry < VCT_CONFERENCE_CID_MAX; ++uiEntry)
            {
                pSession->m_ConferenceCids[uiEntry] = 0;
            }
            VCT_StartStreaming(pSession);
            ++g_vctConferenceClientCount;
            return VCT_RESULT_OK;
        }
    }

    return VCT_RESULT_FAILED;
}

int VCT_RemoveConferenceClient(unsigned short usCid)
{
    if (g_vctState.m_Mode != VCT_MODE_CONFERENCE)
    {
        return VCT_RESULT_INVALID_STATE;
    }

    VCTSession* pSession = vct_find_session(usCid);
    if (pSession == 0)
    {
        return VCT_RESULT_FAILED;
    }

    VCT_StopStreaming(pSession);
    VCT_DeleteSession(pSession);
    --g_vctConferenceClientCount;
    return VCT_RESULT_OK;
}

bool VCT_OnConferenceClient(unsigned short usCid)
{
    if (g_vctState.m_Mode != VCT_MODE_CONFERENCE || vct_find_session(usCid) == 0)
    {
        return false;
    }
    return true;
}

void VCTi_MakeAudioParams(unsigned int uiFrameMilliseconds)
{
    g_vctAudioDataSize = uiFrameMilliseconds * VCT_AUDIO_BYTES_PER_MILLISECOND;
    g_vctAudioFrameLength = uiFrameMilliseconds;
    g_vctPacketSize = vct_get_packet_size(static_cast<unsigned char>(g_vctCodec));
    g_vctSparePacketSize = vct_get_packet_size(static_cast<unsigned char>(g_vctSpareCodec));
    g_vctAudioFrameTicks = OSMillisecondsToTicks(uiFrameMilliseconds);
    g_vctMaxPacketSize = g_vctAudioFrameLength * VCT_PACKET_BYTES_PER_MILLISECOND + VCT_PACKET_HEADER_SIZE;
    VCT_SetDropRecoverContinuousDropFadeLength(static_cast<unsigned char>(VCT_DROP_FADE_SAMPLES / uiFrameMilliseconds));
    g_vctPlayTickTolerance = g_vctAudioFrameTicks + OSMicrosecondsToTicks(VCT_PLAY_GUARD_MICROSECONDS);
    memset(g_vctSparePacketData, 0, sizeof(g_vctSparePacketData));
    nn::pia::chat::AudioStream::MakeStreamParams(g_vctAudioFrameLength);
    g_vctSend.m_PacketCount = 0;
    g_vctSend.m_FlushCount = 0;
    for (unsigned int uiEntry = 0; uiEntry < VCT_CONFERENCE_CID_MAX; ++uiEntry)
    {
        g_vctSend.m_Cids[uiEntry] = 0;
    }
    g_vctSend.m_Unknown26C = 0;
    g_vctSend.m_Unknown270 = 0;
    g_vctSend.m_pSendPacket = 0;
    vct_init_vad();
    for (unsigned int uiStream = 0; uiStream < VCT_STREAM_MAX; ++uiStream)
    {
        nn::pia::chat::AudioStream& stream = g_vctAudioStreams[uiStream];
        vct_init_audio_queue(uiStream);
        stream.InitEx();
    }
    nn::pia::chat::Decoder::Init();
    vct_init_packet_buffer_ex();
}

void VCT_ResetDropFrequencyAll()
{
    for (unsigned int uiStream = 0; uiStream < VCT_STREAM_MAX; ++uiStream)
    {
        g_vctAudioStreams[uiStream].m_DropMeasureStartTick = OSGetTime();
    }
}

int vct_init_audio_ex(VCTCodec codec, unsigned int uiFrameMilliseconds)
{
    g_vctStreamingSessionCount = 0;
    g_vctSendSequence = static_cast<unsigned short>(OSGetTime());
    for (unsigned int uiEntry = 0; uiEntry < VCT_CONFERENCE_CID_MAX; ++uiEntry)
    {
        g_vctConferenceCids[uiEntry] = 0;
    }
    VCTi_InitQualityAlertParam();
    g_vctCodec = codec;
    if (!vct_is_adpcm(codec) && vct_is_opus(codec))
    {
        g_vctSpareCodec = VCT_CODEC_5;
    }
    else
    {
        g_vctSpareCodec = VCT_CODEC_2;
    }
    memset(g_vctReceiveMixBuffer, 0, sizeof(g_vctReceiveMixBuffer));
    g_vctNoiseSkipStartTick = 0;
    for (unsigned int uiStream = 0; uiStream < VCT_STREAM_MAX; ++uiStream)
    {
        g_vctStreamingSessions[uiStream] = 0;
        g_vctAudioStreams[uiStream].Init(uiStream, -1);
    }
    srand(static_cast<unsigned int>(OSGetTime()) & VCT_SEQUENCE_SEED_MASK);
    VCTi_MakeAudioParams(uiFrameMilliseconds);
    VCT_ResetDropFrequencyAll();
    g_vctTotalPlayTime = 0;
    g_vctTotalSpeechTime = 0;
    return 1;
}

int vct_init_ssp(const VCTConfig* pConfig)
{
    VCTSession* pSessions = pConfig->m_pSessions;
    if (pSessions == 0 || pConfig->m_SessionCount == 0 || pConfig->m_SessionCount > VCT_CONFERENCE_CID_MAX)
    {
        return 0;
    }

    g_vctSessionLists.m_Free = pSessions;
    memset(pSessions, 0, pConfig->m_SessionCount * sizeof(VCTSession));
    memset(g_vctSessionLists.m_Unknown40, 0, sizeof(g_vctSessionLists.m_Unknown40));
    memset(g_vctSessionLists.m_Unknown04, 0, sizeof(g_vctSessionLists.m_Unknown04));
    for (unsigned int uiSession = 0; uiSession < pConfig->m_SessionCount - 1; ++uiSession)
    {
        g_vctSessionLists.m_Free[uiSession].m_Next = &g_vctSessionLists.m_Free[uiSession + 1];
    }
    g_vctSessionLists.m_Free[pConfig->m_SessionCount - 1].m_Next = 0;
    g_vctSessionLists.m_Active = 0;
    g_vctConferenceClientCount = 0;
    g_vctState.m_Unknown20 = 0;
    return 1;
}

int VCTi_SendMulti(unsigned int uiCidMask, const void* pData, unsigned int uiSize)
{
    if (g_vctState.m_SendMultiCallback != 0)
    {
        return g_vctState.m_SendMultiCallback(uiCidMask, pData, uiSize);
    }
    return 0;
}

unsigned char VCT_Init(const VCTConfig* pConfig)
{
    OSFastMutex_Init(&g_vctDecoderMutex, 0);
    OSFastMutex_Init(&g_vctAudioQueueMutex, 0);
    OSFastMutex_Init(&g_vctPacketBufferMutex, 0);
    if (pConfig == 0)
    {
        return 0;
    }
    if (g_vctInitialized != 0)
    {
        return 1;
    }
    if (pConfig->m_Mode != VCT_MODE_CONFERENCE || pConfig->m_pAudioBuffer == 0 || pConfig->m_AudioBufferSize == 0
        || (reinterpret_cast<unsigned long>(pConfig->m_pAudioBuffer) & VCT_AUDIO_BUFFER_ALIGNMENT_MASK) != 0)
    {
        return 0;
    }

    memset(&g_vctState, 0, sizeof(g_vctState));
    g_vctState.m_Unknown04 = 0;
    g_vctState.m_SendCallback = pConfig->m_SendCallback;
    g_vctState.m_Unknown10 = pConfig->m_Unknown28;
    g_vctState.m_Mode = pConfig->m_Mode;
    g_vctState.m_Unknown14 = 0;
    g_vctState.m_Cid = pConfig->m_Cid;
    g_vctState.m_SendMultiCallback = pConfig->m_SendMultiCallback;
    if (!vct_init_packet_buffer(pConfig->m_pAudioBuffer, pConfig->m_AudioBufferSize) || !vct_init_ssp(pConfig))
    {
        vct_cleanup_packet_buffer();
        return 0;
    }

    if (!VCT_IsValidCodec(static_cast<VCTCodec>(pConfig->m_Codec)) || pConfig->m_BufferSize < VCT_FRAME_LENGTH_MIN
        || pConfig->m_BufferSize > VCT_FRAME_LENGTH_MAX
        || !vct_init_audio_ex(static_cast<VCTCodec>(pConfig->m_Codec), pConfig->m_BufferSize))
    {
        vct_finish_ssp();
        vct_cleanup_packet_buffer();
        return 0;
    }

    VCT_EnableVAD((pConfig->m_Flags & VCT_CONFIG_FLAG_VAD) != 0);
    VCT_EnableHPF((pConfig->m_Flags & VCT_CONFIG_FLAG_HPF) != 0);
    VCT_EnableSparePacket((pConfig->m_Flags & VCT_CONFIG_FLAG_SPARE_PACKET) != 0);
    VCT_EnableQualityAlert((pConfig->m_Flags & VCT_CONFIG_FLAG_QUALITY_ALERT) != 0);
    VCT_EnableDropRecover((pConfig->m_Flags & VCT_CONFIG_FLAG_DROP_RECOVER) != 0);
    g_vctInitialized = 1;
    return 1;
}

void vct_finish_audio()
{
    g_vctStreamingSessionCount = 0;
}

void vct_finish_ssp()
{
    g_vctSessionLists.m_Active = 0;
    g_vctSessionLists.m_Free = 0;
}

void VCT_Cleanup()
{
    if (g_vctInitialized != 0)
    {
        vct_finish_audio();
        vct_finish_ssp();
        vct_cleanup_packet_buffer();
        g_vctState.m_Mode = 0;
        g_vctInitialized = 0;
    }
}

long long VCTi_GetAudioFrameTicks()
{
    return g_vctAudioFrameTicks;
}

unsigned int VCT_GetTotalPlayTime()
{
    return g_vctTotalPlayTime;
}

unsigned int VCT_GetTotalSpeechTime()
{
    return g_vctTotalSpeechTime;
}

void init_adpcm_state()
{
    g_vctAdpcmState.sample = 0;
    g_vctAdpcmState.index = 0;
    g_vctSpareAdpcmState.sample = 0;
    g_vctSpareAdpcmState.index = 0;
}

int vct_get_packet_size(unsigned char ucCodec)
{
    switch (ucCodec)
    {
    case VCT_CODEC_0:
    case VCT_CODEC_1:
    case VCT_CODEC_2:
    case VCT_CODEC_3:
    case VCT_CODEC_4:
        return g_vctPacketSizeTable[ucCodec][0] * g_vctAudioFrameLength + g_vctPacketSizeTable[ucCodec][1] + 12;
    case VCT_CODEC_5:
    case VCT_CODEC_6:
        return g_vctPacketSizeTable[ucCodec][0] * g_vctAudioFrameLength
               + g_vctPacketSizeTable[ucCodec][1] * (g_vctAudioFrameLength / 0x14) + 12;
    default:
        return 0;
    }
}

nn::pia::chat::AudioStream* vct_get_audio_stream(unsigned int uiStreamIndex)
{
    if (uiStreamIndex >= VCT_STREAM_MAX)
    {
        return 0;
    }
    return &g_vctAudioStreams[uiStreamIndex];
}

int VCTi_ReceiveAudioData(void* pData, unsigned int uiSize, unsigned int uiStreamIndex, unsigned short* pCid)
{
    nn::pia::chat::AudioStream* pStream = &g_vctAudioStreams[uiStreamIndex];
    const unsigned short usStreamCid = static_cast<unsigned short>(pStream->m_Cid);
    bool bFirstPacket = true;

    int iQueued = vct_count_audio_queue(uiStreamIndex);
    if (iQueued != 0)
    {
        do
        {
            const long long llNow = OSGetTime();
            _udp_packet* pPacket = vct_top_audio_queue(uiStreamIndex);
            if (pPacket == 0)
            {
                pStream->m_Info.m_QueuedPacketCount = static_cast<unsigned int>(iQueued);
                return 0;
            }

            const int iArrivalSampleCount = pStream->m_ArrivalSampleCount;
            const int iArrivalDelay = static_cast<int>(llNow) - static_cast<int>(pPacket->m_PlayTick);
            pStream->m_LastArrivalDelay = iArrivalDelay;
            if (pStream->m_MaxArrivalDelay < iArrivalDelay)
            {
                pStream->m_MaxArrivalDelay = iArrivalDelay;
            }
            pStream->m_ArrivalSampleCount = iArrivalSampleCount + 1;
            pStream->m_AverageArrivalDelay = (pStream->m_AverageArrivalDelay * iArrivalSampleCount + iArrivalDelay)
                                             / (iArrivalSampleCount + 1);

            const unsigned int uiLastSequence = pStream->m_LastSequence;
            const unsigned int uiSequence = pPacket->m_Sequence;
            if (uiLastSequence + 1 != uiSequence && uiLastSequence != 0 && pPacket->m_IsSpare == 0)
            {
                if (uiSequence < uiLastSequence)
                {
                    ++pStream->m_Info.m_OutOfOrderCount;
                }
                else if (uiLastSequence < uiSequence)
                {
                    ++pStream->m_Info.m_DropCount;
                }
                else
                {
                    ++pStream->m_Info.m_DuplicateCount;
                }
            }

            const unsigned long long ullGuard = static_cast<unsigned long long>(OSGetSystemInfo()->busClockSpeed >> 2)
                                                * VCT_PLAY_GUARD_MICROSECONDS / 1000000;
            if (static_cast<unsigned long long>(llNow) + ullGuard
                < static_cast<unsigned long long>(pPacket->m_PlayTick))
            {
                ++pStream->m_Info.m_TooEarlyCount;
                if (pPacket->m_IsSpare != 0)
                {
                    return 0;
                }
                if (vct_recover_drop_using_spare(pStream, pPacket, pData, uiSize) == 0)
                {
                    break;
                }
                *pCid = usStreamCid;
                if (pStream->m_PreviousPacket != 0)
                {
                    vct_free_packet_buffer(pStream->m_PreviousPacket);
                    pStream->m_PreviousPacket = 0;
                }
                pStream->m_Info.m_ContinuousDropCount = 0;
                return 1;
            }

            if (llNow > pPacket->m_PlayTick + g_vctPlayTickTolerance)
            {
                ++pStream->m_Info.m_LateCount;
                ++pStream->m_Info.m_ExpiredCount;
                pStream->m_LastSequence = pPacket->m_Sequence;
                iQueued = pStream->SavePreviousBuffer(pPacket);
                pPacket = vct_top_audio_queue(uiStreamIndex);
                bFirstPacket = false;
                continue;
            }

            pStream->m_LastSequence = pPacket->m_Sequence;

            if (bFirstPacket && pStream->m_SkewClampCount != 0)
            {
                iQueued = pStream->SavePreviousBuffer(pPacket);
                pPacket = vct_top_audio_queue(uiStreamIndex);
                pStream->m_SkewClampCount = pStream->m_SkewClampCount - 1;
                continue;
            }

            const int iPreviousDelay = static_cast<int>(pStream->m_PreviousArrivalDelay);
            const int iReceiveDelay = static_cast<int>(llNow) - static_cast<int>(pPacket->m_ReceiveTick);
            pStream->m_PreviousArrivalDelay = static_cast<unsigned int>(iReceiveDelay);
            const int iSmoothedDelay = iReceiveDelay + (-iPreviousDelay >> 4);
            pStream->m_SmoothedArrivalDelay = static_cast<unsigned int>(iSmoothedDelay);
            pStream->m_Info.m_Latency = static_cast<unsigned int>(iSmoothedDelay);

            memcpy(pData, pPacket->m_pFrame, uiSize);
            pStream->SavePreviousBuffer(pPacket);

            if (pStream->m_DropRecovered != 0)
            {
                vct_filter_fade_in(static_cast<const short*>(pData), static_cast<short*>(pData), uiSize >> 1,
                                   VCT_GetDropRecoverInterpolateSampleNum(), pStream->m_LastRecoveredSample);
                pStream->m_DropRecovered = 0;
            }

            *pCid = pPacket->m_Cid;
            pStream->m_Info.m_ContinuousDropCount = 0;
            return 1;
        } while (iQueued != 0);
    }

    const int iRecovered = vct_recover_drop_stream(pStream, pData, uiSize);
    if (iRecovered != 0)
    {
        *pCid = usStreamCid;
    }
    return iRecovered;
}

int VCT_StartStreaming(const VCTSession* pSession)
{
    if (pSession == 0 || g_vctStreamingSessionCount == VCT_STREAM_MAX
        || (pSession->m_State != VCT_SESSION_STREAMING && pSession->m_State != VCT_SESSION_CONFERENCE_STREAMING))
    {
        return 0;
    }

    for (unsigned int uiSlot = 0; uiSlot < VCT_STREAM_MAX; ++uiSlot)
    {
        if (g_vctStreamingSessions[uiSlot] == pSession)
        {
            return 1;
        }
    }

    unsigned int uiStream = 0;
    while (uiStream < VCT_STREAM_MAX && g_vctStreamingSessions[uiStream] != 0)
    {
        ++uiStream;
    }
    if (uiStream < VCT_STREAM_MAX)
    {
        g_vctStreamingSessions[uiStream] = pSession;
    }
    ++g_vctStreamingSessionCount;

    for (unsigned int uiIndex = 0; uiIndex < VCT_CONFERENCE_CID_MAX; ++uiIndex)
    {
        const unsigned short usCid = pSession->m_ConferenceCids[uiIndex];
        if (usCid == 0)
        {
            continue;
        }

        unsigned int uiEntry = 0;
        while (uiEntry < VCT_CONFERENCE_CID_MAX && g_vctConferenceCids[uiEntry] != usCid)
        {
            ++uiEntry;
        }
        if (uiEntry == VCT_CONFERENCE_CID_MAX)
        {
            unsigned short* pSlot = g_vctConferenceCids;
            while (*pSlot != 0)
            {
                ++pSlot;
            }
            *pSlot = usCid;
        }
    }

    g_vctAudioStreams[uiStream].Init(uiStream, pSession->m_OwnerCid);
    vct_init_audio_queue(uiStream);

    if (pSession->m_State == VCT_SESSION_STREAMING && g_vctStreamingSessionCount == 1)
    {
        VCT_ResetVAD();
        g_vctSendSequence = static_cast<unsigned short>(OSGetTime());
        init_adpcm_state();
        g_vctVadResetPending = 1;
        g_vctSend.m_FlushCount = 0;
        g_vctSend.m_PacketCount = 0;
        g_vctSend.m_pSendPacket = 0;
        for (unsigned int uiEntry = 0; uiEntry < VCT_CONFERENCE_CID_MAX; ++uiEntry)
        {
            g_vctSend.m_Cids[uiEntry] = 0;
        }
        g_vctSend.m_Unknown26C = 0;
        g_vctSend.m_Unknown270 = 0;
    }

    return 1;
}

void VCT_StopStreaming(const VCTSession* pSession)
{
    for (unsigned int uiStream = 0; uiStream < VCT_STREAM_MAX; ++uiStream)
    {
        if (g_vctStreamingSessions[uiStream] != pSession)
        {
            continue;
        }

        g_vctStreamingSessions[uiStream] = 0;
        --g_vctStreamingSessionCount;
        g_vctAudioStreams[uiStream].Reset();
        vct_flush_audio_queue(uiStream);
        nn::pia::chat::Decoder::Flush(uiStream);

        for (unsigned int uiIndex = 0; uiIndex < VCT_CONFERENCE_CID_MAX; ++uiIndex)
        {
            const unsigned short usCid = pSession->m_ConferenceCids[uiIndex];
            if (usCid == 0)
            {
                continue;
            }
            for (unsigned int uiEntry = 0; uiEntry < VCT_CONFERENCE_CID_MAX; ++uiEntry)
            {
                if (g_vctConferenceCids[uiEntry] == usCid)
                {
                    g_vctConferenceCids[uiEntry] = 0;
                    break;
                }
            }
        }
    }

    if (g_vctStreamingSessionCount == 0)
    {
        g_vctVadResetPending = 1;
        VCT_ResetVAD();
        for (unsigned int uiEntry = 0; uiEntry < VCT_CONFERENCE_CID_MAX; ++uiEntry)
        {
            g_vctConferenceCids[uiEntry] = 0;
        }
    }
}

#include "platform/wiiu/SndMicDevice.hpp"
#include <coreinit/core.h>
#include <coreinit/memdefaultheap.h>

#include <coreinit/time.h>
#include <coreinit/thread.h>
#include <string.h>

#include "vendor/__N_22_chat_VoiceProtocol_cpp_bf22a0fe.hpp"

namespace nn
{
namespace pia
{
}
}

struct MICStatus
{
    unsigned int uiFlags;
    unsigned int uiAvailableSamples;
    unsigned int uiReadPosition;
};

struct MICRingBuffer
{
    unsigned int uiNumSamples;
    short* pSamples;
};

struct USBMICQueryInfo
{
    unsigned int uiDeviceCount;
    unsigned int uiFlags;
};

struct USBMICOpenInfo
{
    unsigned int uiReserved;
    void* pWorkMemory;
};

struct AXVoiceOffsets
{
    unsigned short usDataType;
    unsigned short usLoopingEnabled;
    unsigned int uiLoopOffset;
    unsigned int uiEndOffset;
    unsigned int uiCurrentOffset;
    const void* pData;
};

struct _AX_FINAL_MIX_CB_STRUCT
{
    int** ppChannelData;
    unsigned short usChannelCount;
    unsigned short usSampleCount;
};

enum eMICState
{
    MIC_STATE_SAMPLERATE = 0,
    MIC_STATE_GAIN = 1,
    MIC_STATE_GAIN_MIN = 2,
    MIC_STATE_GAIN_DEFAULT = 3,
    MIC_STATE_GAIN_MAX = 4,
    MIC_STATE_MUTE = 7,
    MIC_STATE_AGC = 8
};

enum eAXVoiceFormat
{
    AX_VOICE_FORMAT_ADPCM = 0,
    AX_VOICE_FORMAT_LPCM16 = 10,
    AX_VOICE_FORMAT_LPCM8 = 25
};

enum eAXVoiceSrcType
{
    AX_VOICE_SRC_TYPE_NONE = 0,
    AX_VOICE_SRC_TYPE_LINEAR = 1,
    AX_VOICE_SRC_TYPE_FILTERED = 2
};

enum eAXVoiceState
{
    AX_VOICE_STATE_STOPPED = 0,
    AX_VOICE_STATE_RUNNING = 1
};

enum eMICError
{
    MIC_ERROR_NONE = 0,
    MIC_ERROR_NOT_OPENED = -7
};

enum eAXDevice
{
    AX_DEVICE_TV = 0,
    AX_DEVICE_DRC = 1
};

extern "C"
{
void OSSleepTicks(OSTime ticks);
OSThread* OSGetDefaultThread(unsigned int iCoreId);
int OSIsThreadTerminated(OSThread* pThread);
void OSPanic(const char* pFile, int iLine, const char* pMessage);

int MICInit(int iType, void* pArg, MICRingBuffer* pRingBuffer, int* piError);
void MICUninit(int iHandle);
int MICOpen(int iHandle);
int MICClose(int iHandle);
int MICGetStatus(int iHandle, MICStatus* pStatus);
int MICGetState(int iHandle, int iState, unsigned int* puiValue);
int MICSetState(int iHandle, int iState, unsigned int uiValue);
int MICSetDataConsumed(int iHandle, unsigned int uiSamples);

int USBMICOpenQuery(int iHandle, USBMICQueryInfo* pQueryInfo);
int USBMICOpen(int iHandle, USBMICOpenInfo* pOpenInfo, MICRingBuffer* pRingBuffer);
int USBMICClose(int iHandle);
int USBMICStart(int iHandle);
void USBMICStop(int iHandle);
int USBMICGetStatus(int iHandle, MICStatus* pStatus);
int USBMICGetState(int iHandle, int iState, unsigned int* puiValue);
int USBMICSetState(int iHandle, int iState, unsigned int uiValue);
int USBMICSetDataConsumed(int iHandle, unsigned int uiSamples);

void AXInit();
void AXSetDefaultMixerSelect(unsigned int uiSelect);
void AXSetDeviceUpsampleStage(unsigned int uiDevice, unsigned int uiStage);
void AXRegisterDeviceFinalMixCallback(unsigned int uiDevice, void (*pCallback)(_AX_FINAL_MIX_CB_STRUCT*));
void AXRegisterFrameCallback(void (*pCallback)());
void AXUserBegin();
void AXUserEnd();
AXVPB* AXAcquireVoice(unsigned int uiPriority, void (*pDropVoiceCallback)(AXVPB*), unsigned int uiContext);
void AXSetVoiceOffsets(AXVPB* pVoice, const AXVoiceOffsets* pOffsets);
void AXGetVoiceOffsets(AXVPB* pVoice, AXVoiceOffsets* pOffsets);
void AXSetVoiceSrcType(AXVPB* pVoice, unsigned int uiType);
void AXSetVoiceSrcRatio(AXVPB* pVoice, float fRatio);
void AXSetVoiceState(AXVPB* pVoice, unsigned short usState);
void AXPrepareEfxData(void* pData, unsigned int uiSize);

void MIXInit();
void MIXSetSoundMode(unsigned int uiMode);
void MIXInitChannel(AXVPB* pVoice, unsigned int uiDevice, unsigned int uiBus, int iVolume, int iAuxAVolume, int iAuxBVolume, unsigned int uiPan, unsigned int uiSpan, unsigned int uiFader);
void MIXDRCInitChannel(AXVPB* pVoice, unsigned int uiDevice, unsigned int uiBus, unsigned int uiPan, unsigned int uiSpan);
void MIXUpdateSettings();
}

int MicSndFrameThread(int iArgc, void* pArgv);
nn::pia::chat::VoiceProtocol* Chat_GetVoiceProtocol();

SndMicDevice* g_pSndMicDevice = NULL;

static nn::pia::chat::VoiceProtocol* g_pChatVoiceProtocol = NULL;

int MicSndFrameThread(int iArgc, void* pArgv)
{
    return static_cast<SndMicDevice*>(pArgv)->MicSndFrameThreadImpl();
}

int SndMicDevice::RSMP32to8(short* pSrc, int iSrcSamples, short* pDst)
{
    int iDstSamples = 0;

    for (int iSrc = 0; iSrc < iSrcSamples; iSrc += 4)
    {
        pDst[iDstSamples] = pSrc[iSrc];
        iDstSamples++;
    }

    return iDstSamples;
}

SndMicDevice::eSndMicResult SndMicDevice::InitializeMic()
{
    if (m_eMicState != eSndMicState_None)
    {
        return eSndMicResult_MicInvalidState;
    }

    int iError;

    if (m_bUSBMicEnabled)
    {
        USBMICQueryInfo queryInfo;

        iError = USBMICOpenQuery(m_iUSBMicHandle, &queryInfo);

        if (iError == 0)
        {
            MICRingBuffer ringBuffer;
            USBMICOpenInfo openInfo;

            openInfo.uiReserved = 0;
            openInfo.pWorkMemory = NULL;
            ringBuffer.uiNumSamples = MIC_RING_BUFFER_SAMPLES;
            ringBuffer.pSamples = (short*)MEMAllocFromDefaultHeapEx(MIC_RING_BUFFER_BYTES, 64);

            iError = USBMICOpen(m_iUSBMicHandle, &openInfo, &ringBuffer);

            if (iError == 0)
            {
                m_pMicBuffer = ringBuffer.pSamples;
                m_pUSBMicWork = openInfo.pWorkMemory;
                m_eMicState = eSndMicState_MicInitialised;
                m_bUSBMicOpened = true;
                return eSndMicResult_Success;
            }

            MEMFreeToDefaultHeap(openInfo.pWorkMemory);
            MEMFreeToDefaultHeap(ringBuffer.pSamples);
            m_pMicBuffer = NULL;
        }
    }

    m_bUSBMicOpened = false;
    m_iVoiceSendOk = 1;
    m_pMicBuffer = (short*)MEMAllocFromDefaultHeapEx(MIC_RING_BUFFER_BYTES, 64);

    if (m_pMicBuffer == NULL)
    {
        return eSndMicResult_MicFailed;
    }

    MICRingBuffer ringBuffer;

    ringBuffer.uiNumSamples = MIC_RING_BUFFER_SAMPLES;
    ringBuffer.pSamples = m_pMicBuffer;
    int iMicHandle = MICInit(0, NULL, &ringBuffer, &iError);
    m_iMicHandle = iMicHandle;

    if (iMicHandle < 0)
    {
        MEMFreeToDefaultHeap(m_pMicBuffer);
        m_pMicBuffer = NULL;
        return eSndMicResult_MicFailed;
    }

    m_eMicState = eSndMicState_MicInitialised;
    MICSetState(iMicHandle, MIC_STATE_AGC, 1);

    unsigned int uiMuted;

    if (MICGetState(m_iMicHandle, MIC_STATE_MUTE, &uiMuted) == 0)
    {
        MICSetState(m_iMicHandle, MIC_STATE_MUTE, 0);
    }

    return eSndMicResult_Success;
}

SndMicDevice::eSndMicResult SndMicDevice::FinalizeMic()
{
    if (m_eMicState != eSndMicState_MicInitialised && m_eMicState != eSndMicState_MicStopped)
    {
        return eSndMicResult_MicInvalidState;
    }

    if (!m_bUSBMicOpened)
    {
        MICUninit(m_iMicHandle);
        m_iMicHandle = -1;

        if (m_pMicBuffer != NULL)
        {
            MEMFreeToDefaultHeap(m_pMicBuffer);
            m_pMicBuffer = NULL;
        }
    }
    else
    {
        if (m_pMicBuffer != NULL)
        {
            MEMFreeToDefaultHeap(m_pMicBuffer);
            m_pMicBuffer = NULL;
        }

        if (m_pUSBMicWork != NULL)
        {
            MEMFreeToDefaultHeap(m_pUSBMicWork);
            m_pUSBMicWork = NULL;
        }
    }

    m_eMicState = eSndMicState_None;
    return eSndMicResult_Success;
}

SndMicDevice::eSndMicResult SndMicDevice::StartMic()
{
    if (m_eMicState != eSndMicState_MicInitialised && m_eMicState != eSndMicState_MicStopped)
    {
        return eSndMicResult_MicInvalidState;
    }

    m_iVoiceSendOk = 1;

    int iResult;

    if (!m_bUSBMicOpened)
    {
        if (MICOpen(m_iMicHandle) != 0)
        {
            return eSndMicResult_MicFailed;
        }

        for (;;)
        {
            MICStatus status;

            if (MICGetStatus(m_iMicHandle, &status) != 0)
            {
                return eSndMicResult_MicFailed;
            }

            if (status.uiAvailableSamples > MIC_START_SAMPLES)
            {
                break;
            }

            OSSleepTicks(OSMillisecondsToTicks(3));
        }

        do
        {
            do
            {
                OSSleepTicks(OSMillisecondsToTicks(3));
            }
            while (MICGetState(m_iMicHandle, MIC_STATE_SAMPLERATE, &m_uiSampleRate) != 0);
        }
        while (MICGetState(m_iMicHandle, MIC_STATE_GAIN_MAX, &m_uiGainMax) != 0
            || MICGetState(m_iMicHandle, MIC_STATE_GAIN_MIN, &m_uiGainMin) != 0
            || MICGetState(m_iMicHandle, MIC_STATE_GAIN_DEFAULT, &m_uiGainDefault) != 0
            || MICGetState(m_iMicHandle, MIC_STATE_GAIN, &m_uiGain) != 0);

        if ((m_uiGainMax & 0xFFFF8000) != 0)
        {
            return eSndMicResult_MicFailed;
        }

        if ((m_uiGainMin & 0xFFFF8000) != 0)
        {
            return eSndMicResult_MicFailed;
        }

        if ((m_uiGainDefault & 0xFFFF8000) != 0)
        {
            return eSndMicResult_MicFailed;
        }

        if ((m_uiGain & 0xFFFF8000) != 0)
        {
            return eSndMicResult_MicFailed;
        }

        m_uiGain = m_uiGainDefault;
        iResult = MICSetState(m_iMicHandle, MIC_STATE_GAIN, m_uiGain);
    }
    else
    {
        if (USBMICStart(m_iUSBMicHandle) != 0)
        {
            return eSndMicResult_MicFailed;
        }

        for (;;)
        {
            MICStatus status;

            if (USBMICGetStatus(m_iUSBMicHandle, &status) != 0)
            {
                return eSndMicResult_MicFailed;
            }

            if (status.uiAvailableSamples > MIC_START_SAMPLES)
            {
                break;
            }

            OSSleepTicks(OSMillisecondsToTicks(3));
        }

        do
        {
            do
            {
                OSSleepTicks(OSMillisecondsToTicks(3));
            }
            while (USBMICGetState(m_iUSBMicHandle, MIC_STATE_SAMPLERATE, &m_uiSampleRate) != 0);
        }
        while (USBMICGetState(m_iUSBMicHandle, MIC_STATE_GAIN_MAX, &m_uiGainMax) != 0
            || USBMICGetState(m_iUSBMicHandle, MIC_STATE_GAIN_MIN, &m_uiGainMin) != 0
            || USBMICGetState(m_iUSBMicHandle, MIC_STATE_GAIN_DEFAULT, &m_uiGainDefault) != 0
            || USBMICGetState(m_iUSBMicHandle, MIC_STATE_GAIN, &m_uiGain) != 0);

        m_uiGain = m_uiGainDefault;
        iResult = USBMICSetState(m_iUSBMicHandle, MIC_STATE_GAIN, m_uiGain);
    }

    if (iResult != 0)
    {
        return eSndMicResult_MicFailed;
    }

    m_eMicState = eSndMicState_MicStarted;
    return eSndMicResult_Success;
}

SndMicDevice::eSndMicResult SndMicDevice::StopMic()
{
    if (m_eMicState != eSndMicState_MicStarted)
    {
        return eSndMicResult_MicInvalidState;
    }

    int iResult;

    if (!m_bUSBMicOpened)
    {
        iResult = MICClose(m_iMicHandle);
    }
    else
    {
        USBMICStop(m_iUSBMicHandle);
        iResult = USBMICClose(m_iUSBMicHandle);
    }

    if (iResult != MIC_ERROR_NONE && iResult != MIC_ERROR_NOT_OPENED)
    {
        return eSndMicResult_MicFailed;
    }

    m_eMicState = eSndMicState_MicStopped;
    return eSndMicResult_Success;
}

SndMicDevice::eSndMicResult SndMicDevice::ReadMic(short* pBuffer, short sSamples)
{
    if (m_eMicState != eSndMicState_MicStarted)
    {
        return eSndMicResult_MicInvalidState;
    }

    if (pBuffer == NULL || sSamples < 1)
    {
        return eSndMicResult_MicInvalidParam;
    }

    int iResult;

    if (!m_bUSBMicOpened)
    {
        MICStatus status;

        if (MICGetStatus(m_iMicHandle, &status) != 0)
        {
            return eSndMicResult_MicFailed;
        }

        if (status.uiAvailableSamples < (unsigned int)sSamples)
        {
            return eSndMicResult_MicNotEnoughData;
        }

        unsigned int uiReadPosition = status.uiReadPosition;

        short* pOut = pBuffer;

        for (int i = 0; i < sSamples; i++)
        {
            *pOut++ = m_pMicBuffer[uiReadPosition];
            uiReadPosition++;

            if (uiReadPosition >= MIC_RING_BUFFER_SAMPLES)
            {
                uiReadPosition -= MIC_RING_BUFFER_SAMPLES;
            }
        }

        iResult = MICSetDataConsumed(m_iMicHandle, sSamples);
    }
    else
    {
        MICStatus status;

        if (USBMICGetStatus(m_iUSBMicHandle, &status) != 0)
        {
            return eSndMicResult_MicFailed;
        }

        if (status.uiAvailableSamples < (unsigned int)sSamples)
        {
            return eSndMicResult_MicNotEnoughData;
        }

        unsigned int uiReadPosition = status.uiReadPosition;

        short* pOut = pBuffer;

        for (int i = 0; i < sSamples; i++)
        {
            *pOut++ = m_pMicBuffer[uiReadPosition];
            uiReadPosition++;

            if (uiReadPosition >= MIC_RING_BUFFER_SAMPLES)
            {
                uiReadPosition -= MIC_RING_BUFFER_SAMPLES;
            }
        }

        iResult = USBMICSetDataConsumed(m_iUSBMicHandle, sSamples);
    }

    if (iResult != 0)
    {
        return eSndMicResult_MicFailed;
    }

    return eSndMicResult_Success;
}

SndMicDevice::eSndMicResult SndMicDevice::GetStateMic(MIC_STATE_INFO* pStateInfo) const
{
    if (m_eMicState != eSndMicState_MicStarted)
    {
        return eSndMicResult_MicInvalidState;
    }

    if (pStateInfo == NULL)
    {
        return eSndMicResult_MicInvalidParam;
    }

    pStateInfo->uiSampleRate = m_uiSampleRate;
    pStateInfo->uiGainMin = m_uiGainMin;
    pStateInfo->uiGainDefault = m_uiGainDefault;
    pStateInfo->uiGainMax = m_uiGainMax;
    pStateInfo->uiGain = m_uiGain;

    return eSndMicResult_Success;
}

SndMicDevice::eSndMicResult SndMicDevice::InitializeSnd(const SND_PARAM* pParam)
{
    if (m_bSndInitialised)
    {
        return eSndMicResult_Success;
    }

    if (m_eSndState != eSndMicState_None)
    {
        return eSndMicResult_SndInvalidState;
    }

    if (pParam == NULL || pParam->eRateMode >= eSndRateMode_Count)
    {
        return eSndMicResult_SndInvalidParam;
    }

    m_eRateMode = pParam->eRateMode;

    if (m_eRateMode == eSndRateMode_8kHz)
    {
        m_uiFrameSamples = pParam->iFrameLengthMs * 8;
    }
    else if (m_eRateMode == eSndRateMode_16kHz)
    {
        m_uiFrameSamples = pParam->iFrameLengthMs * 16;
    }
    else
    {
        m_uiFrameSamples = pParam->iFrameLengthMs * 32;
    }

    m_pAxBuffer = NULL;
    m_pAxBuffer = (short*)MEMAllocFromDefaultHeapEx(AX_BUFFER_BYTES, 64);

    if (m_pAxBuffer == NULL)
    {
        return eSndMicResult_SndFailed;
    }

    memset(m_pAxBuffer, 0, AX_BUFFER_BYTES);

    m_pSndBuffer = NULL;
    m_pSndBuffer = (short*)MEMAllocFromDefaultHeapEx(m_uiFrameSamples * 6, 64);

    if (m_pSndBuffer == NULL)
    {
        return eSndMicResult_SndFailed;
    }

    memset(m_pSndBuffer, 0, m_uiFrameSamples * 6);

    m_pRefBufferLeft = NULL;
    m_pRefBufferLeft = (short*)MEMAllocFromDefaultHeapEx(m_uiFrameSamples * 12, 64);

    if (m_pRefBufferLeft == NULL)
    {
        return eSndMicResult_SndFailed;
    }

    memset(m_pRefBufferLeft, 0, m_uiFrameSamples * 12);

    m_pRefBufferRight = NULL;
    m_pRefBufferRight = (short*)MEMAllocFromDefaultHeapEx(m_uiFrameSamples * 12, 64);

    if (m_pRefBufferRight == NULL)
    {
        return eSndMicResult_SndFailed;
    }

    memset(m_pRefBufferRight, 0, m_uiFrameSamples * 12);

    m_usRefReadIndex = 0;
    m_usSndReadIndex = 0;
    m_usRefWriteIndex = 0;
    m_uiSamplesSinceFrame = 0;
    m_uiLastVoiceOffset = 0;
    m_usSndStoredSamples = 0;
    m_usRefStoredSamples = 0;
    m_usSndWriteIndex = 0;

    AXInit();
    AXSetDefaultMixerSelect(0);
    MIXInit();
    MIXSetSoundMode(1);
    AXSetDeviceUpsampleStage(AX_DEVICE_TV, 1);
    AXRegisterDeviceFinalMixCallback(AX_DEVICE_TV, finalMixCallback);
    AXRegisterFrameCallback(axFrameCallback);
    AXUserBegin();

    m_pVoice = AXAcquireVoice(15, NULL, 0);

    if (m_pVoice == NULL)
    {
        MEMFreeToDefaultHeap(m_pAxBuffer);
        m_pAxBuffer = NULL;
        MEMFreeToDefaultHeap(m_pSndBuffer);
        m_pSndBuffer = NULL;
        MEMFreeToDefaultHeap(m_pRefBufferLeft);
        m_pRefBufferLeft = NULL;
        MEMFreeToDefaultHeap(m_pRefBufferRight);
        m_pRefBufferRight = NULL;
        return eSndMicResult_SndFailed;
    }

    MIXInitChannel(m_pVoice, 0, 0, -904, -904, -904, 64, 127, 0);
    MIXDRCInitChannel(m_pVoice, 0, 0, 64, 30);
    MIXUpdateSettings();

    AXVoiceOffsets offsets;

    offsets.pData = m_pAxBuffer;
    offsets.uiCurrentOffset = 0;
    offsets.uiEndOffset = AX_BUFFER_SAMPLES - 1;
    offsets.usDataType = AX_VOICE_FORMAT_LPCM16;
    offsets.uiLoopOffset = 0;
    offsets.usLoopingEnabled = 1;

    AXSetVoiceOffsets(m_pVoice, &offsets);
    AXSetVoiceSrcType(m_pVoice, AX_VOICE_SRC_TYPE_LINEAR);

    if (m_eRateMode == eSndRateMode_8kHz)
    {
        m_usAxBlockCount = 64;
        m_usAxBlockIndex = 32;
        m_usAxBlockSamples = 24;
        AXSetVoiceSrcRatio(m_pVoice, 0.25f);
    }
    else if (m_eRateMode == eSndRateMode_16kHz)
    {
        m_usAxBlockIndex = 16;
        m_usAxBlockSamples = 48;
        m_usAxBlockCount = 32;
        AXSetVoiceSrcRatio(m_pVoice, 0.5f);
    }
    else
    {
        m_usAxBlockCount = 16;
        m_usAxBlockIndex = 8;
        m_usAxBlockSamples = 96;
        AXSetVoiceSrcRatio(m_pVoice, 1.0f);
    }

    AXPrepareEfxData(m_pAxBuffer, AX_BUFFER_BYTES);
    AXUserEnd();
    m_eSndState = eSndMicState_SndInitialised;

    return eSndMicResult_Success;
}

SndMicDevice::eSndMicResult SndMicDevice::StartSnd()
{
    if (m_eSndState != eSndMicState_SndInitialised
        && m_eSndState != eSndMicState_SndStopping
        && m_eSndState != eSndMicState_SndStopped)
    {
        return eSndMicResult_SndInvalidState;
    }

    AXSetVoiceState(m_pVoice, AX_VOICE_STATE_RUNNING);
    m_eSndState = eSndMicState_SndStarted;

    return eSndMicResult_Success;
}

SndMicDevice::eSndMicResult SndMicDevice::StopSnd()
{
    if (m_eSndState != eSndMicState_SndStarted)
    {
        return eSndMicResult_SndInvalidState;
    }

    AXSetVoiceState(m_pVoice, AX_VOICE_STATE_STOPPED);
    m_eSndState = eSndMicState_SndStopping;

    return eSndMicResult_Success;
}

SndMicDevice::eSndMicResult SndMicDevice::StoreDataSnd(const short* pData, short sSamples)
{
    if (m_eSndState != eSndMicState_SndStarted)
    {
        return eSndMicResult_SndInvalidState;
    }

    if (pData == NULL
        || sSamples != m_uiFrameSamples
        || m_usSndStoredSamples + sSamples >= m_uiFrameSamples * 3)
    {
        return eSndMicResult_SndInvalidParam;
    }

    memcpy(&m_pSndBuffer[m_usSndWriteIndex], pData, sSamples * 2);

    m_usSndStoredSamples += sSamples;
    m_usSndWriteIndex = (m_usSndWriteIndex + sSamples) % (m_uiFrameSamples * 3);

    return eSndMicResult_Success;
}

void SndMicDevice::InitializeMicSndCommon(SND_PARAM* pParam)
{
    m_iQuitThread = 0;
    m_iPaused = 0;

    OSInitEvent(&m_frameEvent, 0, OS_EVENT_MODE_AUTO);

    memset(m_sRefSendLeft, 0, sizeof(m_sRefSendLeft));
    memset(m_sRefSendRight, 0, sizeof(m_sRefSendRight));

    if (OSGetCoreCount() != 3)
    {
        OSPanic(__FILE__, __LINE__, "Can't run micsnd thread\n");
        return;
    }

    OSGetCoreId();

    if (OSGetMainCoreId() == 0)
    {
        m_pMicSndThread = OSGetDefaultThread(2);
    }
    else
    {
        m_pMicSndThread = OSGetDefaultThread(0);
    }

    if (OSRunThread(m_pMicSndThread,
                    reinterpret_cast<OSThreadEntryPointFn>(MicSndFrameThread), 0,
                    reinterpret_cast<const char**>(this)) == 0)
    {
        OSPanic(__FILE__, __LINE__, "Can't run micsnd thread\n");
    }

    InitializeMic();
    InitializeSnd(pParam);
    StartMic();
    StartSnd();
}

void SndMicDevice::InitializeMicSndCommon(SND_PARAM* pParam, nn::pia::chat::VoiceProtocol* pVoiceProtocol)
{
    g_pChatVoiceProtocol = pVoiceProtocol;
    InitializeMicSndCommon(pParam);
}

void SndMicDevice::FinalizeMicSndCommon()
{
    m_iQuitThread = 1;
    OSSignalEvent(&m_frameEvent);

    while (OSIsThreadTerminated(m_pMicSndThread) != 1)
    {
        OSSleepTicks(OSMillisecondsToTicks(16));
    }

    g_pChatVoiceProtocol = NULL;
}

int SndMicDevice::Pause(SND_PARAM* pParam)
{
    m_iPaused ^= 1;

    if (m_iPaused != 0)
    {
        StopMic();
        StopSnd();
        FinalizeMic();
        FinalizeMicSndCommon();
    }
    else
    {
        InitializeMicSndCommon(pParam, Chat_GetVoiceProtocol());
    }

    return m_iPaused;
}

unsigned int SndMicDevice::GetPacketSquence(unsigned long long uiStationId)
{
    nn::pia::chat::VoiceProtocol* pVoiceProtocol = Chat_GetVoiceProtocol();

    if (pVoiceProtocol != NULL)
    {
        nn::pia::chat::AudioInfo audioInfo;
        nn::pia::Result result = pVoiceProtocol->GetAudioInfo(uiStationId, &audioInfo);

        if (result == 0)
        {
            return audioInfo.m_Sequence;
        }
    }

    return 0xFFFFFFFF;
}

int SndMicDevice::chatSendReceive()
{
    short sRecvBuffer[3072];
    short sSendBuffer[3072];
    short sMicBuffer[6144];

    nn::pia::chat::VoiceProtocol* pVoiceProtocol = g_pChatVoiceProtocol;
    int iAudioFrameLength = pVoiceProtocol->GetAudioFrameLength();
    unsigned int uiAudioDataSize = pVoiceProtocol->GetAudioDataSize();

    if (m_iPaused == 0)
    {
        if (pVoiceProtocol != NULL && pVoiceProtocol->IsInCommunication())
        {
            pVoiceProtocol->Receive(sRecvBuffer, uiAudioDataSize, NULL);
        }
    }
    else
    {
        memset(sRecvBuffer, 0, uiAudioDataSize);
    }

    if (StoreDataSnd(sRecvBuffer, (short)(uiAudioDataSize >> 1)) != eSndMicResult_Success)
    {
        return -1;
    }

    int iFrameSamples = iAudioFrameLength * 32;
    short sFrameSamples = (short)iFrameSamples;

    if (m_iPaused == 0)
    {
        ReadMic(sMicBuffer, sFrameSamples);
    }

    int iSendSamples = RSMP32to8(sMicBuffer, iFrameSamples, sSendBuffer);

    readRefData(&m_sRefSendLeft[REF_SEND_DELAY_SAMPLES], &m_sRefSendRight[REF_SEND_DELAY_SAMPLES], sFrameSamples);

    int iVoiceSendOk = 1;

    if (m_iPaused == 0 && pVoiceProtocol != NULL && pVoiceProtocol->IsInCommunication())
    {
        int result = pVoiceProtocol->Send(sSendBuffer, iSendSamples, m_sRefSendLeft, m_sRefSendRight);

        for (int i = 0; i < REF_SEND_DELAY_SAMPLES; i++)
        {
            m_sRefSendLeft[i] = m_sRefSendLeft[REF_SEND_SAMPLES - REF_SEND_DELAY_SAMPLES + i];
            m_sRefSendRight[i] = m_sRefSendRight[REF_SEND_SAMPLES - REF_SEND_DELAY_SAMPLES + i];
        }

        if (result != 0)
        {
            iVoiceSendOk = 0;
        }
    }

    m_iVoiceSendOk = iVoiceSendOk;

    return 0;
}

int SndMicDevice::MicSndFrameThreadImpl()
{
    for (;;)
    {
        OSWaitEvent(&m_frameEvent);

        if (m_iQuitThread == 1)
        {
            break;
        }

        if (chatSendReceive() == -1)
        {
            return -1;
        }
    }

    return 0;
}

void SndMicDevice::readVoiceData(short* pBuffer, short sSamples)
{
    if (m_usSndStoredSamples < sSamples)
    {
        memset(pBuffer, 0, sSamples * 2);
        return;
    }

    memcpy(pBuffer, &m_pSndBuffer[m_usSndReadIndex], sSamples * 2);

    m_usSndStoredSamples -= sSamples;
    m_usSndReadIndex = (m_usSndReadIndex + sSamples) % (m_uiFrameSamples * 3);
}

void SndMicDevice::readRefData(short* pLeft, short* pRight, short sSamples)
{
    if (m_usRefStoredSamples < sSamples)
    {
        memset(pLeft, 0, sSamples * 2);
        memset(pRight, 0, sSamples * 2);
        return;
    }

    memcpy(pLeft, &m_pRefBufferLeft[m_usRefReadIndex], sSamples * 2);
    memcpy(pRight, &m_pRefBufferRight[m_usRefReadIndex], sSamples * 2);

    m_usRefStoredSamples -= sSamples;
    m_usRefReadIndex = (m_usRefReadIndex + sSamples) % (m_uiFrameSamples * 6);
}

void SndMicDevice::storeRefData(_AX_FINAL_MIX_CB_STRUCT* pMixData)
{
    if (pMixData->usSampleCount != AX_FRAME_SAMPLES)
    {
        return;
    }

    if (m_usRefStoredSamples + AX_FRAME_SAMPLES >= m_uiFrameSamples * 6)
    {
        return;
    }

    short sLeft[AX_FRAME_SAMPLES];
    short sRight[AX_FRAME_SAMPLES];

    int** ppChannelData = pMixData->ppChannelData;
    int* pLeft = ppChannelData[0];
    int* pRight = ppChannelData[1];

    for (int i = 0; i < AX_FRAME_SAMPLES; i++)
    {
        sLeft[i] = (short)*pLeft++;
        sRight[i] = (short)*pRight++;
    }

    memcpy(&m_pRefBufferLeft[m_usRefWriteIndex], sLeft, pMixData->usSampleCount * 2);
    memcpy(&m_pRefBufferRight[m_usRefWriteIndex], sRight, pMixData->usSampleCount * 2);

    m_usRefStoredSamples = (unsigned short)(m_usRefStoredSamples + pMixData->usSampleCount);
    m_usRefWriteIndex = (unsigned short)((m_usRefWriteIndex + pMixData->usSampleCount) % (m_uiFrameSamples * 6));
}

void SndMicDevice::finalMixCallback(_AX_FINAL_MIX_CB_STRUCT* pMixData)
{
    g_pSndMicDevice->storeRefData(pMixData);
}

void SndMicDevice::axSetData()
{
    short sBlock[AX_FRAME_SAMPLES];

    readVoiceData(sBlock, (short)m_usAxBlockSamples);

    memcpy(&m_pAxBuffer[m_usAxBlockSamples * m_usAxBlockIndex], sBlock, m_usAxBlockSamples * 2);
    AXPrepareEfxData(&m_pAxBuffer[m_usAxBlockSamples * m_usAxBlockIndex], m_usAxBlockSamples * 2);

    m_usAxBlockIndex = (unsigned short)((m_usAxBlockIndex + 1) % m_usAxBlockCount);
}

void SndMicDevice::axFrameCallbackImpl()
{
    MIXUpdateSettings();

    switch (m_eSndState)
    {
    case eSndMicState_SndStarted:
    {
        AXVoiceOffsets offsets;

        AXGetVoiceOffsets(m_pVoice, &offsets);

        unsigned int uiCurrentOffset = offsets.uiCurrentOffset;

        if ((unsigned int)m_usAxBlockIndex != uiCurrentOffset / m_usAxBlockSamples)
        {
            axSetData();
        }

        unsigned int uiLastOffset = m_uiLastVoiceOffset;
        unsigned int uiSamplesSinceFrame;

        if (uiLastOffset > uiCurrentOffset)
        {
            uiSamplesSinceFrame = m_uiSamplesSinceFrame + (AX_BUFFER_SAMPLES - (uiLastOffset - uiCurrentOffset));
        }
        else
        {
            uiSamplesSinceFrame = m_uiSamplesSinceFrame + (uiCurrentOffset - uiLastOffset);
        }

        m_uiLastVoiceOffset = uiCurrentOffset;

        if (uiSamplesSinceFrame < m_uiFrameSamples)
        {
            m_uiSamplesSinceFrame = uiSamplesSinceFrame;
            return;
        }

        m_uiSamplesSinceFrame = uiSamplesSinceFrame - m_uiFrameSamples;
        OSSignalEvent(&m_frameEvent);
        break;
    }
    case eSndMicState_SndStopping:
        m_eSndState = eSndMicState_SndStopped;
        break;
    default:
        break;
    }
}

void SndMicDevice::axFrameCallback()
{
    g_pSndMicDevice->axFrameCallbackImpl();
}

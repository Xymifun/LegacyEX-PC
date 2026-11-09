#ifndef PLATFORM_WIIU_SNDMICDEVICE_HPP
#define PLATFORM_WIIU_SNDMICDEVICE_HPP

#include <coreinit/event.h>

struct AXVPB;
struct _AX_FINAL_MIX_CB_STRUCT;

namespace nn
{
namespace pia
{
namespace chat
{
class VoiceProtocol;
}
}
}

enum eSndRateMode
{
    eSndRateMode_8kHz,
    eSndRateMode_16kHz,
    eSndRateMode_32kHz,
    eSndRateMode_Count
};

struct SND_PARAM
{
    eSndRateMode eRateMode;
    int iFrameLengthMs;
};

struct MIC_STATE_INFO
{
    unsigned int uiSampleRate;
    unsigned int uiGain;
    unsigned int uiGainMin;
    unsigned int uiGainDefault;
    unsigned int uiGainMax;
};

class SndMicDevice
{
public:
    enum eSndMicState
    {
        eSndMicState_None = 0,
        eSndMicState_MicInitialised = 1,
        eSndMicState_MicStarted = 2,
        eSndMicState_MicStopped = 3,
        eSndMicState_SndInitialised = 4,
        eSndMicState_SndStarted = 5,
        eSndMicState_SndStopping = 6,
        eSndMicState_SndStopped = 7
    };

    enum eSndMicResult
    {
        eSndMicResult_Success = 0,
        eSndMicResult_MicInvalidParam = 2,
        eSndMicResult_MicInvalidState = 3,
        eSndMicResult_MicFailed = 4,
        eSndMicResult_MicNotEnoughData = 6,
        eSndMicResult_SndInvalidParam = 7,
        eSndMicResult_SndInvalidState = 8,
        eSndMicResult_SndFailed = 9
    };

    enum
    {
        MIC_RING_BUFFER_SAMPLES = 10240,
        MIC_RING_BUFFER_BYTES = 20480,
        MIC_START_SAMPLES = 5120,
        AX_BUFFER_SAMPLES = 1536,
        AX_BUFFER_BYTES = 3072,
        AX_FRAME_SAMPLES = 96,
        REF_SEND_SAMPLES = 4928,
        REF_SEND_DELAY_SAMPLES = 320
    };

    int RSMP32to8(short* pSrc, int iSrcSamples, short* pDst);
    int RSMP32to16(short* pSrc, int iSrcSamples, short* pDst);

    eSndMicResult InitializeMic();
    eSndMicResult InitializeUSBMIC();
    eSndMicResult FinalizeMic();
    eSndMicResult StartMic();
    eSndMicResult StopMic();
    eSndMicResult ReadMic(short* pBuffer, short sSamples);
    eSndMicResult GetStateMic(MIC_STATE_INFO* pStateInfo) const;
    eSndMicResult SetGainLevelMic(unsigned int uiGainLevel, unsigned int uiGainRange);
    eSndMicResult GetGainLevelMic(unsigned int* puiGainLevel) const;

    eSndMicResult InitializeSnd(const SND_PARAM* pParam);
    eSndMicResult FinalizeSnd();
    eSndMicResult StartSnd();
    eSndMicResult StopSnd();
    eSndMicResult StoreDataSnd(const short* pData, short sSamples);
    eSndMicResult GetDRCGainSnd(unsigned short* pusGain) const;
    eSndMicResult SetDRCGainSnd(unsigned short usGain);
    void SetDRCVolume(unsigned int uiVolume);

    void InitializeMicSndCommon(SND_PARAM* pParam);
    void InitializeMicSndCommon(SND_PARAM* pParam, nn::pia::chat::VoiceProtocol* pVoiceProtocol);
    void FinalizeMicSndCommon();
    int Pause(SND_PARAM* pParam);
    unsigned int GetPacketSquence(unsigned long long uiStationId);

    int chatSendReceive();
    int MicSndFrameThreadImpl();

    void readVoiceData(short* pBuffer, short sSamples);
    void readRefData(short* pLeft, short* pRight, short sSamples);
    void storeRefData(_AX_FINAL_MIX_CB_STRUCT* pMixData);
    void axSetData();
    void axFrameCallbackImpl();

    static void finalMixCallback(_AX_FINAL_MIX_CB_STRUCT* pMixData);
    static void axFrameCallback();

private:
    int m_iMicHandle;
    eSndMicState m_eMicState;
    short* m_pMicBuffer;
    unsigned int m_uiSampleRate;
    unsigned int m_uiGainMin;
    unsigned int m_uiGainDefault;
    unsigned int m_uiGainMax;
    unsigned int m_uiGain;
    eSndMicState m_eSndState;
    AXVPB* m_pVoice;
    short* m_pAxBuffer;
    short* m_pSndBuffer;
    unsigned short m_usAxBlockIndex;
    unsigned short m_usAxBlockCount;
    unsigned short m_usAxBlockSamples;
    unsigned short m_usSndWriteIndex;
    unsigned short m_usSndReadIndex;
    unsigned short m_usSndStoredSamples;
    unsigned int m_uiFrameSamples;
    unsigned int m_uiLastVoiceOffset;
    unsigned int m_uiSamplesSinceFrame;
    eSndRateMode m_eRateMode;
    OSEvent m_frameEvent;
    OSThread* m_pMicSndThread;
    int m_iQuitThread;
    int m_iVoiceSendOk;
    int m_iPaused;
    int m_iUSBMicHandle;
    void* m_pUSBMicWork;
    bool m_bUSBMicEnabled;
    bool m_bUSBMicOpened;
    bool m_bMicMuted;
    bool m_bAGCEnabled;
    unsigned short m_usRefWriteIndex;
    unsigned short m_usRefReadIndex;
    unsigned short m_usRefStoredSamples;
    unsigned short m_usDRCGain;
    unsigned int m_uiDRCVolume;
    unsigned int m_uiMicGainLevel;
    unsigned int m_uiMicGainRange;
    short m_sRefSendLeft[REF_SEND_SAMPLES];
    short m_sRefSendRight[REF_SEND_SAMPLES];
    short* m_pRefBufferLeft;
    short* m_pRefBufferRight;
    bool m_bSndInitialised;
};

extern SndMicDevice* g_pSndMicDevice;

#endif

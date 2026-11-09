#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_CHAT_AUDIOSTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_CHAT_AUDIOSTREAM_HPP

#include "vendor/VCTAudio.hpp"

struct _udp_packet;

namespace nn
{
namespace pia
{
namespace chat
{

class AudioStream
{
public:
    AudioStream();
    ~AudioStream();

    void InitEx();
    void Init(unsigned int uiStreamIndex, int iCid);
    void Reset();
    int SavePreviousBuffer(_udp_packet* pPacket);
    void InitSequence(unsigned short usSequence);
    int UpdateSequence(_udp_packet* pPacket, unsigned short usSequence);
    int CalcJitterAndSkew(_udp_packet* pPacket);
    void UpdateSkew(long long llTick);

    static void MakeStreamParams(unsigned int uiFrameMilliseconds);
    static unsigned int GetDefaultJitterBufferSize();

    VCTAudioInfo m_Info;
    unsigned int m_PreviousArrivalDelay;
    unsigned int m_SmoothedArrivalDelay;
    unsigned int m_StreamIndex;
    unsigned int m_audioState;
    unsigned int m_Cid;
    _udp_packet* m_PreviousPacket;
    long long m_BaseReceiveTick;
    long long m_BaseSendTick;
    unsigned int m_timingState;
    unsigned int m_LastQueuedSequence;
    unsigned int m_LastSequence;
    unsigned int m_SequenceBase;
    unsigned int m_SequenceUninitialized;
    unsigned short m_sequenceState;
    unsigned short m_PreviousRawSequence;
    unsigned int m_ExpectedRawSequence;
    unsigned int m_SkewSampleCount;
    unsigned int m_SkewClampCount;
    long long m_SkewAverage;
    long long m_SkewLatest;
    unsigned long long m_DropMeasureStartTick;
    unsigned int m_DropFrequency;
    unsigned int m_EarlyPacketCount;
    unsigned long long m_EarlyBaseSendTick;
    long long m_JitterAverage;
    long long m_JitterBufferTicks;
    long long m_JitterBufferMinimum;
    long long m_PreviousDelay;
    int m_JitterHoldCount;
    unsigned int m_StableFrameCount;
    short m_LastRecoveredSample;
    short m_recoverySample;
    unsigned int m_DropRecovered;
    unsigned int m_Unknown108;
    int m_LastArrivalDelay;
    int m_MaxArrivalDelay;
    int m_AverageArrivalDelay;
    int m_ArrivalSampleCount;
    long long m_SkewCheckTick;
};

}
}
}

#endif

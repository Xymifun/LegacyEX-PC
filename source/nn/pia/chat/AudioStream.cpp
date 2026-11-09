#include "nn/pia/chat/AudioStream.hpp"

#include "vendor/VCTQueue.hpp"

#include <string.h>
#include <coreinit/systeminfo.h>
#include <coreinit/time.h>

namespace
{
const unsigned int kSequenceNumberRange = 0x10000;
const unsigned int kSequenceForwardWindow = 3000;
const unsigned int kSequenceBackwardWindow = 0xff9d;
const unsigned int kSequenceLateThreshold = 0xff9c;
const int kSequenceGapLimit = 100;
const unsigned int kSkewSampleCountMax = 16;
const long long kSkewAverageWeight = 31;
const long long kSkewAverageDivisor = 32;
const long long kJitterAverageShift = 4;
const long long kJitterBufferScale = 3;
const unsigned int kJitterThresholdMsec = 100;
const unsigned int kEarlyPacketCountMin = 10;
const unsigned long long kEarlyResyncMsec = 5000;
const long long kTimestampWrapTicks = static_cast<long long>(VCT_TIMESTAMP_TICK_SCALE) << 32;
const long long kTimestampWrapMargin = 60LL << 32;

long long s_skewCheckInterval = 0;
int s_skewCheckInitialized = 0;
long long s_skewLimit = 0;
long long s_jitterOffsetLimit = 0;
long long s_baseJitterOffset = 0;
unsigned int s_stableFrameThreshold = 0;
}

namespace nn
{
namespace pia
{
namespace chat
{

AudioStream::AudioStream()
{
    memset(&m_Info, 0, sizeof(m_Info));
    m_PreviousArrivalDelay = 0;
    m_SmoothedArrivalDelay = 0;
    m_StreamIndex = 0;
    m_audioState = 0;
    m_Cid = 0;
    m_PreviousPacket = 0;
    m_BaseReceiveTick = 0;
    m_BaseSendTick = 0;
    m_timingState = 0;
    m_LastQueuedSequence = 0;
    m_LastSequence = 0;
    m_SequenceBase = 0;
    m_SequenceUninitialized = 0;
    m_PreviousRawSequence = 0;
    m_ExpectedRawSequence = 0;
    m_SkewSampleCount = 0;
    m_SkewClampCount = 0;
    m_SkewAverage = 0;
    m_SkewLatest = 0;
    m_DropMeasureStartTick = 0;
    m_DropFrequency = 0;
    m_EarlyPacketCount = 0;
    m_EarlyBaseSendTick = 0;
    m_JitterAverage = 0;
    m_JitterBufferTicks = 0;
    m_JitterBufferMinimum = 0;
    m_PreviousDelay = 0;
    m_JitterHoldCount = 0;
    m_StableFrameCount = 0;
    m_LastRecoveredSample = 0;
    m_DropRecovered = 0;
    m_LastArrivalDelay = 0;
    m_MaxArrivalDelay = 0;
    m_AverageArrivalDelay = 0;
    m_ArrivalSampleCount = 0;
    m_SkewCheckTick = 0;
}

AudioStream::~AudioStream()
{
}

void AudioStream::InitEx()
{
    memset(&m_Info, 0, sizeof(m_Info));
    m_Info.m_JitterOffset = s_baseJitterOffset;
    m_Info.m_ContinuousDropCount = 0;
    m_Info.m_ExpiredCount = 0;
    m_Info.m_Unknown4C = 0;
    m_Info.m_Unknown50 = 0;
    m_PreviousArrivalDelay = 0;
    m_SmoothedArrivalDelay = 0;
    m_audioState = 0;
    m_PreviousPacket = 0;
    m_BaseReceiveTick = 0;
    m_BaseSendTick = 0;
    m_timingState = 0;
    m_LastQueuedSequence = 0;
    m_LastSequence = 0;
    m_SequenceUninitialized = 1;
    m_SkewSampleCount = 0;
    m_SkewClampCount = 0;
    m_EarlyPacketCount = 0;
    m_EarlyBaseSendTick = 0;
    m_JitterAverage = 0;
    m_JitterBufferTicks = s_baseJitterOffset;
    m_JitterBufferMinimum = 0;
    m_PreviousDelay = 0;
    m_JitterHoldCount = 0;
    m_StableFrameCount = 0;
    m_LastRecoveredSample = 0;
    m_DropRecovered = 0;
}

void AudioStream::Init(unsigned int uiStreamIndex, int iCid)
{
    InitEx();
    m_StreamIndex = uiStreamIndex;
    m_Cid = static_cast<unsigned int>(iCid);
}

void AudioStream::Reset()
{
    if (m_PreviousPacket != 0)
    {
        vct_free_packet_buffer(m_PreviousPacket);
        m_PreviousPacket = 0;
    }
    m_Cid = 0xffffffff;
}

int AudioStream::SavePreviousBuffer(_udp_packet* pPacket)
{
    if (m_PreviousPacket != 0)
    {
        vct_free_packet_buffer(m_PreviousPacket);
    }
    m_PreviousPacket = pPacket;
    return vct_pop_audio_queue(m_StreamIndex);
}

void AudioStream::InitSequence(unsigned short usSequence)
{
    m_PreviousRawSequence = usSequence;
    m_ExpectedRawSequence = kSequenceNumberRange + 1;
    m_SequenceUninitialized = 0;
    m_SequenceBase = 0;
}

int AudioStream::UpdateSequence(_udp_packet* pPacket, unsigned short usSequence)
{
    unsigned int uiDistance = (usSequence - m_PreviousRawSequence) & 0xffff;
    unsigned int uiExtended;

    if (uiDistance < kSequenceForwardWindow)
    {
        if (usSequence < m_PreviousRawSequence)
        {
            m_SequenceBase = m_SequenceBase + kSequenceNumberRange;
        }
        uiExtended = usSequence + m_SequenceBase;
        m_PreviousRawSequence = usSequence;
    }
    else if (uiDistance < kSequenceBackwardWindow)
    {
        if (usSequence != m_ExpectedRawSequence)
        {
            m_ExpectedRawSequence = (usSequence + 1) & 0xffff;
            return 0;
        }
        InitSequence(usSequence);
        uiExtended = usSequence + m_SequenceBase;
        m_LastQueuedSequence = (usSequence - 1) & 0xffff;
    }
    else
    {
        uiExtended = usSequence + m_SequenceBase;
        if (m_LastQueuedSequence + kSequenceLateThreshold < uiExtended)
        {
            pPacket->m_Sequence = uiExtended - kSequenceNumberRange;
            return 1;
        }
    }

    pPacket->m_Sequence = uiExtended;
    return 1;
}

int AudioStream::CalcJitterAndSkew(_udp_packet* pPacket)
{
    bool bSkewReset = false;
    const unsigned short usSequence = static_cast<unsigned short>(pPacket->m_Sequence);

    if (m_SequenceUninitialized != 0)
    {
        InitSequence(usSequence);
    }
    else
    {
        if (UpdateSequence(pPacket, usSequence) == 0)
        {
            ++m_Info.m_DropCount;
            return 0;
        }
        m_Info.m_LastSequence = pPacket->m_Sequence;
        if (m_LastQueuedSequence == pPacket->m_Sequence)
        {
            return 0;
        }
        if (m_LastQueuedSequence + 1 != pPacket->m_Sequence && pPacket->m_IsSpare == 0)
        {
            const int iGap = static_cast<int>(pPacket->m_Sequence - m_LastQueuedSequence);
            if ((iGap < 0 ? -iGap : iGap) > kSequenceGapLimit)
            {
                m_LastQueuedSequence = pPacket->m_Sequence;
                return 0;
            }
            if (m_LastQueuedSequence > pPacket->m_Sequence)
            {
                ++m_Info.m_OutOfOrderCount;
            }
        }
    }
    m_LastQueuedSequence = pPacket->m_Sequence;

    if (m_BaseSendTick == 0)
    {
        m_BaseSendTick = pPacket->m_SendTick;
        m_BaseReceiveTick = pPacket->m_ReceiveTick;
        m_SkewSampleCount = 0;
    }

    const long long llSendElapsed = (pPacket->m_SendTick - m_BaseSendTick) * VCT_TIMESTAMP_TICK_SCALE;
    long long llDelay = llSendElapsed + m_BaseReceiveTick - pPacket->m_ReceiveTick;
    const long long llFrameTicks = VCTi_GetAudioFrameTicks();

    if (m_PreviousDelay == 0 || pPacket->m_IsSpare != 0)
    {
        llDelay = VCTi_GetAudioFrameTicks();
        m_PreviousDelay = llDelay;
    }
    else
    {
        long long llJitter = llDelay - m_PreviousDelay;
        m_PreviousDelay = llDelay;
        llJitter = llJitter < 0 ? -llJitter : llJitter;
        if (llJitter > (llFrameTicks + m_JitterAverage) * 2
                           + static_cast<unsigned long long>(OSTimerClockSpeed) * kJitterThresholdMsec / 1000)
        {
            m_JitterHoldCount = llJitter / llFrameTicks + 2;
        }
        if (m_JitterHoldCount <= 0)
        {
            m_Info.m_JitterAverage = m_JitterAverage = m_JitterAverage
                                                       + ((llJitter - m_JitterAverage) >> kJitterAverageShift);
        }
    }

    pPacket->m_PlayTick = llSendElapsed + m_BaseReceiveTick + m_Info.m_JitterOffset;
    const long long llNow = OSGetTime();
    if (pPacket->m_PlayTick + llFrameTicks < llNow)
    {
        while (pPacket->m_PlayTick + llFrameTicks + kTimestampWrapMargin < llNow)
        {
            pPacket->m_PlayTick += kTimestampWrapTicks;
        }
        if (pPacket->m_PlayTick + llFrameTicks < llNow)
        {
            ++m_Info.m_LateCount;
            ++m_Info.m_ExpiredCount;
            return 0;
        }
    }

    if (pPacket->m_PlayTick > llNow + OSTimerClockSpeed)
    {
        if (++m_EarlyPacketCount == 1)
        {
            m_EarlyBaseSendTick = llSendElapsed;
            return 0;
        }
        if (m_EarlyPacketCount >= kEarlyPacketCountMin)
        {
            if ((llSendElapsed - m_EarlyBaseSendTick) * 1000 / OSTimerClockSpeed < kEarlyResyncMsec)
            {
                m_BaseSendTick = 0;
            }
            m_EarlyPacketCount = 0;
        }
        return 0;
    }

    if (m_JitterHoldCount <= 0)
    {
        const long long llJitterRange = m_JitterAverage * kJitterBufferScale;
        const long long llFrames = llJitterRange / llFrameTicks;
        if (llJitterRange > m_JitterBufferTicks)
        {
            m_JitterBufferTicks = (llFrames + 1) * llFrameTicks + s_baseJitterOffset;
            m_JitterBufferMinimum = m_JitterBufferTicks - llFrameTicks - llFrameTicks / 2;
            m_StableFrameCount = 0;
            m_Info.m_JitterOffset = s_baseJitterOffset + llFrames * llFrameTicks;
        }
        else if (llJitterRange < m_JitterBufferMinimum)
        {
            if (++m_StableFrameCount > s_stableFrameThreshold)
            {
                m_JitterBufferTicks = (llFrames + 1) * llFrameTicks + s_baseJitterOffset;
                m_JitterBufferMinimum = llFrames * llFrameTicks - llFrameTicks / 2;
                if (m_JitterBufferMinimum < 0)
                {
                    m_JitterBufferMinimum = 0;
                }
                m_StableFrameCount = 0;
                m_Info.m_JitterOffset = s_baseJitterOffset + llFrames * llFrameTicks;
            }
        }
        if (m_Info.m_JitterOffset > s_jitterOffsetLimit)
        {
            m_Info.m_JitterOffset = s_jitterOffsetLimit;
        }

        if (m_SkewSampleCount < kSkewSampleCountMax)
        {
            m_SkewAverage = llDelay;
            ++m_SkewSampleCount;
            m_SkewLatest = llDelay;
        }
        else
        {
            m_SkewAverage = (m_SkewAverage * kSkewAverageWeight + llDelay) / kSkewAverageDivisor;
        }
        m_Info.m_Skew = m_SkewLatest - m_SkewAverage;
        if (m_Info.m_Skew > s_skewLimit)
        {
            m_SkewLatest = m_SkewAverage;
            m_Info.m_Skew = s_skewLimit;
            bSkewReset = true;
        }
        if (m_Info.m_Skew < -s_skewLimit)
        {
            m_SkewSampleCount = 0;
            m_SkewLatest = m_SkewAverage;
            m_Info.m_Skew = -s_skewLimit;
            m_BaseSendTick = 0;
            ++m_SkewClampCount;
        }
        else if (bSkewReset)
        {
            m_BaseSendTick = 0;
            m_SkewSampleCount = 0;
        }
    }

    --m_JitterHoldCount;
    return 1;
}

void AudioStream::UpdateSkew(long long llTick)
{
    if (s_skewCheckInitialized == 0)
    {
        s_skewCheckInitialized = 1;
        s_skewCheckInterval = static_cast<long long>(static_cast<unsigned long long>(OSTimerClockSpeed) * 900);
    }

    if (m_SkewCheckTick != 0)
    {
        if (llTick - m_SkewCheckTick <= s_skewCheckInterval)
        {
            return;
        }
        m_BaseSendTick = 0;
        m_SkewSampleCount = 1;
    }

    m_SkewCheckTick = llTick;
}

void AudioStream::MakeStreamParams(unsigned int uiFrameMilliseconds)
{
    const long long llFrameTicks = VCTi_GetAudioFrameTicks();

    s_skewLimit = static_cast<long long>(static_cast<unsigned long long>(uiFrameMilliseconds - 2) * OSTimerClockSpeed
                                         / 1000);

    const long long llHeadroom = static_cast<long long>(OSTimerClockSpeed) - s_skewLimit;
    s_jitterOffsetLimit = llHeadroom
                          - static_cast<long long>(static_cast<unsigned long long>(OSTimerClockSpeed) * 30 / 1000);

    s_stableFrameThreshold = (1000 / uiFrameMilliseconds) * 5;

    s_baseJitterOffset = llFrameTicks
                         + static_cast<long long>(static_cast<unsigned long long>(OSTimerClockSpeed) * 0x53 / 1000);
}

}
}
}

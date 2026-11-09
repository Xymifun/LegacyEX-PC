#include "nn/nex/EndPointInfo.hpp"

#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

EndPointInfo::EndPointInfo() : m_resetTime()
{
    Reset();
}

void EndPointInfo::Reset()
{
    m_resetTime = Time(Time::GetTime());
    m_uiConnectionID = 0;
    m_uiStreamID = 0;
    m_uiLocalSessionID = 0;
    m_uiRemoteSessionID = 0;
    m_uiFaultReason = 0;
    m_uiRTT = 0;
    m_uiRTTAverage = 0;
    m_uiPacketsSent = 0;
    m_uiUnreliablePacketsSent = 0;
    m_uiUnreliableDataPacketsSent = 0;
    m_uiBroadcastPacketsSent = 0;
    m_uiMulticastPacketsSent = 0;
    m_uiPingsSent = 0;
    m_uiTimeouts = 0;
    m_uiDeferredReliableSends = 0;
    m_uiOversizedUnreliableSends = 0;
    m_uiBytesSent = 0;
    m_uiUnreliableBytesSent = 0;
    m_uiUnreliableDataBytesSent = 0;
    m_uiBroadcastBytesSent = 0;
    m_uiMulticastBytesSent = 0;
    m_uiPacketsReceived = 0;
    m_uiDispatchedPackets = 0;
    m_uiBroadcastPacketsReceived = 0;
    m_uiMulticastPacketsReceived = 0;
    m_uiBytesReceived = 0;
    m_uiDispatchedBytes = 0;
    m_uiBroadcastBytesReceived = 0;
    m_uiMulticastBytesReceived = 0;
    m_uiPingsReceived = 0;
    m_uiAcksSent = 0;
    m_uiUnmatchedAcks = 0;
    m_uiLostPackets = 0;
    m_unknown8c = 0;
    m_uiLatePackets = 0;
    m_uiReliablePacketsSent = 0;
    m_uiReliableBytesSent = 0;
    m_uiFragmentedSends = 0;
    m_uiSendFailures = 0;
    m_uiRetransmissions = 0;
    m_uiRetransmittedBytes = 0;
    m_uiMaxRetransmissions = 0;
    m_uiQueuedReliablePackets = 0;
    m_uiSlidingWindowPushFailures = 0;
    m_uiReliablePacketsReceived = 0;
    m_uiReliableBytesReceived = 0;
    m_uiMaxRTO = 0;
    m_uiMinRTO = 0xFFFFFFFF;
    m_uiLastPingSent = 0;
    m_uiLastPingReceived = 0;
}

void EndPointInfo::Trace(EventLog::LogLevel) const
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    Time::GetTime();
}

}
}

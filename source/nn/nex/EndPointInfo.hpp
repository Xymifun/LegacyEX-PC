#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ENDPOINTINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ENDPOINTINFO_HPP

#include "nn/nex/EventLog.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class EndPointInfo : public RootObject
{
public:
    EndPointInfo();

    void Reset();
    void Trace(EventLog::LogLevel level) const;

    unsigned int m_unknown0;
    unsigned int m_unknown4;
    Time m_resetTime;
    unsigned int m_uiConnectionID;
    unsigned char m_uiStreamID;
    unsigned char m_uiLocalSessionID;
    unsigned char m_uiRemoteSessionID;
    unsigned int m_uiFaultReason;
    unsigned int m_uiRTT;
    unsigned int m_uiRTTAverage;
    unsigned int m_uiPacketsSent;
    unsigned int m_uiUnreliablePacketsSent;
    unsigned int m_uiUnreliableDataPacketsSent;
    unsigned int m_uiBroadcastPacketsSent;
    unsigned int m_uiMulticastPacketsSent;
    unsigned int m_uiPingsSent;
    unsigned int m_uiTimeouts;
    unsigned int m_uiDeferredReliableSends;
    unsigned int m_uiOversizedUnreliableSends;
    unsigned int m_uiBytesSent;
    unsigned int m_uiUnreliableBytesSent;
    unsigned int m_uiUnreliableDataBytesSent;
    unsigned int m_uiBroadcastBytesSent;
    unsigned int m_uiMulticastBytesSent;
    unsigned int m_uiPacketsReceived;
    unsigned int m_uiDispatchedPackets;
    unsigned int m_uiBroadcastPacketsReceived;
    unsigned int m_uiMulticastPacketsReceived;
    unsigned int m_uiBytesReceived;
    unsigned int m_uiDispatchedBytes;
    unsigned int m_uiBroadcastBytesReceived;
    unsigned int m_uiMulticastBytesReceived;
    unsigned int m_uiPingsReceived;
    unsigned int m_uiAcksSent;
    unsigned int m_uiUnmatchedAcks;
    unsigned int m_uiLostPackets;
    unsigned int m_unknown8c;
    unsigned int m_uiLatePackets;
    unsigned int m_uiReliablePacketsSent;
    unsigned int m_uiReliableBytesSent;
    unsigned int m_uiFragmentedSends;
    unsigned int m_uiSendFailures;
    unsigned int m_uiRetransmissions;
    unsigned int m_uiRetransmittedBytes;
    unsigned int m_uiMaxRetransmissions;
    unsigned int m_uiQueuedReliablePackets;
    unsigned int m_uiSlidingWindowPushFailures;
    unsigned int m_uiReliablePacketsReceived;
    unsigned int m_uiReliableBytesReceived;
    unsigned int m_uiMaxRTO;
    unsigned int m_uiMinRTO;
    unsigned int m_uiLastPingSent;
    unsigned int m_uiLastPingReceived;
};

}
}

#endif

#include "nn/nex/PRUDPEndPoint.hpp"

#include <algorithm>
#include <cstring>
#include <utility>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/ByteStream.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/EndPointEventHandler.hpp"
#include "nn/nex/FirstErrorHolder.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/PRUDPMessageSelector.hpp"
#include "nn/nex/PRUDPStream.hpp"
#include "nn/nex/PRUDPUserProtocol.hpp"
#include "nn/nex/PacketBuffer.hpp"
#include "nn/nex/PacketBufferManager.hpp"
#include "nn/nex/PacketBufferPacketOut.hpp"
#include "nn/nex/PacketDispatchQueue.hpp"
#include "nn/nex/PacketIn.hpp"
#include "nn/nex/PacketOut.hpp"
#include "nn/nex/PseudoGlobalVariable.hpp"
#include "nn/nex/Platform.hpp"
#include "nn/nex/Relay.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/Router.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/SignatureBytes.hpp"
#include "nn/nex/SlidingWindow.hpp"
#include "nn/nex/SpinTest.hpp"
#include "nn/nex/StreamSettings.hpp"
#include "nn/nex/SystemError.hpp"
#include "nn/nex/Timeout.hpp"
#include "nn/nex/TransportAdapter.hpp"
#include "nn/nex/TransportEventHandler.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum ResultCode
{
    RESULT_SUCCESS = 0x00010001,
    RESULT_INVALID_STATE = static_cast<int>(0x80050002u),
    RESULT_CONNECTION_FAILED = static_cast<int>(0x80050007u),
    RESULT_CONNECTION_INCOMPLETE = static_cast<int>(0x80050008u),
    RESULT_PAYLOAD_TOO_LARGE = static_cast<int>(0x8001000Au),
    RESULT_PACKET_ALLOCATION_FAILED = static_cast<int>(0x8005001Fu),
    RESULT_PUSH_ERROR = static_cast<int>(0x8005000Du)
};

enum NetFaultCode
{
    NET_FAULT_VERSION_MISMATCH = 1,
    NET_FAULT_KEEP_ALIVE_TIMEOUT = 2,
    NET_FAULT_DISCONNECT = 3,
    NET_FAULT_RETRANSMISSION_LIMIT = 7,
    NET_FAULT_CONNECT_REQUEST = 8,
    NET_FAULT_SLIDING_WINDOW_PUSH = 10,
    NET_FAULT_SILENCE_TIMEOUT = 11,
    NET_FAULT_RETRANSMISSION_EXPIRED = 12,
    NET_FAULT_RETRANSMISSION_EXCEEDED = 13
};

enum PacketFlagValue
{
    PACKET_FLAG_AGGREGATE_ACK = 0x2000
};

enum SendFlag
{
    SEND_FLAG_RELIABLE = 0x01,
    SEND_FLAG_TRAILER = 0x02,
    SEND_FLAG_HISTORY_BUNDLE = 0x04,
    SEND_FLAG_NOTIFY = 0x08,
    SEND_FLAG_BROADCAST = 0x10,
    SEND_FLAG_MULTICAST = 0x20
};

enum SupportedFunction
{
    FUNCTION_AGGREGATE_ACK = 0x01000000,
    FUNCTION_CONNECTION_SIGNATURE_NEGOTIATION = 0x02000000,
    FUNCTION_AGGREGATE_ACK_EXTENDED = 0x04000000,
    FUNCTIONS_VERSION_1 = 0x01000000,
    FUNCTIONS_VERSION_2 = 0x07000000,
    FUNCTIONS_VERSION_3 = 0x0F000000,
    FUNCTIONS_VERSION_4 = 0x1F000000
};

enum NegotiatedFlag
{
    NEGOTIATED_VERSION_MASK = 0xFF,
    NEGOTIATED_FLAGS_SHIFT = 8,
    NEGOTIATED_FLAG_SECURE_STREAM_MODE = 0x100
};

enum Limit
{
    SECURE_MODE_ENABLED = 2,
    MAX_UNRELIABLE_PAYLOAD_RESERVE = 0x24,
    MAX_UNRELIABLE_PAYLOAD_RESERVE_V1 = 0x2C,
    AGGREGATE_ACK_BUFFER_SIZE = 0x80,
    AGGREGATE_ACK_OLD_BUFFER_SIZE = 0xFE,
    AGGREGATE_ACK_SEND_THRESHOLD = 1000,
    AGGREGATE_ACK_OLD_SEQUENCES_PER_PACKET = 500,
    PING_RETRANSMIT_TIMEOUT = 1000,
    DISCONNECT_RETRANSMIT_TIMEOUT = 500,
    DEFAULT_CONNECT_TIMEOUT = 10000000,
    INFINITE_TIMEOUT = -1,
    RETRANSMISSION_RESEND_LIMIT = 5,
    MINIMUM_RETRANSMIT_MARGIN = 0x21,
    RTT_VARIATION_MASK = 0xFFFFFFFC,
    MAX_RESENDS_PER_ACK = 2
};

const unsigned int MinimumRetransmitMargin = MINIMUM_RETRANSMIT_MARGIN;
const unsigned int NO_TIMEOUT = 0xFFFFFFFFu;
const unsigned int APPEND_AT_END = 0xFFFFFFFFu;
const unsigned int SPIN_SLEEP_INTERVAL = 50;
const unsigned int SPIN_CONNECT_WAIT_LINE = 0x1FC;
const unsigned int SPIN_CONNECT_STATE_LINE = 0x1F5;
const unsigned int SPIN_DISCONNECT_WAIT_LINE = 0x266;
const unsigned int SPIN_DISCONNECT_STATE_LINE = 0x25F;
const unsigned int SEQUENCE_RANDOM_RANGE = 0xFFFF;
const unsigned int SESSION_RANDOM_RANGE = 0xFF;
const unsigned long long TRACE_FILTER_SLIDING_WINDOW = 0x800000000000ULL;

PseudoGlobalVariable<unsigned char> g_sessionCounter;

inline unsigned int GetPayloadSize(const Buffer* pPayload)
{
    if (pPayload == 0)
        return 0;
    return static_cast<unsigned short>(pPayload->GetLength());
}

}

PRUDPEndPoint::CalcRetransmissionTimeoutCallback PRUDPEndPoint::s_pCalcRetransmissionTimeoutCallback = 0;
bool PRUDPEndPoint::s_bHybridPartialFaultProcessingMode = false;
bool PRUDPEndPoint::s_bDiscardLateUnreliablePackets = false;
int PRUDPEndPoint::s_iPushErrorMode = 0;

void PRUDPEndPoint::SetCalcRetransmissionTimeoutCallback(CalcRetransmissionTimeoutCallback pCallback)
{
    s_pCalcRetransmissionTimeoutCallback = pCallback;
}

void PRUDPEndPoint::ResetEndPointInfo()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_endPointInfo.Reset();
}

bool PRUDPEndPoint::GetEndPointInfo(EndPointInfo* pEndPointInfo) const
{
    if (pEndPointInfo == 0)
        return false;
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    *pEndPointInfo = m_endPointInfo;
    pEndPointInfo->m_uiStreamID = m_stationURL.GetStreamID();
    pEndPointInfo->m_uiLocalSessionID = m_uiSessionID;
    pEndPointInfo->m_uiConnectionID = m_uiConnectionID;
    pEndPointInfo->m_uiRemoteSessionID = m_uiPeerSessionID;
    pEndPointInfo->m_uiRTT = GetRTT();
    pEndPointInfo->m_uiRTTAverage = GetRTTAverage();
    pEndPointInfo->m_uiFaultReason = m_uiFaultReason;
    return true;
}

unsigned int PRUDPEndPoint::GetReliableBufferPacketCount() const
{
    return m_slidingWindows[0]->GetBufferedPacketCount();
}

void PRUDPEndPoint::GetReliableBufferPacketCounts(qVector<unsigned int>& rCounts) const
{
    rCounts.clear();
    for (int i = 0; i < m_uiMaxSubStreamID + 1; ++i)
        rCounts.push_back(m_slidingWindows[i]->GetBufferedPacketCount());
}

unsigned char PRUDPEndPoint::GetMaxSubStreamID() const
{
    return m_uiMaxSubStreamID;
}

bool PRUDPEndPoint::IsNotConnected()
{
    return m_connectionState == NotConnected;
}

bool PRUDPEndPoint::IsConnecting()
{
    return m_connectionState == Connecting;
}

bool PRUDPEndPoint::IsDisconnecting()
{
    return m_connectionState == Disconnecting;
}

bool PRUDPEndPoint::IsFaulty()
{
    return m_connectionState == Faulty;
}

bool PRUDPEndPoint::IsConnected()
{
    return m_connectionState == Connected;
}

EndPoint::_ConnectionState PRUDPEndPoint::GetConnectionState() const
{
    return m_connectionState;
}

bool PRUDPEndPoint::PeerIsConnected()
{
    return m_bPeerConnected;
}

bool PRUDPEndPoint::PeerIsDisconnected()
{
    return !m_bPeerConnected;
}

unsigned int PRUDPEndPoint::GetKeepAliveTimeout()
{
    return m_uiKeepAliveTimeout;
}

unsigned int PRUDPEndPoint::GetMaxSilenceTime()
{
    return m_uiMaxSilenceTime;
}

void PRUDPEndPoint::SetMaxSilenceTime(unsigned int uiSilenceTime)
{
    m_uiMaxSilenceTime = uiSilenceTime;
}

void PRUDPEndPoint::SetPeerConnected()
{
    m_bPeerConnected = true;
}

void PRUDPEndPoint::SetPeerDisconnected()
{
    m_bPeerConnected = false;
}

void PRUDPEndPoint::SetEncryptionKey(const Key& rKey)
{
    m_packetEncDec.SetEncryptionKey(rKey);
}

Key& PRUDPEndPoint::GetEncryptionKey()
{
    return m_packetEncDec.GetEncryptionKey();
}

unsigned long long PRUDPEndPoint::GetSupportedFunctionsFlags() const
{
    return m_uiSupportedFunctionsFlags;
}

unsigned int PRUDPEndPoint::GetSignaledFaultError() const
{
    return m_uiSignaledFaultError;
}

unsigned int PRUDPEndPoint::GetRTT() const
{
    return m_rtt.GetLast();
}

unsigned int PRUDPEndPoint::GetRTTAverage() const
{
    return m_rtt.GetAverage();
}

void PRUDPEndPoint::AnalyzeRttRto(unsigned int uiRTO)
{
    if (uiRTO > m_endPointInfo.m_uiMaxRTO)
        m_endPointInfo.m_uiMaxRTO = uiRTO;
    if (uiRTO < m_endPointInfo.m_uiMinRTO)
        m_endPointInfo.m_uiMinRTO = uiRTO;
}

void PRUDPEndPoint::AdjustRTT(const Time& rTime, const PacketOut* pSentPacket)
{
    m_rtt.Adjust(static_cast<unsigned int>(rTime - pSentPacket->m_unknownF8));
    unsigned int uiVariation = m_rtt.GetVariation() & RTT_VARIATION_MASK;
    AnalyzeRttRto(m_rtt.GetAverage() + std::max(MinimumRetransmitMargin, uiVariation));
}

void PRUDPEndPoint::AdjustRTT(const Packet* pPacket, const PacketOut* pSentPacket)
{
    AdjustRTT(Time(pPacket->m_unknownF8), pSentPacket);
}

bool PRUDPEndPoint::CancelPacketTimeout(PacketOut* pPacket)
{
    return static_cast<PRUDPStream*>(m_pStream)->GetTimeoutManager().CancelPacketTimeout(pPacket);
}

float PRUDPEndPoint::GetTimeoutMultiplier(unsigned short uiRetransmissions, StreamSettings& rSettings) const
{
    if (uiRetransmissions < rSettings.GetExtraRetransmitTimeoutTrigger())
        return rSettings.GetRetransmitTimeoutMultiplier();
    return rSettings.GetExtraRetransmitTimeoutMultiplier();
}

unsigned int PRUDPEndPoint::ComputeRetransmitTimeout(const PacketOut* pPacket)
{
    StreamSettings& rSettings = g_streamSettings[m_pStream->GetType()].Get();
    if (s_pCalcRetransmissionTimeoutCallback != 0)
    {
        unsigned int uiVariation = m_rtt.GetVariation() & RTT_VARIATION_MASK;
        return s_pCalcRetransmissionTimeoutCallback(m_rtt.GetAverage() + std::max(MinimumRetransmitMargin, uiVariation),
                                                    pPacket->m_unknown118);
    }
    float fTimeout;
    if ((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_SYN)
    {
        fTimeout = static_cast<float>(rSettings.GetSynInitialRTT() * pPacket->m_unknown118)
                   * GetTimeoutMultiplier(pPacket->m_unknown118, rSettings);
    }
    else
    {
        unsigned int uiVariation = m_rtt.GetVariation() & RTT_VARIATION_MASK;
        fTimeout = static_cast<float>((m_rtt.GetAverage() + std::max(MinimumRetransmitMargin, uiVariation))
                                      * pPacket->m_unknown118)
                   * GetTimeoutMultiplier(pPacket->m_unknown118, rSettings);
    }
    return std::min(static_cast<unsigned int>(fTimeout), rSettings.GetMaxRetransmitTimeout());
}

PRUDPMessageInterface::SignatureMethod PRUDPEndPoint::DecideSignatureMethod(const Packet* pPacket, Stream::Type type,
                                                                            bool bFlag)
{
    if ((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_CONNECT)
    {
        if (m_uiVersion == 1)
        {
            bool bNegotiationSupported = (GetSupportedFunctionsFlags() & FUNCTION_CONNECTION_SIGNATURE_NEGOTIATION)
                                         != 0;
            if (!bNegotiationSupported)
                return PRUDPMessageInterface::SIGNATURE_METHOD_4;
        }
    }
    return m_pMessageSelector->DecideSignatureMethod(pPacket, type, bFlag);
}

bool PRUDPEndPoint::Defrag(PacketIn* pPacket)
{
    unsigned int uiSubStreamID = pPacket->m_uiSubStreamID;
    Buffer* pPayload = pPacket->m_pPayload;
    if (m_defragBuffers[uiSubStreamID] == 0)
    {
        m_defragBuffers[uiSubStreamID] = pPayload;
        pPayload = 0;
        m_defragBuffers[uiSubStreamID]->AddRef();
    }
    bool bLastFragment = pPacket->m_uiFragmentID == 0;
    if (!bLastFragment && pPacket->m_uiFragmentID != m_defragFragmentIDs[uiSubStreamID])
    {
        if (m_defragBuffers[uiSubStreamID] != 0)
        {
            m_defragBuffers[uiSubStreamID]->ReleaseRef();
            m_defragBuffers[uiSubStreamID] = 0;
        }
        return bLastFragment;
    }
    if (pPayload != 0)
    {
        if (m_defragBuffers[uiSubStreamID]->IsPooled())
        {
            unsigned int uiLength = m_defragBuffers[uiSubStreamID]->GetLength() + pPayload->GetLength();
            if (uiLength > PacketBufferManager::MaxUnitSize)
            {
                Buffer* pBuffer = new Buffer(uiLength, 0);
                *pBuffer = *m_defragBuffers[uiSubStreamID];
                m_defragBuffers[uiSubStreamID]->ReleaseRef();
                m_defragBuffers[uiSubStreamID] = pBuffer;
                *pBuffer += *pPayload;
            }
            else
            {
                if (!m_defragBuffers[uiSubStreamID]->AttemptExpand(pPayload->GetLength()))
                    return false;
                *m_defragBuffers[uiSubStreamID] += *pPayload;
            }
        }
        else
        {
            *m_defragBuffers[uiSubStreamID] += *pPayload;
        }
    }
    if (!bLastFragment)
    {
        ++m_defragFragmentIDs[uiSubStreamID];
        if (m_defragFragmentIDs[uiSubStreamID] == 0)
            ++m_defragFragmentIDs[uiSubStreamID];
    }
    return bLastFragment;
}

void PRUDPEndPoint::Dispatch(PacketIn* pPacket, Time)
{
    m_packetEncDec.Decode(pPacket);
    if ((pPacket->m_uiTypeAndFlags & PACKET_FLAG_RELIABLE) == 0)
    {
        HandlerDispatch(pPacket->m_pPayload, pPacket);
        return;
    }
    if (Defrag(pPacket))
    {
        HandlerDispatch(m_defragBuffers[pPacket->m_uiSubStreamID], pPacket);
        m_defragFragmentIDs[pPacket->m_uiSubStreamID] = 1;
        m_defragBuffers[pPacket->m_uiSubStreamID]->ReleaseRef();
        m_defragBuffers[pPacket->m_uiSubStreamID] = 0;
    }
}

void PRUDPEndPoint::HandlerDispatch(Buffer* pBuffer, PacketIn* pPacket)
{
    if (pPacket->m_uiTypeAndFlags & PACKET_FLAG_HISTORY_BUNDLE)
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        PRUDPUserProtocol* pUserProtocol = PRUDPUserProtocol::GetInstance();
        if (pUserProtocol != 0)
        {
            pUserProtocol->Receive(this, pBuffer, pPacket->m_uiTypeAndFlags, pPacket->m_uiSubStreamID);
        }
    }
    else if (m_pEventHandler != 0)
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        m_pEventHandler->Receive(this, pBuffer, pPacket->m_uiSubStreamID);
    }
}

void PRUDPEndPoint::InitUnreliableSeq(PacketOut* pPacket)
{
    if (pPacket->m_uiVersion != 0 && m_unreliableSequence == SequenceClock())
    {
        unsigned short uiSequence = static_cast<unsigned short>((Time::GetTime() & SEQUENCE_RANDOM_RANGE)
                                                                + Platform::GetRandomNumber(SEQUENCE_RANDOM_RANGE));
        m_unreliableSequence = uiSequence;
        if (uiSequence == 0)
        {
            uiSequence = 1;
            m_unreliableSequence = 1;
        }
        pPacket->m_unknown4c = uiSequence;
    }
}

bool PRUDPEndPoint::IsDuplicateReorderingPacket(PacketIn* pPacket)
{
    unsigned short uiFlags = pPacket->m_uiTypeAndFlags;
    if ((uiFlags & PACKET_TYPE_MASK) != PACKET_TYPE_DATA)
        return false;
    if ((uiFlags & PACKET_FLAG_RELIABLE) != 0)
        return false;
    if ((uiFlags & PACKET_FLAG_ACK) != 0)
    {
        if (pPacket->m_uiSubStreamID > m_uiMaxSubStreamID)
            return false;
        return m_slidingWindows[pPacket->m_uiSubStreamID]->GetOldestUnacknowledged() > pPacket->m_sequenceID ? true
                                                                                                             : false;
    }
    if ((uiFlags & PACKET_FLAG_AGGREGATE_ACK) != 0)
        return false;
    bool bLate;
    if ((uiFlags & PACKET_FLAG_ALTERNATE_KEY_A) == 0 && (uiFlags & PACKET_FLAG_ALTERNATE_KEY_B) == 0)
    {
        bLate = m_expectedUnreliableSequence > pPacket->m_sequenceID;
    }
    else
    {
        if (m_uiBroadcastSessionID == 0)
            return false;
        bLate = m_expectedBroadcastSequence > pPacket->m_sequenceID;
    }
    if (bLate)
    {
        ++m_endPointInfo.m_uiLatePackets;
        return true;
    }
    return false;
}

bool PRUDPEndPoint::IsNegotiationConnectionSignatureVersion(const Packet* pPacket)
{
    if (pPacket->m_uiVersion == 0
        || (pPacket->m_uiVersion == 1 && static_cast<unsigned char>(pPacket->m_unknown90) < 2))
        return false;
    return true;
}

bool PRUDPEndPoint::IsSendAggregateACK()
{
    if (m_uiVersion == 0 || (m_uiSupportedFunctionsFlags & FUNCTION_AGGREGATE_ACK) == 0)
        return false;
    return true;
}

void PRUDPEndPoint::SetSupportedFunctionsFlags()
{
    unsigned long long uiFunctions = 0;
    unsigned int uiVersion = m_uiNegotiatedFlags & NEGOTIATED_VERSION_MASK;
    if (uiVersion != 0)
    {
        uiFunctions = FUNCTIONS_VERSION_1;
        if (uiVersion >= 2)
        {
            uiFunctions = FUNCTIONS_VERSION_2;
            if (uiVersion >= 3)
            {
                uiFunctions = FUNCTIONS_VERSION_3;
                if (uiVersion >= 4)
                    uiFunctions = FUNCTIONS_VERSION_4;
            }
        }
    }
    m_uiSupportedFunctionsFlags = uiFunctions | (m_uiNegotiatedFlags >> NEGOTIATED_FLAGS_SHIFT);
}

bool PRUDPEndPoint::IsAnyPacketProcessed()
{
    for (int i = 0; i < m_uiMaxSubStreamID + 1; ++i)
    {
        SequenceClock initialSequence;
        initialSequence = 1;
        if (m_dispatchQueues[i]->GetNextSequence() != initialSequence)
            return true;
    }
    return false;
}

int PRUDPEndPoint::GetMaxPacketSize()
{
    int iSize = RootTransport::GetInstance()->GetBufferSize();
    iSize -= Packet::GetOverhead();
    if (RootTransport::GetInstance()->GetRelay()->IsEnabled())
    {
        iSize -= MAX_UNRELIABLE_PAYLOAD_RESERVE_V1;
    }
    else
    {
        if (RootTransport::GetInstance()->GetRouter()->IsEnabled() == false && m_uiVersion != 0)
        {
            if (m_uiVersion != 1)
                return iSize;
            if ((m_uiNegotiatedFlags & NEGOTIATED_VERSION_MASK) != 0)
                return iSize;
        }
        iSize -= MAX_UNRELIABLE_PAYLOAD_RESERVE;
    }
    return iSize;
}

void PRUDPEndPoint::SendAggregateACK()
{
    if (IsSendAggregateACK())
    {
        if ((GetSupportedFunctionsFlags() & FUNCTION_AGGREGATE_ACK_EXTENDED) != 0)
        {
            SendAggregateACKNew();
            return;
        }
        if (m_dispatchQueues[0] != 0)
        {
            if (m_pendingAcknowledgements[0])
            {
                m_pendingAcknowledgements[0] = false;
                SendAggregateACKOld();
            }
        }
    }
}

void PRUDPEndPoint::DecideMaxUserReliableSubStreams(unsigned short uiMaxSubStreams)
{
    int iCount = uiMaxSubStreams + 1;
    m_uiMaxSubStreamID = uiMaxSubStreams;
    m_dispatchQueues.reserve(iCount);
    m_slidingWindows.reserve(iCount);
    m_pendingAcknowledgements.resize(iCount, false);
    m_defragBuffers.resize(iCount, 0);
    m_defragFragmentIDs.resize(iCount, 1);
    m_packetEncDec.DecideMaxUserReliableSubStreams(uiMaxSubStreams);
    for (int i = 0; i < uiMaxSubStreams + 1; ++i)
    {
        m_dispatchQueues.push_back(new PacketDispatchQueue());
        m_slidingWindows.push_back(
            new SlidingWindow(g_streamSettings[m_pStream->GetType()].Get().GetWindowSize(),
                              g_streamSettings[m_pStream->GetType()].Get().GetMaxBufferedPackets()));
    }
}

void PRUDPEndPoint::SendAggregateACK(Buffer* pBuffer, unsigned short uiSequence, unsigned char uiSubStreamID)
{
    PacketOut* pPacket;
    if (!pBuffer->IsPooled())
    {
        pPacket = new PacketOut(this, PACKET_TYPE_DATA, PACKET_FLAG_AGGREGATE_ACK, pBuffer, 0);
    }
    else
    {
        pPacket = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT)
            PacketBufferPacketOut(this, PACKET_TYPE_DATA, PACKET_FLAG_AGGREGATE_ACK, pBuffer, 0);
        if (pPacket == 0)
            return;
        if (!pPacket->IsValid())
        {
            delete pPacket;
            return;
        }
    }
    pPacket->m_uiVersion = m_uiVersion;
    pPacket->m_sequenceID = uiSequence;
    pPacket->m_uiSubStreamID = uiSubStreamID;
    SendPacket(pPacket);
    pPacket->ReleaseRef();
}

void PRUDPEndPoint::SendACK(PacketIn* pPacket)
{
    unsigned short uiType = pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK;
    PacketOut* pAcknowledgement;
    if (uiType == PACKET_TYPE_CONNECT)
    {
        pAcknowledgement = new PacketOut(this, PACKET_TYPE_CONNECT, PACKET_FLAG_ACK, pPacket->m_pPayload, 0);
        m_uiVersion = pPacket->m_uiVersion;
        m_uiNegotiatedFlags = pPacket->m_unknown90;
        pAcknowledgement->m_uiVersion = m_uiVersion;
        SetSupportedFunctionsFlags();
        DecideMaxUserReliableSubStreams(pPacket->m_unknown6b);
        pAcknowledgement->m_unknown90 = m_uiNegotiatedFlags;
        pAcknowledgement->m_unknown6b = static_cast<unsigned char>(m_uiMaxSubStreamID);
        InitUnreliableSeq(pAcknowledgement);
        pAcknowledgement->m_uiFragmentID = pPacket->m_uiFragmentID;
        pAcknowledgement->m_sequenceID = pPacket->m_sequenceID.GetValue();
        ++m_endPointInfo.m_uiAcksSent;
        SendPacket(pAcknowledgement);
        uiType = pAcknowledgement->m_uiTypeAndFlags;
    }
    else
    {
        if (uiType == PACKET_TYPE_DATA && pPacket->m_pPayload != 0 && pPacket->m_pPayload->IsPooled())
        {
            pAcknowledgement = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT) PacketBufferPacketOut(
                this, static_cast<PacketType>(pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK), PACKET_FLAG_ACK, 0, 0);
            if (pAcknowledgement == 0)
                return;
            if (!pAcknowledgement->IsValid())
            {
                delete pAcknowledgement;
                return;
            }
        }
        else
        {
            pAcknowledgement = new PacketOut(this, static_cast<PacketType>(uiType), PACKET_FLAG_ACK, 0, 0);
        }
        pAcknowledgement->m_uiVersion = m_uiVersion;
        ++m_endPointInfo.m_uiAcksSent;
        pAcknowledgement->m_sequenceID = pPacket->m_sequenceID.GetValue();
        pAcknowledgement->m_uiFragmentID = pPacket->m_uiFragmentID;
        SendPacket(pAcknowledgement);
        uiType = pAcknowledgement->m_uiTypeAndFlags;
    }
    if ((uiType & PACKET_TYPE_MASK) == PACKET_TYPE_DISCONNECT)
    {
        for (int i = 0; i < 2; ++i)
        {
            PacketOut* pDisconnect = new PacketOut(this, PACKET_TYPE_DISCONNECT, PACKET_FLAG_ACK, 0, 0);
            pDisconnect->m_uiFragmentID = pPacket->m_uiFragmentID;
            pDisconnect->m_uiVersion = m_uiVersion;
            pDisconnect->m_sequenceID = pPacket->m_sequenceID.GetValue();
            SendPacket(pDisconnect);
            pDisconnect->ReleaseRef();
        }
    }
    pAcknowledgement->ReleaseRef();
}

void PRUDPEndPoint::SendResetDisconnectPacket()
{
    PacketOut* pPacket;
    for (int i = 0; i < 2; ++i)
    {
        pPacket = new PacketOut(this, PACKET_TYPE_DISCONNECT, 0, 0, 0);
        pPacket->m_uiVersion = m_uiVersion;
        SendPacket(pPacket);
        pPacket->ReleaseRef();
    }
}

void PRUDPEndPoint::StartPing()
{
    Timeout* pTimeout;
    if (m_pPingPacket == 0)
    {
        m_pPingPacket = new PacketOut(this, PACKET_TYPE_PING, PACKET_FLAG_NEED_ACK, 0, 0);
        m_pPingPacket->m_uiVersion = m_uiVersion;
        if (GetKeepAliveTimeout() == 0 || GetKeepAliveTimeout() == NO_TIMEOUT)
        {
            m_pPingPacket->m_pTimeout->SetRTO(PING_RETRANSMIT_TIMEOUT);
            static_cast<PRUDPStream*>(m_pStream)->GetTimeoutManager().SchedulePacketTimeout(m_pPingPacket);
        }
        else
        {
            pTimeout = m_pPingPacket->m_pTimeout;
            pTimeout->SetRTO(GetKeepAliveTimeout());
            static_cast<PRUDPStream*>(m_pStream)->GetTimeoutManager().SchedulePacketTimeout(m_pPingPacket);
        }
    }
}

void PRUDPEndPoint::StopPing()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_pPingPacket != 0)
    {
        CancelPacketTimeout(m_pPingPacket);
        m_pPingPacket->ReleaseRef();
        m_pPingPacket = 0;
    }
}

void PRUDPEndPoint::SetKeepAliveTimeout(unsigned int uiTimeout)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_uiKeepAliveTimeout = uiTimeout;
    if (m_pPingPacket != 0)
        StopPing();
    StartPing();
}

void PRUDPEndPoint::SignalFaultEvent(unsigned int uiError, bool bProcessNow)
{
    if (m_bFaultIgnored)
    {
        m_bFaultIgnored = false;
        return;
    }
    if (bProcessNow)
        ProcessFaultEvent(uiError);
    else
        m_uiSignaledFaultError = uiError;
    if (uiError != NET_FAULT_KEEP_ALIVE_TIMEOUT)
        m_uiFaultReason = uiError;
}

bool PRUDPEndPoint::SetConnectionState(_ConnectionState state)
{
    _ConnectionState previousState = m_connectionState;
    if (state == Connected)
        m_bFaultIgnored = false;
    m_connectionState = state;
    if (m_ioRequest.m_pCallback != 0)
    {
        qResult result(RESULT_CONNECTION_FAILED);
        if (previousState == Connecting)
        {
            result = state == Connected ? qResult(RESULT_SUCCESS) : qResult(RESULT_CONNECTION_FAILED);
        }
        if (previousState == Disconnecting)
        {
            result = state == NotConnected ? qResult(RESULT_SUCCESS) : qResult(RESULT_CONNECTION_FAILED);
        }
        qResult callbackResult = result;
        ConnectionCallback pCallback = m_ioRequest.m_pCallback;
        if (m_ioRequest.m_pRequest != 0)
            m_ioRequest.m_pRequest->ReleaseRef();
        m_ioRequest.m_pResponse = 0;
        m_ioRequest.m_pRequest = 0;
        m_ioRequest.m_pCallback = 0;
        pCallback(this, callbackResult, &m_ioRequest.m_userContext);
    }
    return true;
}

bool PRUDPEndPoint::CompleteIO(qResult result)
{
    ConnectionCallback pCallback = m_ioRequest.m_pCallback;
    if (pCallback != 0)
    {
        qResult callbackResult = result;
        if (m_ioRequest.m_pRequest != 0)
            m_ioRequest.m_pRequest->ReleaseRef();
        m_ioRequest.m_pRequest = 0;
        m_ioRequest.m_pCallback = 0;
        m_ioRequest.m_pResponse = 0;
        pCallback(this, callbackResult, &m_ioRequest.m_userContext);
        if (m_ioRequest.m_pRequest != 0)
            m_ioRequest.m_pRequest->ReleaseRef();
        m_ioRequest.m_pResponse = 0;
        m_ioRequest.m_pRequest = 0;
        m_ioRequest.m_pCallback = 0;
        return true;
    }
    return false;
}

void PRUDPEndPoint::ProcessFaultEvent(unsigned int uiError)
{
    m_uiSignaledFaultError = uiError;
    {
        ScopedCS lock(&Network::GetInstance()->m_unknown43c);
        Network::GetInstance()->m_unknown42c.push_back(std::pair<void*, unsigned int>(this, m_uiConnectionID));
    }
    if (IsConnected() || PeerIsConnected())
    {
        if (uiError != NET_FAULT_KEEP_ALIVE_TIMEOUT && uiError != NET_FAULT_DISCONNECT)
            SendResetDisconnectPacket();
        SetConnectionState(Faulty);
        StopPing();
        if (m_pEventHandler != 0)
            m_pEventHandler->FaultDetection(this, uiError);
        SetPeerDisconnected();
        static_cast<PRUDPStream*>(m_pStream)->ReleaseEndPoint(this);
    }
    else
    {
        switch (m_connectionState)
        {
        case Connecting:
        case Disconnecting:
            SetConnectionState(NotConnected);
            break;
        default:
            break;
        }
    }
}

void PRUDPEndPoint::ServiceTimeout(PacketOut* pPacket)
{
    if ((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_PING)
    {
        TimeToPing();
        return;
    }
    if (pPacket->m_unknown118 < g_streamSettings[m_pStream->GetType()].Get().GetMaxRetransmission()
        && !pPacket->m_pTimeout->IsExpired())
    {
        SendReliable(pPacket);
        return;
    }
    if ((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_DISCONNECT
        && GetConnectionState() == Disconnecting)
        SetConnectionState(NotConnected);
    if (((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_CONNECT
         || (pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_SYN)
        && GetConnectionState() == Connecting)
        SetConnectionState(NotConnected);
    bool bFaulty;
    if (!pPacket->m_pTimeout->IsExpired())
    {
        m_uiFaultReason = NET_FAULT_RETRANSMISSION_EXCEEDED;
        IsFaulty();
        bFaulty = IsFaulty();
    }
    else
    {
        ++m_endPointInfo.m_uiTimeouts;
        m_uiFaultReason = NET_FAULT_RETRANSMISSION_EXPIRED;
        IsFaulty();
        bFaulty = IsFaulty();
    }
    if (!bFaulty)
        SignalFaultEvent(NET_FAULT_RETRANSMISSION_LIMIT, false);
}

void PRUDPEndPoint::SendNextReliable(unsigned char uiSubStreamID)
{
    m_slidingWindows[uiSubStreamID]->Trace(TRACE_FILTER_SLIDING_WINDOW);
    PacketOut* pPacket = m_slidingWindows[uiSubStreamID]->GetNextToSend();
    if (pPacket == 0)
        return;
    if (!g_streamSettings[m_pStream->GetType()].Get().UsesEarlyActivityTimestamps())
        m_lastSendTime = Time(Time::GetTime());
    SendReliable(pPacket);
}

void PRUDPEndPoint::SendReliable(PacketOut* pPacket)
{
    if ((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_DISCONNECT)
    {
        for (int i = 0; i < m_uiMaxSubStreamID + 1; ++i)
        {
            if (m_slidingWindows[i]->DataPending())
            {
                ++m_endPointInfo.m_uiDeferredReliableSends;
                static_cast<PRUDPStream*>(m_pStream)->GetTimeoutManager().SchedulePacketTimeout(pPacket);
                return;
            }
        }
    }
    ++pPacket->m_unknown118;
    if (m_endPointInfo.m_uiMaxRetransmissions < pPacket->m_unknown118)
        m_endPointInfo.m_uiMaxRetransmissions = pPacket->m_unknown118;
    if (pPacket->m_unknown118 > 1)
    {
        ++m_endPointInfo.m_uiRetransmissions;
        m_endPointInfo.m_uiRetransmittedBytes += GetPayloadSize(pPacket->m_pPayload);
    }
    pPacket->m_pTimeout->SetRTO(ComputeRetransmitTimeout(pPacket));
    static_cast<PRUDPStream*>(m_pStream)->GetTimeoutManager().SchedulePacketTimeout(pPacket);
    if (g_streamSettings[m_pStream->GetType()].Get().UsesEarlyActivityTimestamps())
        m_lastSendTime = Time(Time::GetTime());
    if (!static_cast<PRUDPStream*>(m_pStream)->Send(m_uiPort, m_stationURL.GetStreamID(), pPacket,
                                                    &m_packetEncDec.GetEncryptionKey()))
        ++m_endPointInfo.m_uiSendFailures;
}

qResult PRUDPEndPoint::SendPacket(PacketOut* pPacket)
{
    ++m_endPointInfo.m_uiPacketsSent;
    m_endPointInfo.m_uiBytesSent += GetPayloadSize(pPacket->m_pPayload);
    unsigned short uiFlags = pPacket->m_uiTypeAndFlags;
    if ((uiFlags & PACKET_FLAG_ALTERNATE_KEY_A) != 0)
    {
        InetAddress address;
        address = g_transportAdapter->GetBroadcastAddress();
        address.SetPortNumber(m_stationURL.GetInetAddress()->GetPortNumber());
        pPacket->m_address = address;
        ++m_endPointInfo.m_uiBroadcastPacketsSent;
        m_endPointInfo.m_uiBroadcastBytesSent += GetPayloadSize(pPacket->m_pPayload);
    }
    else if ((uiFlags & PACKET_FLAG_ALTERNATE_KEY_B) != 0)
    {
        InetAddress address;
        address = g_transportAdapter->GetMulticastAddress();
        pPacket->m_address = address;
        ++m_endPointInfo.m_uiMulticastPacketsSent;
        m_endPointInfo.m_uiMulticastBytesSent += GetPayloadSize(pPacket->m_pPayload);
    }
    else
    {
        pPacket->m_address = *m_stationURL.GetInetAddress();
    }
    memcpy(pPacket->m_aSignature, m_signature.m_bytes, m_signature.m_uiLength);
    uiFlags = pPacket->m_uiTypeAndFlags;
    pPacket->m_uiSignatureSize = m_signature.m_uiLength;
    pPacket->m_uiSessionID = m_uiSessionID;
    if ((uiFlags & PACKET_FLAG_RELIABLE) != 0)
    {
        ++m_endPointInfo.m_uiReliablePacketsSent;
        m_endPointInfo.m_uiReliableBytesSent += GetPayloadSize(pPacket->m_pPayload);
        if (pPacket->m_unknown118 == 0)
        {
            ++m_endPointInfo.m_uiQueuedReliablePackets;
            if (!m_slidingWindows[pPacket->m_uiSubStreamID]->Push(pPacket, &m_packetEncDec, m_pStream))
                return HandleSlidingWindowPushError();
        }
        SendNextReliable(pPacket->m_uiSubStreamID);
        return qResult(RESULT_SUCCESS);
    }
    ++m_endPointInfo.m_uiUnreliablePacketsSent;
    m_endPointInfo.m_uiUnreliableBytesSent += GetPayloadSize(pPacket->m_pPayload);
    if ((uiFlags & PACKET_FLAG_ACK) == 0 && (uiFlags & PACKET_FLAG_AGGREGATE_ACK) == 0
        && (uiFlags & PACKET_TYPE_MASK) == PACKET_TYPE_DATA)
    {
        ++m_endPointInfo.m_uiUnreliableDataPacketsSent;
        m_endPointInfo.m_uiUnreliableDataBytesSent += GetPayloadSize(pPacket->m_pPayload);
        if ((uiFlags & PACKET_FLAG_ALTERNATE_KEY_A) == 0 && (uiFlags & PACKET_FLAG_ALTERNATE_KEY_B) == 0)
        {
            pPacket->m_sequenceID = m_unreliableSequence;
            ++m_unreliableSequence;
            m_packetEncDec.Encode(pPacket, m_pStream->GetType(), true);
        }
        else
        {
            pPacket->m_sequenceID = *m_pBroadcastSequence;
            ++*m_pBroadcastSequence;
            m_packetEncDec.Encode(pPacket, m_pStream->GetType(), true);
        }
    }
    if (g_streamSettings[m_pStream->GetType()].Get().UsesEarlyActivityTimestamps())
        m_lastSendTime = Time(Time::GetTime());
    static_cast<PRUDPStream*>(m_pStream)->Send(m_uiPort, m_stationURL.GetStreamID(), pPacket,
                                               &m_packetEncDec.GetEncryptionKey());
    return qResult(RESULT_SUCCESS);
}

qResult PRUDPEndPoint::HandleSlidingWindowPushError()
{
    qResult result(true);
    ++m_endPointInfo.m_uiSlidingWindowPushFailures;
    m_uiFaultReason = NET_FAULT_SLIDING_WINDOW_PUSH;
    SignalFaultEvent(NET_FAULT_KEEP_ALIVE_TIMEOUT, true);
    if (m_pStream->GetType() != Stream::RendezVousSecure
        && (m_pStream->GetType() != Stream::Unknown1 || s_iPushErrorMode != 1))
    {
        result = qResult(RESULT_PUSH_ERROR);
        if (m_pErrorHolder != 0)
        {
            if (m_pErrorHolder->GetFirstError())
                m_pErrorHolder->SetFirstError(result);
        }
    }
    return result;
}

void PRUDPEndPoint::TimeToPing()
{
    Time currentTime(Time::GetTime());
    if (m_uiSignaledFaultError != 0)
    {
        ProcessFaultEvent(m_uiSignaledFaultError);
        return;
    }
    if (RootTransport::GetInstance()->IsStopped())
    {
        SignalFaultEvent(NET_FAULT_DISCONNECT, true);
        return;
    }
    if (static_cast<long long>(currentTime - m_lastReceiveTime) > GetMaxSilenceTime())
    {
        if (IsConnected() && s_bHybridPartialFaultProcessingMode
            && static_cast<PRUDPStream*>(m_pStream)->m_pEventHandler->SwitchToRoutingConnection(this))
            return;
        m_uiFaultReason = NET_FAULT_SILENCE_TIMEOUT;
        ProcessFaultEvent(NET_FAULT_KEEP_ALIVE_TIMEOUT);
        return;
    }
    StreamSettings& rSettings = g_streamSettings[m_pStream->GetType()].Get();
    if (static_cast<long long>(currentTime - m_lastSendTime) > GetKeepAliveTimeout()
        && rSettings.FaultDetectionIsActive())
    {
        ++m_pPingPacket->m_sequenceID;
        m_pPingPacket->m_bHeaderParsed = 0;
        if (GetKeepAliveTimeout() != 0 && GetKeepAliveTimeout() != NO_TIMEOUT)
        {
            ++m_endPointInfo.m_uiPingsSent;
            m_endPointInfo.m_uiLastPingSent = m_pPingPacket->m_sequenceID.GetValue();
            SendPacket(m_pPingPacket);
        }
    }
    static_cast<PRUDPStream*>(m_pStream)->GetTimeoutManager().SchedulePacketTimeout(m_pPingPacket);
}

void PRUDPEndPoint::PacketAcknowledged(const Time& rTime, const SequenceClock& rSequence, unsigned char uiSubStreamID)
{
    PacketOut* pPacket = m_slidingWindows[uiSubStreamID]->GetPacket(rSequence);
    if (pPacket == 0 || !CancelPacketTimeout(pPacket))
    {
        ++m_endPointInfo.m_uiUnmatchedAcks;
        return;
    }
    if (g_streamSettings[m_pStream->GetType()].Get().GetRTTRetransmit() > pPacket->m_unknown118)
        AdjustRTT(rTime, pPacket);
    Time sentTime(pPacket->m_unknownF8);
    m_slidingWindows[uiSubStreamID]->Acknowledged(rSequence, m_pendingFragmentCounts);
    SlidingWindow::PacketMap::iterator position = m_slidingWindows[uiSubStreamID]->AcquireIterator();
    PacketOut* pNext = m_slidingWindows[uiSubStreamID]->GetPacket(position);
    SequenceClock threshold;
    threshold = static_cast<unsigned short>(rSequence.GetValue() - 2);
    int iResent = 0;
    while (pNext != 0 && pNext->m_sequenceID - threshold <= 0)
    {
        if (pNext->m_unknownF8 < sentTime && pNext->m_unknown118 < RETRANSMISSION_RESEND_LIMIT)
        {
            CancelPacketTimeout(pNext);
            SendReliable(pNext);
            ++iResent;
        }
        ++position;
        pNext = m_slidingWindows[uiSubStreamID]->GetPacket(position);
        if (iResent > MAX_RESENDS_PER_ACK - 1)
            break;
    }
    m_slidingWindows[uiSubStreamID]->ReleaseIterator();
    SendNextReliable(uiSubStreamID);
}

void PRUDPEndPoint::PacketAggregateAcknoledged(Packet* pPacket)
{
    if ((GetSupportedFunctionsFlags() & FUNCTION_AGGREGATE_ACK_EXTENDED) != 0)
        PacketAggregateAcknoledgedNew(pPacket);
    else
        PacketAggregateAcknoledgedOld(pPacket);
}

void PRUDPEndPoint::PacketAggregateAcknoledgedOld(Packet* pPacket)
{
    SlidingWindow::PacketMap::iterator position = m_slidingWindows[0]->AcquireIterator();
    unsigned short uiSequence = pPacket->m_sequenceID.GetValue();
    PacketOut* pFirst = m_slidingWindows[0]->GetPacket(position);
    std::vector<SequenceClock, MemAllocator<SequenceClock> > acknowledged;
    while (pFirst != 0
           && (uiSequence == pFirst->m_sequenceID.GetValue()
               || static_cast<short>(uiSequence - pFirst->m_sequenceID.GetValue()) > 0))
    {
        acknowledged.push_back(position->first);
        ++position;
        pFirst = m_slidingWindows[0]->GetPacket(position);
    }
    m_slidingWindows[0]->ReleaseIterator();
    for (unsigned int i = 0; i < acknowledged.size(); ++i)
        PacketAcknowledged(Time(pPacket->m_unknownF8), acknowledged[i], 0);
    ByteStream stream(pPacket->m_pPayload);
    while (stream.GetPosition() < stream.GetBuffer()->GetLength())
    {
        unsigned short uiAcknowledged = 0;
        if (!stream.ExtractPrimitive(&uiAcknowledged))
            break;
        SequenceClock sequence;
        sequence = uiAcknowledged;
        PacketAcknowledged(Time(pPacket->m_unknownF8), sequence, 0);
    }
}

void PRUDPEndPoint::PacketAggregateAcknoledgedNew(Packet* pPacket)
{
    ByteStream stream(pPacket->m_pPayload);
    while (stream.GetPosition() < stream.GetBuffer()->GetLength())
    {
        unsigned char uiSubStreamID;
        unsigned char uiCount;
        unsigned short uiSequence;
        stream.ExtractPrimitive(&uiSubStreamID);
        stream.ExtractPrimitive(&uiCount);
        stream.ExtractPrimitive(&uiSequence);
        if (stream.HasError() || !stream.ValidateBufferLimit(uiCount << 1))
            break;
        if (m_uiMaxSubStreamID < uiSubStreamID)
        {
            unsigned int uiPosition = stream.GetPosition() + uiCount * 2;
            if (uiPosition <= stream.GetBuffer()->GetLength())
                stream.SetPosition(uiPosition);
        }
        else
        {
            SlidingWindow::PacketMap::iterator position = m_slidingWindows[uiSubStreamID]->AcquireIterator();
            PacketOut* pFirst = m_slidingWindows[uiSubStreamID]->GetPacket(position);
            std::vector<SequenceClock, MemAllocator<SequenceClock> > acknowledged;
            while (pFirst != 0
                   && (uiSequence == pFirst->m_sequenceID.GetValue()
                       || static_cast<short>(uiSequence - pFirst->m_sequenceID.GetValue()) > 0))
            {
                acknowledged.push_back(position->first);
                ++position;
                pFirst = m_slidingWindows[uiSubStreamID]->GetPacket(position);
            }
            m_slidingWindows[uiSubStreamID]->ReleaseIterator();
            for (unsigned int i = 0; i < acknowledged.size(); ++i)
                PacketAcknowledged(Time(pPacket->m_unknownF8), acknowledged[i], uiSubStreamID);
            for (int i = 0; i < uiCount; ++i)
            {
                unsigned short uiAcknowledged = 0;
                if (!stream.ExtractPrimitive(&uiAcknowledged))
                    break;
                SequenceClock sequence;
                sequence = uiAcknowledged;
                PacketAcknowledged(Time(pPacket->m_unknownF8), sequence, uiSubStreamID);
            }
        }
    }
}

void PRUDPEndPoint::SendAggregateACKOld()
{
    PacketDispatchQueue* pQueue = m_dispatchQueues[0];
    PacketDispatchQueue::PacketMap::iterator position = pQueue->m_packets.begin();
    unsigned short uiLastSequence = pQueue->m_nextToDispatch.GetValue() - 1;
    while (position != pQueue->m_packets.end()
           && position->first.GetValue() == static_cast<unsigned short>(uiLastSequence + 1))
    {
        uiLastSequence = position->first.GetValue();
        ++position;
    }
    bool bPooled = true;
    ByteStream* pStream;
    if (m_pStream->GetType() == Stream::RendezVousSecure || m_pStream->GetType() == Stream::RendezVous)
    {
        bPooled = false;
        pStream = new ByteStream(AGGREGATE_ACK_OLD_BUFFER_SIZE);
    }
    else
    {
        Buffer* pBuffer = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT)
            PacketBuffer(AGGREGATE_ACK_OLD_BUFFER_SIZE, Buffer::BUFFER_FLAG_POOLED);
        if (pBuffer == 0)
            return;
        if (!pBuffer->IsValid())
        {
            delete pBuffer;
            return;
        }
        pStream = new ByteStream(pBuffer);
        pBuffer->ReleaseRef();
    }
    int iSequenceCount = 0;
    int iPacketsSent = 0;
    while (position != pQueue->m_packets.end())
    {
        if (!pStream->GetBuffer()->AttemptExpand(2))
            break;
        unsigned short uiSequence = position->first.GetValue();
        pStream->Append(reinterpret_cast<const unsigned char*>(&uiSequence), 2, 1);
        ++iSequenceCount;
        if (iSequenceCount > AGGREGATE_ACK_OLD_SEQUENCES_PER_PACKET - 1)
        {
            SendAggregateACK(pStream->GetBuffer(), uiLastSequence, 0);
            iSequenceCount = 0;
            delete pStream;
            if (bPooled)
            {
                Buffer* pBuffer = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT)
                    PacketBuffer(AGGREGATE_ACK_OLD_BUFFER_SIZE, Buffer::BUFFER_FLAG_POOLED);
                if (pBuffer == 0)
                    return;
                if (!pBuffer->IsValid())
                {
                    delete pBuffer;
                    return;
                }
                pStream = new ByteStream(pBuffer);
                pBuffer->ReleaseRef();
                ++iPacketsSent;
            }
            else
            {
                pStream = new ByteStream(AGGREGATE_ACK_OLD_BUFFER_SIZE);
                ++iPacketsSent;
            }
        }
        ++position;
    }
    if (iSequenceCount < 1 && iPacketsSent != 0)
    {
        if (pStream != 0)
            delete pStream;
    }
    else
    {
        SendAggregateACK(pStream->GetBuffer(), uiLastSequence, 0);
        delete pStream;
    }
}

void PRUDPEndPoint::SendAggregateACKNew()
{
    bool bPooled = true;
    Buffer* pBuffer;
    if (m_pStream->GetType() == Stream::RendezVousSecure || m_pStream->GetType() == Stream::RendezVous)
    {
        bPooled = false;
        pBuffer = new Buffer(AGGREGATE_ACK_BUFFER_SIZE, 0);
    }
    else
    {
        pBuffer = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT)
            PacketBuffer(AGGREGATE_ACK_BUFFER_SIZE, Buffer::BUFFER_FLAG_POOLED);
        if (pBuffer == 0)
            return;
        if (!pBuffer->IsValid())
        {
            delete pBuffer;
            return;
        }
    }
    ByteStream stream(pBuffer);
    pBuffer->ReleaseRef();
    std::vector<unsigned short, MemAllocator<unsigned short> > sequences;
    for (int i = 0; i < m_uiMaxSubStreamID + 1; ++i)
    {
        PacketDispatchQueue* pQueue = m_dispatchQueues[i];
        if (pQueue != 0 && m_pendingAcknowledgements[i])
        {
            m_pendingAcknowledgements[i] = false;
            unsigned short uiLastSequence = pQueue->m_nextToDispatch.GetValue() - 1;
            PacketDispatchQueue::PacketMap::iterator position = pQueue->m_packets.begin();
            while (position != pQueue->m_packets.end()
                   && position->first.GetValue() == static_cast<unsigned short>(uiLastSequence + 1))
            {
                uiLastSequence = position->first.GetValue();
                ++position;
            }
            sequences.clear();
            while (position != pQueue->m_packets.end())
            {
                sequences.push_back(position->first.GetValue());
                ++position;
            }
            int iRecordSize = sequences.size() * 2 + 4;
            if (!stream.GetBuffer()->AttemptExpand(iRecordSize))
                break;
            if (stream.GetBuffer()->GetLength() + iRecordSize > AGGREGATE_ACK_SEND_THRESHOLD - 1)
            {
                SendAggregateACK(stream.GetBuffer(), 0, 1);
                if (bPooled)
                {
                    pBuffer = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT)
                        PacketBuffer(AGGREGATE_ACK_BUFFER_SIZE, Buffer::BUFFER_FLAG_POOLED);
                    if (pBuffer == 0)
                        return;
                    if (!pBuffer->IsValid())
                    {
                        delete pBuffer;
                        return;
                    }
                }
                else
                {
                    pBuffer = new Buffer(AGGREGATE_ACK_BUFFER_SIZE, 0);
                }
                stream = ByteStream(pBuffer);
                pBuffer->ReleaseRef();
            }
            unsigned char uiValue = static_cast<unsigned char>(i);
            stream.Append(&uiValue, 1, 1);
            uiValue = static_cast<unsigned char>(sequences.size());
            stream.Append(&uiValue, 1, 1);
            stream.Append(reinterpret_cast<const unsigned char*>(&uiLastSequence), 2, 1);
            for (unsigned int j = 0; j < sequences.size(); ++j)
                stream.Append(reinterpret_cast<const unsigned char*>(&sequences[j]), 2, 1);
        }
    }
    if (stream.GetBuffer()->GetLength() != 0)
        SendAggregateACK(stream.GetBuffer(), 0, 1);
}

qResult PRUDPEndPoint::Frag(Buffer* pBuffer, unsigned int uiBufferSize, unsigned int uiFragmentSize, PacketType type,
                            unsigned short uiFlags, bool bNotify, unsigned int uiKey, unsigned char uiSubStreamID)
{
    std::vector<PacketOut*, MemAllocator<PacketOut*> > fragments;
    unsigned int uiOffset = 0;
    unsigned char uiFragmentID = 1;
    int iCount = 0;
    if (pBuffer->GetLength() != 0)
    {
        do
        {
            Buffer* pFragment;
            if (!pBuffer->IsPooled())
            {
                pFragment = new Buffer(uiBufferSize, 0);
            }
            else
            {
                pFragment = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT)
                    PacketBuffer(uiBufferSize, Buffer::BUFFER_FLAG_POOLED);
                if (pFragment == 0)
                    break;
                if (!pFragment->IsValid())
                {
                    delete pFragment;
                    break;
                }
            }
            unsigned int uiLength = pBuffer->GetLength() - uiOffset;
            if (uiFragmentSize < uiLength)
                uiLength = uiFragmentSize;
            pFragment->AppendData(pBuffer->GetData() + uiOffset, uiLength, APPEND_AT_END);
            PacketOut* pPacket;
            if (pBuffer->IsPooled())
            {
                pPacket = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT)
                    PacketBufferPacketOut(this, type, uiFlags, pFragment, uiKey);
                if (pPacket == 0 || !pPacket->IsValid())
                {
                    pFragment->ReleaseRef();
                    if (pPacket != 0)
                        delete pPacket;
                    break;
                }
            }
            else
            {
                pPacket = new PacketOut(this, type, uiFlags, pFragment, uiKey);
            }
            pPacket->m_unknown128 = bNotify;
            pPacket->m_uiVersion = m_uiVersion;
            uiOffset += uiFragmentSize;
            pPacket->m_uiSubStreamID = uiSubStreamID;
            if (uiOffset < pBuffer->GetLength())
            {
                pPacket->m_uiFragmentID = uiFragmentID;
                ++uiFragmentID;
                if (uiFragmentID == 0)
                    uiFragmentID = 1;
                fragments.push_back(pPacket);
            }
            else
            {
                pPacket->m_uiFragmentID = 0;
                fragments.push_back(pPacket);
            }
            ++iCount;
        } while (uiOffset < pBuffer->GetLength());
    }
    std::vector<PacketOut*, MemAllocator<PacketOut*> >::iterator position = fragments.begin();
    while (true)
    {
        if (position == fragments.end())
        {
            if ((uiFlags & PACKET_FLAG_RELIABLE) != 0 && uiKey != 0)
                m_pendingFragmentCounts[uiKey] = iCount;
            return qResult(RESULT_SUCCESS);
        }
        Buffer* pPayload = (*position)->m_pPayload;
        qResult result = SendPacket(*position);
        if (result == false)
        {
            for (; position != fragments.end(); ++position)
            {
                pPayload = (*position)->m_pPayload;
                (*position)->ReleaseRef();
                pPayload->ReleaseRef();
            }
            return result;
        }
        (*position)->ReleaseRef();
        pPayload->ReleaseRef();
        ++position;
    }
}

qResult PRUDPEndPoint::_Send(Buffer* pBuffer, unsigned int uiFlags, bool bCopyPayload, unsigned char uiSubStreamID,
                             unsigned int uiKey)
{
    if (!IsConnected() && !PeerIsConnected())
        return qResult(RESULT_INVALID_STATE);
    if (RootTransport::GetInstance()->IsStopped())
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        ProcessFaultEvent(NET_FAULT_DISCONNECT);
        return qResult(RESULT_INVALID_STATE);
    }
    if (pBuffer->IsPooled())
    {
        if (PacketBufferManager::GetInstance() != 0
            && !PacketBufferManager::GetInstance()->IsAllocatable(
                PacketBufferManager::MaxUnitSize, PacketBufferManager::GetInstance()->GetUnitAlignment()))
            return qResult(RESULT_PACKET_ALLOCATION_FAILED);
    }
    unsigned int uiBufferSize = RootTransport::GetInstance()->GetBufferSize();
    unsigned int uiMaxPacketSize = GetMaxPacketSize();
    unsigned int uiPacketFlags = 0;
    if ((uiFlags & SEND_FLAG_RELIABLE) != 0)
        uiPacketFlags = PACKET_FLAG_RELIABLE | PACKET_FLAG_NEED_ACK;
    if ((uiFlags & SEND_FLAG_TRAILER) != 0)
        uiPacketFlags = uiPacketFlags | PACKET_FLAG_TRAILER;
    if ((uiFlags & SEND_FLAG_BROADCAST) != 0)
        uiPacketFlags = uiPacketFlags | PACKET_FLAG_ALTERNATE_KEY_A;
    if ((uiFlags & SEND_FLAG_MULTICAST) != 0)
        uiPacketFlags = uiPacketFlags | PACKET_FLAG_ALTERNATE_KEY_B;
    if ((uiFlags & SEND_FLAG_HISTORY_BUNDLE) != 0)
        uiPacketFlags = uiPacketFlags | PACKET_FLAG_HISTORY_BUNDLE;
    if (uiMaxPacketSize < pBuffer->GetLength())
    {
        if ((uiFlags & SEND_FLAG_RELIABLE) == 0)
        {
            ++m_endPointInfo.m_uiOversizedUnreliableSends;
            return qResult(RESULT_PAYLOAD_TOO_LARGE);
        }
        ++m_endPointInfo.m_uiFragmentedSends;
        return Frag(pBuffer, uiBufferSize, uiMaxPacketSize, PACKET_TYPE_DATA, uiPacketFlags,
                    (uiFlags & SEND_FLAG_NOTIFY) != 0, uiKey, uiSubStreamID);
    }
    PacketOut* pPacket;
    if (pBuffer->IsPooled())
    {
        pPacket = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT)
            PacketBufferPacketOut(this, PACKET_TYPE_DATA, uiPacketFlags, pBuffer, uiKey);
        if (pPacket == 0 || !pPacket->IsValid())
        {
            if (pPacket != 0)
                delete pPacket;
            return qResult(RESULT_PACKET_ALLOCATION_FAILED);
        }
    }
    else
    {
        pPacket = new PacketOut(this, PACKET_TYPE_DATA, uiPacketFlags, pBuffer, uiKey);
    }
    if (bCopyPayload && !pPacket->SetInternalPayload(pBuffer))
    {
        pPacket->ReleaseRef();
        return qResult(RESULT_PACKET_ALLOCATION_FAILED);
    }
    pPacket->m_uiFragmentID = 0;
    pPacket->m_unknown128 = (uiFlags & SEND_FLAG_NOTIFY) != 0;
    pPacket->m_uiSubStreamID = uiSubStreamID;
    pPacket->m_uiVersion = m_uiVersion;
    qResult result = SendPacket(pPacket);
    if (result == false)
    {
        pPacket->ReleaseRef();
        return result;
    }
    if ((pPacket->m_uiTypeAndFlags & PACKET_FLAG_RELIABLE) != 0 && uiKey != 0)
        m_pendingFragmentCounts[uiKey] = 1;
    pPacket->ReleaseRef();
    return qResult(RESULT_SUCCESS);
}

qResult PRUDPEndPoint::_Connect(Buffer* pRequestData, Buffer* pResponseData, ConnectionCallback pCallback,
                                const UserContext& rUserContext, unsigned int uiTimeout)
{
    if (uiTimeout == NO_TIMEOUT)
        uiTimeout = DEFAULT_CONNECT_TIMEOUT;
    if (!PeerIsConnected() && m_connectionState != NotConnected)
        return qResult(RESULT_INVALID_STATE);
    if (IsConnecting() || IsConnected())
        return qResult(RESULT_INVALID_STATE);
    m_bConnectSignatureSent = false;
    SetConnectionState(Connecting);
    if (pRequestData != 0)
        pRequestData->AddRef();
    m_ioRequest.m_pRequest = pRequestData;
    m_ioRequest.m_pResponse = pResponseData;
    m_ioRequest.m_pCallback = pCallback;
    m_ioRequest.m_userContext = rUserContext;
    Timeout* pTimeout = m_pConnectPacket->m_pTimeout;
    if (!Core::GetInstance()->GetTerminateImmediately())
    {
        pTimeout->SetRelativeExpirationTime(uiTimeout);
        m_pConnectPacket->m_address = *m_stationURL.GetInetAddress();
        SendReliable(m_pConnectPacket);
        m_bFaultIgnored = !PeerIsConnected();
    }
    else
    {
        pTimeout->SetRelativeExpirationTime(
            static_cast<int>(Core::GetInstance()->GetTerminateImmediatelyTime() + g_terminateImmediatelyTimeout));
        m_pConnectPacket->m_address = *m_stationURL.GetInetAddress();
        SendReliable(m_pConnectPacket);
        m_bFaultIgnored = !PeerIsConnected();
    }
    if (pCallback != 0)
        return qResult(RESULT_SUCCESS);
    if (!Core::GetInstance()->GetTerminateImmediately())
    {
        SpinTest spinTest(0x32, uiTimeout);
        spinTest.LeaveOnTimeout();
        while (IsConnecting())
        {
            if (Core::GetInstance()->GetTerminateImmediately())
                break;
            Scheduler::GlobalSingleThreadSpecialDispatch();
            if (spinTest.SpinOnce(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Transport/PRUDP/"
                                  L"PRUDPEndPoint.cpp",
                                  SPIN_CONNECT_WAIT_LINE,
                                  L"(IsConnecting() && !Core::GetInstance()->GetTerminateImmediately())")
                == 0)
                break;
        }
    }
    else
    {
        SpinTest spinTest(0x32, g_terminateImmediatelyTimeout);
        spinTest.LeaveOnTimeout();
        while (IsConnecting())
        {
            Scheduler::GlobalSingleThreadSpecialDispatch();
            if (spinTest.SpinOnce(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Transport/PRUDP/"
                                  L"PRUDPEndPoint.cpp",
                                  SPIN_CONNECT_STATE_LINE, L"IsConnecting()")
                == 0)
                break;
        }
    }
    if (m_ioRequest.m_pRequest != 0)
        m_ioRequest.m_pRequest->ReleaseRef();
    m_ioRequest.m_pCallback = 0;
    m_ioRequest.m_pResponse = 0;
    m_ioRequest.m_pRequest = 0;
    if (!IsConnected())
    {
        SetConnectionState(NotConnected);
        return qResult(RESULT_CONNECTION_INCOMPLETE);
    }
    return qResult(RESULT_SUCCESS);
}

qResult PRUDPEndPoint::_Disconnect(ConnectionCallback pCallback, const UserContext& rUserContext,
                                   unsigned int uiTimeout)
{
    if (m_iOpenCount == 0)
        return qResult(RESULT_INVALID_STATE);
    if (!IsConnected())
        return qResult(RESULT_INVALID_STATE);
    SendAggregateACK();
    SetConnectionState(Disconnecting);
    m_ioRequest.m_pCallback = pCallback;
    m_ioRequest.m_pResponse = 0;
    m_ioRequest.m_pRequest = 0;
    m_ioRequest.m_userContext = rUserContext;
    PacketOut* pPacket = new PacketOut(this, PACKET_TYPE_DISCONNECT, PACKET_FLAG_RELIABLE | PACKET_FLAG_NEED_ACK, 0, 0);
    pPacket->m_uiVersion = m_uiVersion;
    Timeout* pTimeout = pPacket->m_pTimeout;
    qResult result;
    if (!Core::GetInstance()->GetTerminateImmediately())
    {
        pTimeout->SetRelativeExpirationTime(uiTimeout);
        pTimeout->SetRTO(DISCONNECT_RETRANSMIT_TIMEOUT);
        static_cast<PRUDPStream*>(m_pStream)->GetTimeoutManager().SchedulePacketTimeout(pPacket);
        result = SendPacket(pPacket);
    }
    else
    {
        pTimeout->SetRelativeExpirationTime(
            static_cast<int>(Core::GetInstance()->GetTerminateImmediatelyTime() + g_terminateImmediatelyTimeout));
        pTimeout->SetRTO(DISCONNECT_RETRANSMIT_TIMEOUT);
        static_cast<PRUDPStream*>(m_pStream)->GetTimeoutManager().SchedulePacketTimeout(pPacket);
        result = SendPacket(pPacket);
    }
    if (result == false)
    {
        pPacket->ReleaseRef();
        if (m_ioRequest.m_pRequest != 0)
            m_ioRequest.m_pRequest->ReleaseRef();
        m_ioRequest.m_pResponse = 0;
        m_ioRequest.m_pCallback = 0;
        m_ioRequest.m_pRequest = 0;
        return result;
    }
    pPacket->ReleaseRef();
    if (pCallback != 0)
        return qResult(RESULT_SUCCESS);
    if (!Core::GetInstance()->GetTerminateImmediately())
    {
        SpinTest spinTest(0x32, uiTimeout);
        spinTest.LeaveOnTimeout();
        while (IsDisconnecting())
        {
            if (Core::GetInstance()->GetTerminateImmediately())
                break;
            Scheduler::GlobalSingleThreadSpecialDispatch();
            if (spinTest.SpinOnce(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Transport/PRUDP/"
                                  L"PRUDPEndPoint.cpp",
                                  SPIN_DISCONNECT_WAIT_LINE,
                                  L"(IsDisconnecting() && !Core::GetInstance()->GetTerminateImmediately())")
                == 0)
                break;
        }
    }
    else
    {
        SpinTest spinTest(0x32, g_terminateImmediatelyTimeout);
        spinTest.LeaveOnTimeout();
        while (IsDisconnecting())
        {
            Scheduler::GlobalSingleThreadSpecialDispatch();
            if (spinTest.SpinOnce(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Transport/PRUDP/"
                                  L"PRUDPEndPoint.cpp",
                                  SPIN_DISCONNECT_STATE_LINE, L"IsDisconnecting()")
                == 0)
                break;
        }
    }
    if (m_ioRequest.m_pRequest != 0)
        m_ioRequest.m_pRequest->ReleaseRef();
    m_ioRequest.m_pResponse = 0;
    m_ioRequest.m_pCallback = 0;
    m_ioRequest.m_pRequest = 0;
    if (!IsNotConnected())
    {
        SetConnectionState(NotConnected);
        return qResult(RESULT_CONNECTION_INCOMPLETE);
    }
    return qResult(RESULT_SUCCESS);
}
PRUDPEndPoint::PRUDPEndPoint(ConnectionOrientedStream* pStream, const StationURL* pStationURL,
                             SequenceClock* pBroadcastSequence, unsigned char uiVersion)
    : EndPoint(pStream, pStationURL),
      m_uiPort(0),
      m_dispatchQueues(1),
      m_slidingWindows(1),
      m_rtt(g_streamSettings[pStream->GetType()].Get().GetInitialRTT()),
      m_defragBuffers(1),
      m_defragFragmentIDs(1),
      m_pendingAcknowledgements(1),
      m_sendProfile(L"Send", 1000, L"")
{
    m_bPeerConnected = false;
    m_bFaultIgnored = false;
    m_connectionState = NotConnected;
    m_dispatchQueues[0] = new PacketDispatchQueue();
    StreamSettings& rSettings = g_streamSettings[pStream->GetType()].Get();
    m_slidingWindows[0] = new SlidingWindow(rSettings.GetWindowSize(), rSettings.GetMaxBufferedPackets());
    m_uiNegotiatedFlags = 0;
    m_pPingPacket = 0;
    m_expectedUnreliableSequence = 0;
    m_unreliableSequence = 0;
    m_pBroadcastSequence = pBroadcastSequence;
    m_uiVersion = uiVersion;
    m_uiSignaledFaultError = 0;
    m_expectedBroadcastSequence = 0;
    m_uiNegotiatedFlags = (g_negotiationFlagsMask << NEGOTIATED_FLAGS_SHIFT) | g_negotiationVersion.Get();
    if ((pStream->GetType() != Stream::RendezVousSecure && pStream->GetType() != Stream::RendezVous)
        || rSettings.GetSecureMode() != SECURE_MODE_ENABLED)
        m_uiNegotiatedFlags &= ~NEGOTIATED_FLAG_SECURE_STREAM_MODE;
    m_uiMaxSubStreamID = rSettings.GetMaxSubStreamID();
    m_pConnectPacket = new PacketOut(this, PACKET_TYPE_SYN, PACKET_FLAG_NEED_ACK, 0, 0);
    m_pConnectPacket->m_uiVersion = uiVersion;
    m_pConnectPacket->m_unknown90 = m_uiNegotiatedFlags;
    m_pConnectPacket->m_unknown6b = static_cast<unsigned char>(m_uiMaxSubStreamID);
    m_defragBuffers[0] = 0;
    m_defragFragmentIDs[0] = 1;
    unsigned char& rSessionCounter = g_sessionCounter.Get();
    if (rSessionCounter == 0)
        rSessionCounter = static_cast<unsigned char>(static_cast<unsigned long long>(Time::GetTime())
                                                     + Platform::GetRandomNumber(SESSION_RANDOM_RANGE));
    else
        ++rSessionCounter;
    if (rSessionCounter == 0)
        ++rSessionCounter;
    m_uiSessionID = rSessionCounter;
    m_uiSupportedFunctionsFlags = 0;
    m_uiBroadcastSessionID = 0;
    m_bConnectSignatureSent = false;
    m_uiPeerSessionID = 0;
    m_uiFaultReason = 0;
    m_uiMaxSilenceTime = rSettings.GetMaxSilenceTime();
    m_uiKeepAliveTimeout = rSettings.GetKeepAliveTimeout();
    m_pMessageSelector = &Network::GetInstance()->GetMessageSelector();
    ResetEndPointInfo();
}

PRUDPEndPoint::~PRUDPEndPoint()
{
    StopPing();
    if (m_pConnectPacket != 0)
    {
        static_cast<PRUDPStream*>(m_pStream)->GetTimeoutManager().CancelPacketTimeout(m_pConnectPacket);
        m_pConnectPacket->ReleaseRef();
        m_pConnectPacket = 0;
    }
    for (SlidingWindowVector::iterator window = m_slidingWindows.begin(); window != m_slidingWindows.end(); ++window)
    {
        SlidingWindow::PacketMap::iterator position = (*window)->AcquireIterator();
        PacketOut* pPacket = (*window)->GetPacket(position);
        while (pPacket != 0)
        {
            CancelPacketTimeout(pPacket);
            ++position;
            pPacket = (*window)->GetPacket(position);
        }
        (*window)->ReleaseIterator();
        (*window)->Purge();
        delete *window;
    }
    for (DispatchQueueVector::iterator queue = m_dispatchQueues.begin(); queue != m_dispatchQueues.end(); ++queue)
    {
        (*queue)->Purge();
        delete *queue;
    }
    for (BufferVector::iterator buffer = m_defragBuffers.begin(); buffer != m_defragBuffers.end(); ++buffer)
    {
        if (*buffer != 0)
            (*buffer)->ReleaseRef();
    }
}

void PRUDPEndPoint::ServiceIncomingPacket(PacketIn* pPacket)
{
    unsigned short uiFlags = pPacket->m_uiTypeAndFlags;
    unsigned short uiType = uiFlags & PACKET_TYPE_MASK;
    bool bDispatch = false;
    if (m_pStream->GetType() == Stream::RendezVousSecure || m_pStream->GetType() == Stream::RendezVous)
    {
        if (uiType == PACKET_TYPE_SYN && (uiFlags & PACKET_FLAG_ACK) != 0 && IsConnecting() && !m_bConnectSignatureSent
            && (pPacket->m_unknown90 & NEGOTIATED_FLAG_SECURE_STREAM_MODE) != 0)
        {
            ProcessFaultEvent(NET_FAULT_CONNECT_REQUEST);
            return;
        }
    }
    else if ((uiFlags & (PACKET_FLAG_ACK | PACKET_FLAG_AGGREGATE_ACK)) == 0 && uiType == PACKET_TYPE_DATA
             && (uiFlags & PACKET_FLAG_HISTORY_BUNDLE) == 0)
    {
        if (PacketBufferManager::GetInstance() != 0
            && !PacketBufferManager::GetInstance()->IsAllocatable(
                PacketBufferManager::MaxUnitSize, PacketBufferManager::GetInstance()->GetUnitHeaderSize()))
            return;
    }
    if (g_streamSettings[m_pStream->GetType()].Get().UsesEarlyActivityTimestamps())
        m_lastReceiveTime = Time(g_lastTime);
    uiFlags = pPacket->m_uiTypeAndFlags;
    if (((uiFlags & PACKET_FLAG_ALTERNATE_KEY_A) != 0 || (uiFlags & PACKET_FLAG_ALTERNATE_KEY_B) != 0)
        && m_BroadcastState != BroadcastUnrestricted)
    {
        if (m_uiPeerSessionID == 0)
            return;
        if (m_connectionState != Connected)
            return;
        if (m_BroadcastState == BroadcastEnabled)
            return;
    }
    if ((uiFlags & PACKET_FLAG_AGGREGATE_ACK) == 0 && m_uiMaxSubStreamID < pPacket->m_uiSubStreamID)
        return;
    if ((uiFlags & PACKET_FLAG_ALTERNATE_KEY_A) == 0 && (uiFlags & PACKET_FLAG_ALTERNATE_KEY_B) == 0)
    {
        if ((uiType == PACKET_TYPE_SYN && (uiFlags & PACKET_FLAG_ACK) != 0)
            || (uiType == PACKET_TYPE_CONNECT && (uiFlags & PACKET_FLAG_ACK) == 0))
        {
            if (!IsNegotiationConnectionSignatureVersion(pPacket))
            {
                memcpy(m_signature.m_bytes, pPacket->m_aConnectionSignature, pPacket->m_uiConnectionSignatureSize);
                m_signature.m_uiLength = pPacket->m_uiConnectionSignatureSize;
            }
            else
            {
                memcpy(m_negotiationSignature.m_bytes, pPacket->m_aConnectionSignature,
                       pPacket->m_uiConnectionSignatureSize);
                m_negotiationSignature.m_uiLength = pPacket->m_uiConnectionSignatureSize;
            }
        }
        if (uiType != PACKET_TYPE_SYN)
        {
            unsigned char uiSessionID = pPacket->m_uiSessionID;
            if (m_uiPeerSessionID != 0)
            {
                if (uiSessionID != m_uiPeerSessionID)
                {
                    if (uiType != PACKET_TYPE_CONNECT)
                        return;
                    if ((pPacket->m_uiTypeAndFlags & PACKET_FLAG_ACK) != 0)
                        return;
                    ProcessFaultEvent(NET_FAULT_VERSION_MISMATCH);
                    return;
                }
            }
            else if ((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) != PACKET_TYPE_CONNECT)
            {
                if (uiSessionID != 0)
                    return;
            }
            else
            {
                m_uiPeerSessionID = uiSessionID;
                if (pPacket->m_uiVersion != 0 && pPacket->m_unknown4c != SequenceClock())
                    m_expectedUnreliableSequence = pPacket->m_unknown4c;
                Stream::Type streamType = VirtualPort(pPacket->m_uiSource).GetType();
                if ((streamType == Stream::Unknown1 || streamType == Stream::Unknown9)
                    && g_streamSettings[streamType].Get().IsEncryptionRequired())
                {
                    LiteBuffer<StreamSettings::KEY_BUFFER_SIZE> keyBuffer =
                        g_streamSettings[streamType].Get().GetEncryptionKey();
                    Key key(keyBuffer.GetData(), keyBuffer.GetLength());
                    if (key.GetLength() != 0)
                    {
                        m_packetEncDec.SetUnreliableEncryptionKey(key);
                        m_packetEncDec.SetReliableEncryptionKey(key, m_uiSessionID, m_uiPeerSessionID);
                    }
                }
            }
        }
    }
    if ((pPacket->m_uiTypeAndFlags & PACKET_FLAG_NEED_ACK) != 0)
    {
        if ((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) != PACKET_TYPE_DATA || !IsSendAggregateACK())
            SendACK(pPacket);
        else
            m_pendingAcknowledgements[pPacket->m_uiSubStreamID] = true;
    }
    if (!g_streamSettings[m_pStream->GetType()].Get().UsesEarlyActivityTimestamps())
        m_lastReceiveTime = Time(g_lastTime);
    if ((pPacket->m_uiTypeAndFlags & PACKET_FLAG_RELIABLE) != 0)
    {
        ++m_endPointInfo.m_uiReliablePacketsReceived;
        m_endPointInfo.m_uiReliableBytesReceived += GetPayloadSize(pPacket->m_pPayload);
        m_dispatchQueues[pPacket->m_uiSubStreamID]->Queue(pPacket);
    }
    if (uiType == PACKET_TYPE_SYN)
    {
        if (IsConnecting() && (pPacket->m_uiTypeAndFlags & PACKET_FLAG_ACK) != 0 && !m_bConnectSignatureSent)
        {
            PacketOut* pConnect = new PacketOut(this, PACKET_TYPE_CONNECT, PACKET_FLAG_RELIABLE | PACKET_FLAG_NEED_ACK,
                                                m_ioRequest.m_pRequest, 0);
            m_uiVersion = pPacket->m_uiVersion;
            pConnect->m_uiVersion = pPacket->m_uiVersion;
            m_uiNegotiatedFlags = pPacket->m_unknown90;
            DecideMaxUserReliableSubStreams(pPacket->m_unknown6b);
            SetSupportedFunctionsFlags();
            pConnect->m_unknown90 = m_uiNegotiatedFlags;
            pConnect->m_unknown6b = static_cast<unsigned char>(m_uiMaxSubStreamID);
            InitUnreliableSeq(pConnect);
            SignatureBytes signature;
            signature.m_uiLength = 0;
            m_pMessageSelector->CalcSignature(
                pPacket, PRUDPMessageInterface::SIGNATURE_METHOD_1, 0, VirtualPort(pPacket->m_uiSource).GetType(),
                &static_cast<PRUDPStream*>(m_pStream)->m_signatureGenerator, 0, signature);
            memcpy(pConnect->m_aConnectionSignature, signature.m_bytes, signature.m_uiLength);
            pConnect->m_uiConnectionSignatureSize = signature.m_uiLength;
            if ((GetSupportedFunctionsFlags() & FUNCTION_CONNECTION_SIGNATURE_NEGOTIATION) == 0)
            {
                memcpy(m_negotiationSignature.m_bytes, signature.m_bytes, signature.m_uiLength);
                m_negotiationSignature.m_uiLength = signature.m_uiLength;
            }
            else
            {
                memcpy(m_signature.m_bytes, signature.m_bytes, signature.m_uiLength);
                m_signature.m_uiLength = signature.m_uiLength;
            }
            pConnect->m_pTimeout->SetExpirationTime(Time(m_pConnectPacket->m_pTimeout->GetExpirationTime()));
            SendPacket(pConnect);
            pConnect->ReleaseRef();
            CancelPacketTimeout(m_pConnectPacket);
            m_bConnectSignatureSent = true;
        }
    }
    else if (uiType == PACKET_TYPE_CONNECT)
    {
        if (IsConnecting() && (pPacket->m_uiTypeAndFlags & PACKET_FLAG_ACK) != 0)
        {
            if (m_ioRequest.m_pResponse != 0)
            {
                *m_ioRequest.m_pResponse = *pPacket->m_pPayload;
                m_ioRequest.m_pResponse = 0;
            }
            PacketAcknowledged(Time(pPacket->m_unknownF8), pPacket->m_sequenceID, 0);
            SetConnectionState(Connected);
            StartPing();
        }
        else
        {
            if ((pPacket->m_uiTypeAndFlags & PACKET_FLAG_ACK) == 0)
                SetPeerConnected();
            StartPing();
        }
    }
    else if (uiType == PACKET_TYPE_DATA)
    {
        if (!IsConnecting())
        {
            uiFlags = pPacket->m_uiTypeAndFlags;
            if ((uiFlags & PACKET_FLAG_ACK) != 0)
            {
                PacketAcknowledged(Time(pPacket->m_unknownF8), pPacket->m_sequenceID, pPacket->m_uiSubStreamID);
            }
            else if ((uiFlags & PACKET_FLAG_AGGREGATE_ACK) != 0)
            {
                PacketAggregateAcknoledged(pPacket);
            }
            else
            {
                bDispatch = true;
                if ((uiFlags & PACKET_FLAG_RELIABLE) == 0)
                {
                    short iDifference;
                    if ((uiFlags & PACKET_FLAG_ALTERNATE_KEY_A) == 0 && (uiFlags & PACKET_FLAG_ALTERNATE_KEY_B) == 0)
                    {
                        iDifference = pPacket->m_sequenceID - m_expectedUnreliableSequence;
                    }
                    else
                    {
                        if (m_uiBroadcastSessionID == 0)
                        {
                            m_uiBroadcastSessionID = pPacket->m_uiSessionID;
                            m_expectedBroadcastSequence = pPacket->m_sequenceID;
                        }
                        if (m_uiBroadcastSessionID != pPacket->m_uiSessionID)
                            m_uiBroadcastSessionID = pPacket->m_uiSessionID;
                        iDifference = pPacket->m_sequenceID - m_expectedBroadcastSequence;
                    }
                    if (iDifference < 0)
                    {
                        ++m_endPointInfo.m_uiLatePackets;
                        if (s_bDiscardLateUnreliablePackets)
                            bDispatch = false;
                    }
                    else
                    {
                        m_endPointInfo.m_uiLostPackets += iDifference;
                        if ((uiFlags & PACKET_FLAG_ALTERNATE_KEY_A) == 0
                            && (uiFlags & PACKET_FLAG_ALTERNATE_KEY_B) == 0)
                            m_expectedUnreliableSequence = static_cast<unsigned short>(pPacket->m_sequenceID.GetValue()
                                                                                       + 1);
                        else
                            m_expectedBroadcastSequence = static_cast<unsigned short>(pPacket->m_sequenceID.GetValue()
                                                                                      + 1);
                    }
                }
            }
        }
    }
    else if (uiType == PACKET_TYPE_DISCONNECT)
    {
        if ((pPacket->m_uiTypeAndFlags & PACKET_FLAG_ACK) == 0)
        {
            SendAggregateACK();
            bool bPeerConnected = PeerIsConnected();
            SetPeerDisconnected();
            if (bPeerConnected && m_pEventHandler != 0)
                m_pEventHandler->PeerDisconnected(this);
            if (m_pStream != 0
                && (m_pStream->GetType() == Stream::RendezVousSecure || m_pStream->GetType() == Stream::RendezVous)
                && IsConnected() && (pPacket->m_uiTypeAndFlags & PACKET_FLAG_NEED_ACK) == 0)
            {
                SetConnectionState(Faulty);
                StopPing();
            }
            static_cast<PRUDPStream*>(m_pStream)->ReleaseEndPoint(this);
        }
        else
        {
            if (IsDisconnecting())
                SetConnectionState(NotConnected);
            PacketAcknowledged(Time(pPacket->m_unknownF8), pPacket->m_sequenceID, 0);
        }
    }
    else if (uiType == PACKET_TYPE_PING)
    {
        if ((pPacket->m_uiTypeAndFlags & PACKET_FLAG_ACK) == 0 || m_pPingPacket == 0)
        {
            ++m_endPointInfo.m_uiPingsReceived;
            m_endPointInfo.m_uiLastPingReceived = pPacket->m_sequenceID.GetValue();
        }
        else if (m_pPingPacket->m_sequenceID == pPacket->m_sequenceID)
        {
            AdjustRTT(pPacket, m_pPingPacket);
        }
    }
    if ((pPacket->m_uiTypeAndFlags & PACKET_FLAG_RELIABLE) == 0)
    {
        uiFlags = pPacket->m_uiTypeAndFlags;
        ++m_endPointInfo.m_uiPacketsReceived;
        m_endPointInfo.m_uiBytesReceived += GetPayloadSize(pPacket->m_pPayload);
        if ((uiFlags & PACKET_FLAG_ALTERNATE_KEY_A) != 0)
        {
            ++m_endPointInfo.m_uiBroadcastPacketsReceived;
            m_endPointInfo.m_uiBroadcastBytesReceived += GetPayloadSize(pPacket->m_pPayload);
        }
        else if ((uiFlags & PACKET_FLAG_ALTERNATE_KEY_B) != 0)
        {
            ++m_endPointInfo.m_uiMulticastPacketsReceived;
            m_endPointInfo.m_uiMulticastBytesReceived += GetPayloadSize(pPacket->m_pPayload);
        }
        if (bDispatch)
        {
            ++m_endPointInfo.m_uiDispatchedPackets;
            m_endPointInfo.m_uiDispatchedBytes += GetPayloadSize(pPacket->m_pPayload);
            Dispatch(pPacket, Time(pPacket->m_unknownF8));
        }
    }
    PacketIn* pNext = m_dispatchQueues[pPacket->m_uiSubStreamID]->GetNextToDispatch();
    while (pNext != 0)
    {
        if ((pNext->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_DATA)
            Dispatch(pNext, Time(pPacket->m_unknownF8));
        m_dispatchQueues[pPacket->m_uiSubStreamID]->Dispatched(pNext);
        pNext = m_dispatchQueues[pPacket->m_uiSubStreamID]->GetNextToDispatch();
    }
}

}
}

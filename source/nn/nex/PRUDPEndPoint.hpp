#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PRUDPENDPOINT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PRUDPENDPOINT_HPP

#include <functional>
#include <vector>

#include "nn/nex/EndPoint.hpp"
#include "nn/nex/EndPointInfo.hpp"
#include "nn/nex/IORequestContext.hpp"
#include "nn/nex/LogicalClockTmpl.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/PRUDPMessageInterface.hpp"
#include "nn/nex/Packet.hpp"
#include "nn/nex/PacketEncDec.hpp"
#include "nn/nex/ProfilingUnit.hpp"
#include "nn/nex/RTT.hpp"
#include "nn/nex/SignatureBytes.hpp"
#include "nn/nex/Stream.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/UserContext.hpp"
#include "nn/nex/qMap.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class Key;
class PRUDPMessageSelector;
class PRUDPStream;
class PacketDispatchQueue;
class PacketIn;
class PacketOut;
class SlidingWindow;
class StreamSettings;

class PRUDPEndPoint : public EndPoint
{
public:
    typedef LogicalClockTmpl<unsigned short, short> SequenceClock;
    typedef unsigned int (*CalcRetransmissionTimeoutCallback)(unsigned int, unsigned short);

    PRUDPEndPoint(ConnectionOrientedStream* pStream, const StationURL* pStationURL, SequenceClock* pBroadcastSequence,
                  unsigned char uiVersion);
    virtual ~PRUDPEndPoint();

    virtual void ResetEndPointInfo();
    virtual bool GetEndPointInfo(EndPointInfo* pEndPointInfo) const;
    virtual unsigned int GetReliableBufferPacketCount() const;
    virtual void GetReliableBufferPacketCounts(qVector<unsigned int>& rCounts) const;
    virtual unsigned char GetMaxSubStreamID() const;
    virtual bool IsNotConnected();
    virtual bool IsConnecting();
    virtual bool IsDisconnecting();
    virtual bool IsFaulty();
    virtual bool IsConnected();
    virtual bool PeerIsConnected();
    virtual bool PeerIsDisconnected();
    virtual void SetKeepAliveTimeout(unsigned int uiTimeout);
    virtual void SetMaxSilenceTime(unsigned int uiSilenceTime);
    virtual unsigned int GetKeepAliveTimeout();
    virtual unsigned int GetMaxSilenceTime();
    virtual void SetPeerConnected();
    virtual void SetPeerDisconnected();
    virtual _ConnectionState GetConnectionState() const;
    virtual bool SetConnectionState(_ConnectionState state);
    virtual bool IsAnyPacketProcessed();
    virtual unsigned int GetSignaledFaultError() const;
    virtual unsigned int GetRTT() const;
    virtual unsigned int GetRTTAverage() const;
    virtual qResult _Connect(Buffer* pRequestData, Buffer* pResponseData, ConnectionCallback pCallback,
                             const UserContext& rUserContext, unsigned int uiTimeout);
    virtual qResult _Disconnect(ConnectionCallback pCallback, const UserContext& rUserContext, unsigned int uiTimeout);
    virtual qResult _Send(Buffer* pBuffer, unsigned int uiFlags, bool bCopyPayload, unsigned char uiSubStreamID,
                          unsigned int uiKey);
    virtual unsigned long long GetSupportedFunctionsFlags() const;
    virtual void SignalFaultEvent(unsigned int uiError, bool bProcessNow);
    virtual void SetEncryptionKey(const Key& rKey);
    virtual Key& GetEncryptionKey();

    void ServiceIncomingPacket(PacketIn* pPacket);
    void ServiceTimeout(PacketOut* pPacket);
    qResult SendPacket(PacketOut* pPacket);
    bool IsDuplicateReorderingPacket(PacketIn* pPacket);
    int GetMaxPacketSize();
    void ProcessFaultEvent(unsigned int uiError);
    bool CompleteIO(qResult result);
    void StartPing();
    void StopPing();
    void TimeToPing();
    bool IsSendAggregateACK();
    void SendAggregateACK();
    void SetSupportedFunctionsFlags();

    static bool IsNegotiationConnectionSignatureVersion(const Packet* pPacket);

    static void SetCalcRetransmissionTimeoutCallback(CalcRetransmissionTimeoutCallback pCallback);

    static void SetHybridPartialFaultProcessingMode(bool bEnabled) { s_bHybridPartialFaultProcessingMode = bEnabled; }

private:
    friend class PRUDPStream;

    typedef std::vector<PacketDispatchQueue*, MemAllocator<PacketDispatchQueue*> > DispatchQueueVector;
    typedef std::vector<SlidingWindow*, MemAllocator<SlidingWindow*> > SlidingWindowVector;
    typedef std::vector<Buffer*, MemAllocator<Buffer*> > BufferVector;
    typedef std::vector<unsigned char, MemAllocator<unsigned char> > FragmentIDVector;
    typedef std::vector<bool, MemAllocator<bool> > BitVector;

    qResult Frag(Buffer* pBuffer, unsigned int uiBufferSize, unsigned int uiFragmentSize, PacketType type,
                 unsigned short uiFlags, bool bNotify, unsigned int uiKey, unsigned char uiSubStreamID);
    bool Defrag(PacketIn* pPacket);
    void Dispatch(PacketIn* pPacket, Time receiveTime);
    void HandlerDispatch(Buffer* pBuffer, PacketIn* pPacket);
    qResult HandleSlidingWindowPushError();
    void SendReliable(PacketOut* pPacket);
    void SendNextReliable(unsigned char uiSubStreamID);
    void SendACK(PacketIn* pPacket);
    void SendAggregateACK(Buffer* pBuffer, unsigned short uiSequence, unsigned char uiSubStreamID);
    void SendAggregateACKNew();
    void SendAggregateACKOld();
    void SendResetDisconnectPacket();
    void PacketAcknowledged(const Time& rTime, const SequenceClock& rSequence, unsigned char uiSubStreamID);
    void PacketAggregateAcknoledged(Packet* pPacket);
    void PacketAggregateAcknoledgedNew(Packet* pPacket);
    void PacketAggregateAcknoledgedOld(Packet* pPacket);
    void InitUnreliableSeq(PacketOut* pPacket);
    void DecideMaxUserReliableSubStreams(unsigned short uiMaxSubStreams);
    PRUDPMessageInterface::SignatureMethod DecideSignatureMethod(const Packet* pPacket, Stream::Type type, bool bFlag);
    bool CancelPacketTimeout(PacketOut* pPacket);
    unsigned int ComputeRetransmitTimeout(const PacketOut* pPacket);
    float GetTimeoutMultiplier(unsigned short uiRetransmissions, StreamSettings& rSettings) const;
    void AdjustRTT(const Packet* pPacket, const PacketOut* pSentPacket);
    void AdjustRTT(const Time& rTime, const PacketOut* pSentPacket);
    void AnalyzeRttRto(unsigned int uiRTO);

    unsigned short m_uiPort;
    DispatchQueueVector m_dispatchQueues;
    SlidingWindowVector m_slidingWindows;
    PacketOut* m_pPingPacket;
    PacketOut* m_pConnectPacket;
    bool m_bConnectSignatureSent;
    unsigned int m_uiMaxSilenceTime;
    unsigned int m_uiKeepAliveTimeout;
    _ConnectionState m_connectionState;
    bool m_bPeerConnected;
    unsigned int m_uiSignaledFaultError;
    RTT m_rtt;
    Time m_lastSendTime;
    Time m_lastReceiveTime;
    SequenceClock m_unreliableSequence;
    SequenceClock m_expectedUnreliableSequence;
    SequenceClock* m_pBroadcastSequence;
    SequenceClock m_expectedBroadcastSequence;
    unsigned short m_uiMaxSubStreamID;
    SignatureBytes m_signature;
    SignatureBytes m_negotiationSignature;
    BufferVector m_defragBuffers;
    FragmentIDVector m_defragFragmentIDs;
    unsigned char m_uiPeerSessionID;
    unsigned char m_uiSessionID;
    unsigned char m_uiFaultReason;
    unsigned char m_uiBroadcastSessionID;
    unsigned char m_uiVersion;
    unsigned int m_uiNegotiatedFlags;
    unsigned long long m_uiSupportedFunctionsFlags;
    bool m_bUnknown148;
    BitVector m_pendingAcknowledgements;
    ProfilingUnit m_sendProfile;
    IORequestContext m_ioRequest;
    PRUDPMessageSelector* m_pMessageSelector;
    PacketEncDec m_packetEncDec;
    bool m_bFaultIgnored;
    EndPointInfo m_endPointInfo;
    qMap<unsigned int, unsigned int> m_pendingFragmentCounts;

    static CalcRetransmissionTimeoutCallback s_pCalcRetransmissionTimeoutCallback;
    static bool s_bHybridPartialFaultProcessingMode;
    static bool s_bDiscardLateUnreliablePackets;
    static int s_iPushErrorMode;
};

}
}

#endif

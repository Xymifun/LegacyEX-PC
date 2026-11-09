#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SOCKETTRANSPORT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SOCKETTRANSPORT_HPP

#include "nn/nex/ChainPolicyHistoryPacket.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/InterfaceTable.hpp"
#include "nn/nex/Job.hpp"
#include "nn/nex/PacketQueue.hpp"
#include "nn/nex/ProfilingUnit.hpp"
#include "nn/nex/Relay.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/Router.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/qChain.hpp"
#include "nn/nex/qSortedVector.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class InetAddress;
class QueuingSocket;
class TransportBuffer;
class TransportBufferThreadInterface;

class SocketTransport : public RootTransport
{
public:
    SocketTransport();
    virtual ~SocketTransport();

    virtual qResult Initialize();
    virtual bool StartListen(unsigned short uiPort, unsigned short* pPortOut, bool bPublish, unsigned int uiTrafficType,
                             bool bFlag);
    virtual bool StartListenReuse(unsigned short uiPort, unsigned short* pPortOut, bool bPublish,
                                  unsigned int uiTrafficType, bool bFlag);
    virtual bool StopListen(unsigned short uiPort);
    virtual bool StopListen();
    virtual unsigned int GetNbListeningPorts();
    virtual bool Send(unsigned short uiPort, Stream::Type type, unsigned char uiSourceStreamID,
                      unsigned char uiDestinationStreamID, PacketOut* pPacket, bool bFlag);
    virtual qResult Send(StationURL* pURL, Buffer* pBuffer);
    virtual bool Receive(unsigned short uiPort, Buffer* pBuffer, const InetAddress* pAddress);
    virtual Router* GetRouter();
    virtual Relay* GetRelay();
    virtual unsigned int GetPacketQueueSize() const;
    virtual unsigned int GetEstimatedPacketQueueMemoryUsage() const;
    virtual void ClearPacketInPacketBufferPool();
    virtual void StopTransportThreadImpl();

    bool TerminateTransport();
    void StartEventListener();
    void SetTransportBufferThreadCurrentPriority(unsigned int uiPriority);
    unsigned int GetTransportBufferThreadCurrentPriority() const;
    void GetBufferSetEmptyDropCount(unsigned int& rSendCount, unsigned int& rReceiveCount) const;
    void ResetAnalyzeInfo();

private:
    class TransportJob : public Job
    {
    public:
        explicit TransportJob(SocketTransport* pTransport)
            : Job(JOB_TYPE_0, DebugString(String(L"SocketTransport::TransportJob"))),
              m_pTransport(pTransport)
        {
        }

        virtual void Execute();

    private:
        SocketTransport* m_pTransport;
    };

    typedef qChain<Packet*, ChainPolicyHistoryPacket<Packet*> > HistoryChain;
    typedef qSortedVector<unsigned short, QueuingSocket*> SocketTable;

    bool StartListenImpl(unsigned short uiPort, unsigned short* pPortOut, bool bPublish, unsigned int uiTrafficType,
                         bool bFlag, bool bReuse);
    bool BindSocket(unsigned short uiPort, unsigned short* pPortOut, unsigned int uiTrafficType, bool bFlag,
                    bool bReuse);
    QueuingSocket* GetSocket(unsigned short uiPort);
    bool FillPacketQueueFromBuffer(QueuingSocket* pSocket, Buffer* pBuffer, const InetAddress* pAddress);
    void DeliverIncomingBuffer();
    void DeliverIncoming();
    bool RouteRelaySendMultiMessage(Buffer* pBuffer, unsigned short uiPort);
    void DeliverOutgoingBuffer(QueuingSocket* pSocket, Buffer* pBuffer, const InetAddress& rAddress,
                               unsigned short uiPort);
    void DeliverOutgoingImpl();
    void DeliverOutgoing();
    void TransportJobMethod();

    TransportBufferThreadInterface* m_pBufferThread;
    TransportJob* m_pJob;
    SocketTable m_sockets;
    bool m_bStopRequested;
    InterfaceTable m_interfaceTable;
    PacketQueue m_incomingQueue;
    PacketQueue* m_pOutgoingQueue;
    HistoryChain m_history;
    ProfilingUnit m_profilingUnit;
    unsigned int m_uiContextID;
    Router m_router;
    Relay m_relay;
    Time m_nextHistoryTime;
    TransportBuffer* m_pTransportBuffer;
    unsigned int m_aAnalyzeInfo[2];
};

}
}

#endif

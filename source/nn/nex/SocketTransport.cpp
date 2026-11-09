#include "nn/nex/SocketTransport.hpp"

#include <string.h>

#include "nn/nex/BerkeleySocketDriver.hpp"
#include "nn/nex/Buffer.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/PacketBuffer.hpp"
#include "nn/nex/PacketBufferPacketOut.hpp"
#include "nn/nex/PacketHistory.hpp"
#include "nn/nex/PacketOut.hpp"
#include "nn/nex/ProtectedPacketQueue.hpp"
#include "nn/nex/QueuingSocketTransportBuffer.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/TransportAdapter.hpp"
#include "nn/nex/TransportBuffer.hpp"
#include "nn/nex/TransportBufferMultiThread.hpp"
#include "nn/nex/TransportBufferThread.hpp"
#include "nn/nex/VirtualFilterDevice.hpp"
#include "nn/nex/VirtualNATDevice.hpp"

namespace nn
{
namespace nex
{

namespace
{

const int RESULT_SUCCESS = 0x10001;
const int RESULT_INVALID_ARGUMENT = static_cast<int>(0x8001000Au);
const int RESULT_INVALID_URL = static_cast<int>(0x80050003u);
const int RESULT_NOT_CONNECTED = static_cast<int>(0x80050002u);
const int RESULT_PACKET_ALLOCATION_FAILED = static_cast<int>(0x8005001Fu);

const unsigned int SOCKET_TABLE_RESERVE = 2;
const unsigned int PROFILING_TRACE_MASK = 1000;
const unsigned int HISTORY_RETRANSMIT_INTERVAL = 15;
const unsigned int RELAY_PACKET_OVERHEAD = 44;
const unsigned int ROUTING_PACKET_OVERHEAD = 24;

}

void SocketTransport::TransportJob::Execute()
{
    m_pTransport->TransportJobMethod();
}

SocketTransport::SocketTransport()
    : m_sockets(SOCKET_TABLE_RESERVE),
      m_incomingQueue(L"UDP Incoming Queue Size"),
      m_profilingUnit(L"Transport Job", PROFILING_TRACE_MASK, L"")
{
    Core::AcquireInstance();
    if (g_threadSafetyEnabled)
    {
        CriticalSection* pCriticalSection = &Scheduler::GetInstance()->GetCriticalSection();
        m_pOutgoingQueue = new ProtectedPacketQueue(L"UDP Outgoing Queue Size", pCriticalSection);
    }
    else
    {
        m_pOutgoingQueue = new PacketQueue(L"UDP Outgoing Queue Size");
    }
    m_pJob = 0;
    m_pBufferThread = 0;
    m_bStopRequested = false;
    m_uiContextID = g_usingInstantiationContext ? g_instantiationContextVariable->GetRef() : 0;
    m_pTransportBuffer = 0;
    m_router.SetTransport(this);
    m_relay.SetTransport(this);
    ResetAnalyzeInfo();
}

SocketTransport::~SocketTransport()
{
    TerminateTransport();
    if (m_pVirtualNATDevice != 0)
        DetachVNATDevice(m_pVirtualNATDevice);
    StopListen();
    if (m_pTransportBuffer != 0)
    {
        delete m_pTransportBuffer;
        m_pTransportBuffer = 0;
    }
    m_pOutgoingQueue->Purge();
    delete m_pOutgoingQueue;
    PacketHistoryChainPurge(&m_history);
    m_incomingQueue.Purge();
    Core::ReleaseInstance();
}

qResult SocketTransport::Initialize()
{
    StartEventListener();
    const int code = RESULT_SUCCESS;
    return qResult(code);
}

void SocketTransport::StartEventListener()
{
    if (m_pJob == 0)
    {
        if (Scheduler::GetInstance() != 0)
        {
            m_pJob = new TransportJob(this);
            m_pJob->AddRef();
            Scheduler::GetInstance()->RegisterSpecialDispatchJob(m_pJob, false);
        }
    }
    if (g_transportThreadEnabled)
    {
        if (m_pBufferThread != 0)
        {
            m_pBufferThread->Stop();
            delete m_pBufferThread;
            m_pBufferThread = 0;
            if (m_pTransportBuffer != 0)
            {
                delete m_pTransportBuffer;
                m_pTransportBuffer = 0;
            }
        }
        m_pTransportBuffer = 0;
        if (g_transportBufferPoolEnabled)
            m_pTransportBuffer = new TransportBuffer(g_transportBufferCount, m_uiBufferSize);
        if (g_transportBufferMode == 1)
            m_pBufferThread = new TransportBufferMultiThread();
        else
            m_pBufferThread = new TransportBufferThread();
        m_pBufferThread->Start();
    }
}

bool SocketTransport::TerminateTransport()
{
    m_bStopRequested = true;
    if (g_transportThreadEnabled && m_pBufferThread != 0)
    {
        m_pBufferThread->Stop();
        delete m_pBufferThread;
        m_pBufferThread = 0;
    }
    if (m_pJob != 0)
    {
        if (Scheduler::GetInstance() != 0)
        {
            Scheduler::GetInstance()->UnregisterSpecialDispatchJob(m_pJob);
            m_pJob->ReleaseRef();
            m_pJob = 0;
        }
    }
    return true;
}

bool SocketTransport::BindSocket(unsigned short uiPort, unsigned short* pPortOut, unsigned int uiTrafficType,
                                 bool bFlag, bool bReuse)
{
    QueuingSocket* pSocket;
    bool bOpened;
    if (g_transportThreadEnabled)
    {
        Condition* pCondition = 0;
        if (g_transportBufferMode == 1)
            pCondition = static_cast<TransportBufferMultiThread*>(m_pBufferThread)->GetCondition();
        pSocket = new QueuingSocketTransportBuffer(uiTrafficType, this, g_transportRecvCountPerCycle,
                                                   g_transportSendCountPerCycle, m_uiBufferSize, pCondition,
                                                   m_pTransportBuffer);
        bOpened = pSocket->Open(true);
    }
    else
    {
        pSocket = new QueuingSocket(uiTrafficType, this, m_uiBufferSize, m_pTransportBuffer);
        bOpened = pSocket->Open(true);
    }
    if (!bOpened)
    {
        delete pSocket;
        return false;
    }
    InetAddress address;
    address.SetPortNumber(uiPort);
    pSocket->m_unknownF8 = bFlag;
    if (!pSocket->Bind(&address, bReuse))
    {
        pSocket->Close();
        delete pSocket;
        *pPortOut = 0;
        return false;
    }
    *pPortOut = address.GetPortNumber();
    m_sockets.insert(std::make_pair(*pPortOut, pSocket));
    if (g_transportThreadEnabled && m_pBufferThread != 0 && !m_pBufferThread->InsertSocketPort(*pPortOut, pSocket))
    {
        m_sockets.erase(*pPortOut);
        pSocket->Close();
        delete pSocket;
        *pPortOut = 0;
        return false;
    }
    return true;
}

bool SocketTransport::StartListenImpl(unsigned short uiPort, unsigned short* pPortOut, bool bPublish,
                                      unsigned int uiTrafficType, bool bFlag, bool bReuse)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (uiPort == 0 && !m_sockets.empty() && m_sockets.front().second->m_uiTrafficType == uiTrafficType)
        uiPort = m_sockets.front().first;
    unsigned short uiBoundPort = 0;
    bool bBound = false;
    QueuingSocket* pExistingSocket = GetSocket(uiPort);
    if (pExistingSocket != 0)
    {
        ++pExistingSocket->m_unknown38;
        uiBoundPort = pExistingSocket->GetAddress()->GetPortNumber();
    }
    if (uiBoundPort == 0)
    {
        if (!BindSocket(uiPort, &uiBoundPort, uiTrafficType, bFlag, bReuse))
        {
            RootTransport::GetInstance();
            RootTransport::GetWellKnownPortNumber();
            return false;
        }
        if (bPublish)
        {
            bBound = true;
            qList<StationURL> urls;
            g_transportAdapter->GetLocalURLs(&urls);
            while (!urls.empty())
            {
                urls.front().SetPortNumber(uiBoundPort);
                g_transportAdapter->SetPortNumber(uiBoundPort);
                Network::GetInstance()->AddLocalURL(urls.front());
                urls.erase(urls.begin());
            }
        }
    }
    if (pPortOut != 0)
        *pPortOut = uiBoundPort;
    if (bBound)
        m_uiPortNumber = uiBoundPort;
    return true;
}

bool SocketTransport::StartListen(unsigned short uiPort, unsigned short* pPortOut, bool bPublish,
                                  unsigned int uiTrafficType, bool bFlag)
{
    return StartListenImpl(uiPort, pPortOut, bPublish, uiTrafficType, bFlag, false);
}

bool SocketTransport::StartListenReuse(unsigned short uiPort, unsigned short* pPortOut, bool bPublish,
                                       unsigned int uiTrafficType, bool bFlag)
{
    return StartListenImpl(uiPort, pPortOut, bPublish, uiTrafficType, bFlag, true);
}

bool SocketTransport::StopListen(unsigned short uiPort)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    QueuingSocket* pSocket = GetSocket(uiPort);
    if (pSocket == 0)
        return false;
    if (pSocket->m_unknown38 != 1)
    {
        --pSocket->m_unknown38;
        return true;
    }
    if (m_pBufferThread != 0)
        m_pBufferThread->EraseSocketPort(uiPort);
    m_sockets.erase(uiPort);
    pSocket->Close();
    delete pSocket;
    if (Network::GetInstance() != 0 && g_transportAdapter != 0)
    {
        qList<StationURL> urls;
        g_transportAdapter->GetLocalURLs(&urls);
        while (!urls.empty())
        {
            urls.front().SetPortNumber(uiPort);
            qProtectedList<StationURL>& rLocalURLs = Network::GetInstance()->GetLocalURLs();
            CriticalSection& rLocalLock = rLocalURLs.GetCriticalSection();
            rLocalLock.Enter();
            Network::GetInstance()->RemoveLocalURL(urls.front());
            urls.erase(urls.begin());
            rLocalLock.Leave();
        }
    }
    return true;
}

bool SocketTransport::StopListen()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    while (!m_sockets.empty())
    {
        if (!StopListen(m_sockets.front().first))
            return false;
    }
    return true;
}

unsigned int SocketTransport::GetNbListeningPorts()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    return m_sockets.size();
}

QueuingSocket* SocketTransport::GetSocket(unsigned short uiPort)
{
    if (g_threadSafetyEnabled)
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        SocketTable::iterator socket = m_sockets.find(uiPort);
        if (socket == m_sockets.end())
            return 0;
        return socket->second;
    }
    SocketTable::iterator socket = m_sockets.find(uiPort);
    if (socket == m_sockets.end())
        return 0;
    return socket->second;
}

bool SocketTransport::Send(unsigned short uiPort, Stream::Type type, unsigned char uiSourceStreamID,
                           unsigned char uiDestinationStreamID, PacketOut* pPacket, bool bFlag)
{
    if (m_reserved4)
        return false;
    VirtualPort sourcePort(type, uiSourceStreamID);
    if (m_streamManager.GetStream(sourcePort) == 0)
        return false;
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (GetSocket(uiPort) == 0)
        return false;
    if (m_pVirtualNATDevice != 0)
    {
        StationURL url;
        url.SetInetAddress(&pPacket->m_address);
        bool bHandled = m_pVirtualNATDevice->ProcessOutgoingAddress(&url, uiPort);
        pPacket->m_address = *url.GetInetAddress();
        if (bHandled)
            return true;
    }
    pPacket->m_unknownF8 = g_lastTime;
    pPacket->m_uiPort = uiPort;
    pPacket->m_uiSource = sourcePort.m_uiValue;
    pPacket->m_uiDestination = VirtualPort(type, uiDestinationStreamID).m_uiValue;
    m_pOutgoingQueue->Queue(pPacket, true);
    return true;
}

qResult SocketTransport::Send(StationURL* pURL, Buffer* pBuffer)
{
    if (pURL->GetURLType() != StationURL::Udp)
    {
        const int code = RESULT_INVALID_URL;
        return qResult(code);
    }
    if (!pURL->IsValid())
    {
        const int code = RESULT_INVALID_URL;
        return qResult(code);
    }
    if (m_reserved4)
    {
        const int code = RESULT_NOT_CONNECTED;
        return qResult(code);
    }
    if (pBuffer == 0)
    {
        const int code = RESULT_INVALID_ARGUMENT;
        return qResult(code);
    }
    PacketOut* pPacket;
    if (pBuffer->IsPooled())
    {
        pPacket = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT)
            PacketBufferPacketOut(0, PACKET_TYPE_RAW, 0, pBuffer, 0);
        if (pPacket == 0 || !pPacket->IsValid())
        {
            delete pPacket;
            const int code = RESULT_PACKET_ALLOCATION_FAILED;
            return qResult(code);
        }
    }
    else
    {
        pPacket = new PacketOut(0, PACKET_TYPE_RAW, 0, pBuffer, 0);
    }
    pPacket->m_address = *pURL->GetInetAddress();
    pPacket->m_uiPort = m_uiPortNumber;
    pPacket->m_uiVersion = 0;
    m_pOutgoingQueue->Queue(pPacket, true);
    pPacket->ReleaseRef();
    const int code = RESULT_SUCCESS;
    return qResult(code);
}

bool SocketTransport::Receive(unsigned short uiPort, Buffer* pBuffer, const InetAddress* pAddress)
{
    bool bResult = false;
    QueuingSocket* pSocket = GetSocket(uiPort);
    if (pSocket != 0)
        bResult = FillPacketQueueFromBuffer(pSocket, pBuffer, pAddress);
    return bResult;
}

bool SocketTransport::FillPacketQueueFromBuffer(QueuingSocket* pSocket, Buffer* pBuffer, const InetAddress* pAddress)
{
    if (g_transportAdapter->GetTransportMode() == TransportAdapter::TRANSPORT_MODE_1
        && *pAddress == g_transportAdapter->GetLocalAddress())
        return true;
    bool bFailed = false;
    bool bResult = true;
    int iResult = pSocket->FillPacketQueueFromBuffer(pBuffer, pAddress, &m_incomingQueue, &m_streamManager, &bFailed);
    if (!bFailed && iResult == 0 && pBuffer->GetLength() != 0)
    {
        InetAddress address(*pAddress);
        bResult = !RejectPacket(pBuffer, &address);
    }
    return bResult;
}

void SocketTransport::DeliverIncomingBuffer()
{
    for (SocketTable::iterator socket = m_sockets.begin(); socket != m_sockets.end(); ++socket)
    {
        QueuingSocket* pSocket = socket->second;
        if (!pSocket->m_bConnected)
        {
            if (!m_reserved4)
                Core::GetInstance()->SetTerminateImmediately(true);
            m_reserved4 = true;
        }
        unsigned int uiBufferSize = m_uiBufferSize;
        pSocket->RecvAndQueueIncomingBuffer(uiBufferSize);
        int iResult = 0;
        while (pSocket->IsRecvDataExists() && iResult == 0)
        {
            Buffer* pBuffer = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT) PacketBuffer(uiBufferSize, 4);
            if (pBuffer == 0)
                break;
            if (!pBuffer->IsValid())
            {
                delete pBuffer;
                break;
            }
            InetAddress address;
            iResult = pSocket->RecvNonBlock(pBuffer, &address);
            if (iResult == 0)
            {
                FillPacketQueueFromBuffer(pSocket, pBuffer, &address);
                ++m_aAnalyzeInfo[1];
            }
            pBuffer->ReleaseRef();
        }
    }
}

void SocketTransport::DeliverIncoming()
{
    DeliverIncomingBuffer();
    PacketQueue::Chain::iterator packet = m_incomingQueue.begin();
    while (packet != m_incomingQueue.end())
    {
        (*packet)->m_unknownF8 = g_lastTime;
        m_streamManager.DemuxIncomingPacket(*packet);
        packet = m_incomingQueue.Dequeue(packet);
    }
}

bool SocketTransport::RouteRelaySendMultiMessage(Buffer* pBuffer, unsigned short uiPort)
{
    bool bResult = true;
    InetAddress aPeers[BerkeleySocketDriver::PEER_ADDRESS_CAPACITY];
    unsigned int uiCount = 0;
    BerkeleySocketDriver::GetPeerAddress(aPeers, BerkeleySocketDriver::PEER_ADDRESS_CAPACITY, &uiCount);
    InetAddress address;
    for (unsigned int i = 0; i < uiCount; ++i)
    {
        InetAddress* pPeer = &aPeers[i];
        bool bShouldRoute = m_router.ShouldRoute(pPeer);
        bool bShouldRelay = m_relay.ShouldRelay(pPeer);
        if (!bShouldRelay && !bShouldRoute)
            bResult = false;
        else if (bShouldRelay)
            m_relay.RelayMessage(pBuffer, pPeer, uiPort);
        else
            m_router.RouteMessage(pBuffer, pPeer, uiPort);
    }
    return bResult;
}

void SocketTransport::DeliverOutgoingBuffer(QueuingSocket* pSocket, Buffer* pBuffer, const InetAddress& rAddress,
                                            unsigned short uiPort)
{
    if (TransportAdapter::IsSendMultiAddress(&rAddress))
    {
        if (!RouteRelaySendMultiMessage(pBuffer, uiPort))
        {
            ++m_aAnalyzeInfo[0];
            pSocket->Send(*pBuffer, rAddress);
        }
    }
    else if (m_relay.ShouldRelay(&rAddress))
    {
        m_relay.RelayMessage(pBuffer, &rAddress, uiPort);
    }
    else if (m_router.ShouldRoute(&rAddress))
    {
        m_router.RouteMessage(pBuffer, &rAddress, uiPort);
    }
    else if (m_pInetAddressSet == 0 || !m_pInetAddressSet->Match(rAddress))
    {
        ++m_aAnalyzeInfo[0];
        pSocket->Send(*pBuffer, rAddress);
    }
}

void SocketTransport::DeliverOutgoingImpl()
{
    if (g_historyBundlingEnabled && m_pOutgoingQueue->size() == 0 && m_history.size() != 0)
    {
        unsigned long long ullNow = Time::GetTime();
        if (m_nextHistoryTime <= ullNow)
        {
            QueuingSocket::RetransmitFromHistoryPacketQueue(m_pOutgoingQueue, &m_history, m_history.front()->m_uiPort);
            m_nextHistoryTime = Time(ullNow + HISTORY_RETRANSMIT_INTERVAL);
        }
    }
    while (m_pOutgoingQueue->size() != 0)
    {
        Packet* pPacket = m_pOutgoingQueue->front();
        unsigned short uiPort = pPacket->m_uiPort;
        QueuingSocket* pSocket = GetSocket(uiPort);
        if (pSocket == 0)
        {
            m_pOutgoingQueue->Dequeue(PacketQueue::Chain::iterator(pPacket));
            continue;
        }
        InetAddress address(pPacket->m_address);
        unsigned int uiMaxSize = m_uiBufferSize;
        Buffer* pBuffer;
        if (m_relay.ShouldRelay(&address))
        {
            pBuffer = pSocket->CreateBufferFromPacketQueue(m_pOutgoingQueue, &m_history,
                                                           uiMaxSize - RELAY_PACKET_OVERHEAD);
        }
        else
        {
            if (m_router.ShouldRoute(&address))
                uiMaxSize -= ROUTING_PACKET_OVERHEAD;
            pBuffer = pSocket->CreateBufferFromPacketQueue(m_pOutgoingQueue, &m_history, uiMaxSize);
        }
        if (pBuffer == 0)
            break;
        DeliverOutgoingBuffer(pSocket, pBuffer, address, uiPort);
        pBuffer->ReleaseRef();
    }
    if (m_outputEmulation.IsEnabled())
    {
        for (SocketTable::iterator socket = m_sockets.begin(); socket != m_sockets.end(); ++socket)
            socket->second->DequeueAndSendOutgoingBuffer();
    }
}

void SocketTransport::DeliverOutgoing()
{
    CriticalSection* pLock = m_pOutgoingQueue->GetLock();
    if (pLock != 0)
    {
        pLock->Enter();
        DeliverOutgoingImpl();
        pLock->Leave();
    }
    else
    {
        DeliverOutgoingImpl();
    }
}

void SocketTransport::TransportJobMethod()
{
    if (!m_sockets.empty())
    {
        g_lastTime = Time::GetTime();
        DeliverIncoming();
        m_streamManager.DoWork();
        DeliverOutgoing();
    }
}

void SocketTransport::ClearPacketInPacketBufferPool()
{
    PacketQueue::Chain::iterator outgoing;
    for (outgoing = m_pOutgoingQueue->begin(); outgoing != m_pOutgoingQueue->end();)
    {
        Buffer* pPayload = (*outgoing)->m_pPayload;
        if (pPayload != 0 && pPayload->IsPooled())
            outgoing = m_pOutgoingQueue->Dequeue(outgoing);
        else
            ++outgoing;
    }
    HistoryChain::iterator history;
    for (history = m_history.begin(); history != m_history.end();)
    {
        Buffer* pPayload = (*history)->m_pPayload;
        if (pPayload != 0 && pPayload->IsPooled())
        {
            (*history)->ReleaseRef();
            history = m_history.erase(history);
        }
        else
        {
            ++history;
        }
    }
    PacketQueue::Chain::iterator incoming;
    for (incoming = m_incomingQueue.begin(); incoming != m_incomingQueue.end();)
    {
        Buffer* pPayload = (*incoming)->m_pPayload;
        if (pPayload != 0 && pPayload->IsPooled())
        {
            (*incoming)->ReleaseRef();
            incoming = m_incomingQueue.Dequeue(incoming);
        }
        else
        {
            ++incoming;
        }
    }
}

Router* SocketTransport::GetRouter()
{
    return &m_router;
}

Relay* SocketTransport::GetRelay()
{
    return &m_relay;
}

unsigned int SocketTransport::GetPacketQueueSize() const
{
    return 0;
}

unsigned int SocketTransport::GetEstimatedPacketQueueMemoryUsage() const
{
    return 0;
}

void SocketTransport::StopTransportThreadImpl()
{
    m_bStopRequested = true;
}

void SocketTransport::SetTransportBufferThreadCurrentPriority(unsigned int uiPriority)
{
    if (m_pBufferThread != 0)
        m_pBufferThread->SetPriority(uiPriority);
}

unsigned int SocketTransport::GetTransportBufferThreadCurrentPriority() const
{
    if (m_pBufferThread == 0)
        return 0xFFFFFFFF;
    return m_pBufferThread->GetPriority();
}

void SocketTransport::GetBufferSetEmptyDropCount(unsigned int& rSendCount, unsigned int& rReceiveCount) const
{
    rSendCount = 0;
    rReceiveCount = 0;
    if (g_transportThreadEnabled)
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        for (SocketTable::const_iterator socket = m_sockets.begin(); socket != m_sockets.end(); ++socket)
        {
            const QueuingSocketTransportBuffer* pSocket = static_cast<const QueuingSocketTransportBuffer*>(
                socket->second);
            rSendCount += pSocket->m_sendSet.GetEmptyDropCount();
            rReceiveCount += pSocket->m_recvSet.GetEmptyDropCount();
        }
    }
}

void SocketTransport::ResetAnalyzeInfo()
{
    memset(m_aAnalyzeInfo, 0, sizeof(m_aAnalyzeInfo));
    BerkeleySocketDriver::ResetAnalyzeInfo();
}

}
}

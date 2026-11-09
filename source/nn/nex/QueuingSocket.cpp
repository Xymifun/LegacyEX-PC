#include "nn/nex/QueuingSocket.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/ByteStream.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/PacketBuffer.hpp"
#include "nn/nex/PacketBufferPacketIn.hpp"
#include "nn/nex/PacketHistory.hpp"
#include "nn/nex/PacketIn.hpp"
#include "nn/nex/PacketOut.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/StreamSettings.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/TransportStreamManager.hpp"

namespace nn
{
namespace nex
{

namespace
{

unsigned int s_uiBuffersWithoutHistory = 0;
unsigned int s_uiBundlesEmpty = 0;
unsigned int s_uiBundlesNonEmpty = 0;
unsigned int s_uiBundlesHalfFull = 0;
unsigned int s_uiBundlesFull = 0;
unsigned int s_uiBuffersCreated = 0;

}

QueuingSocket::QueuingSocket(unsigned int uiTrafficType, RootTransport* pTransport, unsigned int uiMTU,
                             TransportBuffer* pTransportBuffer)
    : Socket(uiTrafficType, uiMTU),
      m_unknown38(1),
      m_outputEmulator(pTransport->GetOutputEmulationDevice()),
      m_inputEmulator(pTransport->GetInputEmulationDevice()),
      m_unknownD0(1),
      m_sendingQueue(L"Sending Packet Queue")
{
    m_pTransportBuffer = pTransportBuffer;
    m_unknownF8 = false;
    m_pMessageSelector = &Network::GetInstance()->GetMessageSelector();
}

QueuingSocket::~QueuingSocket()
{
    Close();
}

bool QueuingSocket::Bind(InetAddress* pAddress, bool bReuse)
{
    return Socket::Bind(pAddress, bReuse);
}

int QueuingSocket::OutputQueue(const Buffer& rBuffer, const InetAddress& rAddress)
{
    Buffer* pCopy = new Buffer(rBuffer);
    m_outputEmulator.Queue(pCopy, rAddress);
    pCopy->ReleaseRef();
    return 0;
}

int QueuingSocket::Send(const Buffer& rBuffer, const InetAddress& rAddress)
{
    if (!m_outputEmulator.GetDevice()->IsEnabled())
        return SendBuffer(rBuffer, rAddress);

    return OutputQueue(rBuffer, rAddress);
}

int QueuingSocket::Recv(Buffer* pBuffer, InetAddress* pAddress)
{
    return -1;
}

int QueuingSocket::RecvNonBlock(Buffer* pBuffer, InetAddress* pAddress)
{
    if (!m_inputEmulator.GetDevice()->IsEnabled())
        return RecvBuffer(pBuffer, pAddress);

    Buffer* pHead;
    if (!m_inputEmulator.GetHead(&pHead, pAddress))
        return -1;

    *pBuffer = *pHead;
    m_inputEmulator.Dequeue(pHead);
    return 0;
}

int QueuingSocket::SendBuffer(const Buffer& rBuffer, const InetAddress& rAddress)
{
    if (rBuffer.GetLength() == 0)
        return 0;

    int iResult = Socket::Send(rBuffer, rAddress);
    if (!m_bConnected)
        m_uiState = STATE_CLOSED;

    return iResult;
}

int QueuingSocket::RecvBuffer(Buffer* pBuffer, InetAddress* pAddress)
{
    return Socket::RecvNonBlock(pBuffer, pAddress);
}

void QueuingSocket::RecvAndQueueIncomingBuffer(unsigned int uiBufferSize)
{
    if (m_inputEmulator.GetDevice()->IsEnabled())
    {
        int iResult = 0;
        while (IsRecvDataExists() && iResult == 0)
        {
            Buffer* pBuffer = new Buffer(uiBufferSize, 0);
            InetAddress address;
            iResult = RecvBuffer(pBuffer, &address);
            if (iResult == 0)
                m_inputEmulator.Queue(pBuffer, address);

            pBuffer->ReleaseRef();
        }
    }
}

void QueuingSocket::DequeueAndSendOutgoingBuffer()
{
    if (m_outputEmulator.GetDevice()->IsEnabled())
    {
        for (;;)
        {
            InetAddress address;
            Buffer* pBuffer;
            if (!m_outputEmulator.GetHead(&pBuffer, &address))
                break;

            SendBuffer(*pBuffer, address);
            m_outputEmulator.Dequeue(pBuffer);
        }
    }
}

int QueuingSocket::FillPacketQueueFromBuffer(Buffer* pBuffer, const InetAddress* pAddress, PacketQueue* pQueue,
                                             TransportStreamManager* pStreamManager, bool* pFailed)
{
    if (m_bVDP)
    {
        if (pBuffer->GetLength() <= 2)
            return 0;
        unsigned int uiRemaining = pBuffer->GetLength() - 2;
        pBuffer->AppendData(pBuffer->GetData() + 2, uiRemaining, 0);
        pBuffer->SetLength(uiRemaining);
    }
    int iCount = 0;
    do
    {
        Packet* pPacket = ExtractIncomingPacket(pStreamManager, pAddress, pBuffer, pFailed);
        if (pPacket == 0)
            return iCount;
        ++iCount;
        pPacket->m_uiPort = GetAddress()->GetPortNumber();
        pQueue->Queue(pPacket, true);
    } while (pBuffer->GetLength() != 0);
    return iCount;
}

Packet* QueuingSocket::ExtractIncomingPacket(TransportStreamManager* pStreamManager, const InetAddress* pAddress,
                                             Buffer* pBuffer, bool* pFailed)
{
    PacketIn* pPacket;
    if (!m_unknownF8)
    {
        pPacket = new PacketIn();
    }
    else
    {
        pPacket = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT) PacketBufferPacketIn(pBuffer->GetLength());
        if (pPacket == 0)
        {
            *pFailed = true;
            return 0;
        }
        if (pPacket->m_pPayload == 0 || !pPacket->m_pPayload->IsValid())
        {
            delete pPacket;
            *pFailed = true;
            return 0;
        }
    }
    pPacket->m_address = *pAddress;
    ByteStream stream(pBuffer);
    unsigned int uiConsumed;
    if (g_historyBundlingEnabled)
    {
        unsigned int uiBundled = 0;
        PRUDPMessageSelector& rSelector = Network::GetInstance()->GetMessageSelector();
        bool bUnpacked = rSelector.FastUnpack(&stream, pPacket, &uiConsumed);
        while (bUnpacked && pPacket->CanHistoryBundle()
               && (g_historyBundleWithoutFlag || (pPacket->m_uiTypeAndFlags & PACKET_FLAG_HISTORY_BUNDLE))
               && pStreamManager->IsDuplicateReorderingPacket(pPacket))
        {
            uiBundled += uiConsumed;
            bUnpacked = rSelector.FastUnpack(&stream, pPacket, &uiConsumed);
        }
        if (uiBundled != 0)
        {
            unsigned int uiRemaining = pBuffer->GetLength() - uiBundled;
            if (uiRemaining == 0)
            {
                pBuffer->Clear();
                pPacket->ReleaseRef();
                return 0;
            }
            pBuffer->AppendData(pBuffer->GetData() + uiBundled, uiRemaining, 0);
            pBuffer->SetLength(uiRemaining);
        }
    }
    if (!m_pMessageSelector->Unpack(&stream, pPacket, &uiConsumed))
    {
        pPacket->ReleaseRef();
        return 0;
    }
    pBuffer->SetHeadShiftSize(pBuffer->GetHeadShiftSize() + uiConsumed);
    return pPacket;
}

void QueuingSocket::RetransmitFromHistoryPacketQueue(PacketQueue* pQueue, HistoryChain* pHistory, unsigned short uiPort)
{
    Time now = Time::GetTime();
    HistoryChain::iterator position = pHistory->begin();
    while (position != pHistory->end())
    {
        PacketOut* pPacket = static_cast<PacketOut*>(*position);
        if (pPacket->m_unknown129)
        {
            position = PacketHistoryChainDequeue(pHistory, position);
        }
        else if (pPacket->m_uiPort == uiPort)
        {
            if (pPacket->m_unknown130 == 0)
            {
                if (now < pPacket->m_unknown138)
                {
                    ++position;
                }
                else
                {
                    pPacket->m_unknown130 = 2;
                    pPacket->m_unknown128 = true;
                    ++pPacket->m_unknown11a;
                    pPacket->AddRef();
                    position = PacketHistoryChainDequeue(pHistory, position);
                    pQueue->QueueFront(pPacket, false);
                    pPacket->ReleaseRef();
                }
            }
            else if (pPacket->m_unknown130 < 3)
            {
                if (pPacket->m_unknown138 <= now)
                    pPacket->m_unknown130 = 3;
                ++position;
            }
            else if (pPacket->m_unknown130 == 3)
            {
                position = pHistory->end();
            }
        }
        else
        {
            ++position;
        }
    }
}

Buffer* QueuingSocket::CreateBufferFromPacketQueue(PacketQueue* pQueue, HistoryChain* pHistory, unsigned int uiMaxSize)
{
    PacketOut* pPacket = static_cast<PacketOut*>(pQueue->front());
    PacketOut* pHistoryPacket = 0;
    unsigned int uiBundled = 0;
    if (pPacket == 0 || !pPacket->ReadyToSend())
        return 0;
    StreamBundling& rBundling = g_streamSettings[pPacket->m_uiSource >> 4].Get().GetBundling();
    unsigned int uiReliableLimit = rBundling.GetReliableBundleLimit();
    unsigned int uiPlainLimit = rBundling.GetPlainBundleLimit();
    unsigned int uiMaxCount = uiReliableLimit < uiPlainLimit ? uiPlainLimit : uiReliableLimit;
    unsigned char uiTTL = pPacket->m_uiTTL;
    Buffer* pScratchBuffer = 0;
    Buffer* pOutputBuffer = 0;
    if (((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_PING || pPacket->m_pPayload == 0
         || pPacket->m_pPayload->IsPooled())
        && m_unknownF8)
    {
        pScratchBuffer = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT)
            PacketBuffer(uiMaxSize, Buffer::BUFFER_FLAG_POOLED);
        pOutputBuffer = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT)
            PacketBuffer(uiMaxSize, Buffer::BUFFER_FLAG_POOLED);
        if (pScratchBuffer == 0 || !pScratchBuffer->IsValid() || pOutputBuffer == 0 || !pOutputBuffer->IsValid())
        {
            delete pScratchBuffer;
            delete pOutputBuffer;
            pScratchBuffer = new Buffer(uiMaxSize, 0);
            pOutputBuffer = new Buffer(uiMaxSize, 0);
        }
    }
    else
    {
        pScratchBuffer = new Buffer(uiMaxSize, 0);
        pOutputBuffer = new Buffer(uiMaxSize, 0);
    }
    ByteStream scratch(pScratchBuffer);
    pScratchBuffer->ReleaseRef();
    ByteStream output(pOutputBuffer);
    pOutputBuffer->ReleaseRef();
    if (m_bVDP)
    {
        unsigned short uiPrefix = 0;
        output.Append(reinterpret_cast<const unsigned char*>(&uiPrefix), 2, 1);
    }
    ByteStream* pFinal = &output;
    ByteStream* pTrailing = 0;
    PRUDPMessageSelector& rSelector = Network::GetInstance()->GetMessageSelector();
    PacketQueue::Chain::iterator position = pQueue->begin();
    while (position != pQueue->end())
    {
        pPacket = static_cast<PacketOut*>(*position);
        scratch.Clear();
        rSelector.Pack(pPacket, &scratch);
        unsigned int uiTrailingSize = pTrailing != 0 ? pTrailing->GetBuffer()->GetLength() : 0;
        if (uiMaxSize < output.GetBuffer()->GetLength() + scratch.GetBuffer()->GetLength() + uiTrailingSize)
            break;
        if (!m_bVDP || (pPacket->m_uiTypeAndFlags & PACKET_FLAG_TRAILER) == 0)
        {
            output << *scratch.GetBuffer();
        }
        else
        {
            if (pTrailing == 0)
                pTrailing = new ByteStream();
            *pTrailing << *scratch.GetBuffer();
        }
        pPacket->AddRef();
        PacketQueue::Chain::iterator next = pQueue->Dequeue(position);
        if (pPacket->m_unknown12c != 0 && g_historyBundlingEnabled && pPacket->CanHistoryBundle()
            && !pPacket->m_unknown129)
        {
            m_sendingQueue.Queue(pPacket, false);
            pHistoryPacket = pPacket;
        }
        ++uiBundled;
        if (uiMaxCount < uiBundled)
        {
            pPacket->ReleaseRef();
            break;
        }
        while (next != pQueue->end())
        {
            PacketOut* pNextPacket = static_cast<PacketOut*>(*next);
            if (pPacket->IsBundlable() && pNextPacket->IsBundlableWithPortTypeCheck(pPacket))
            {
                unsigned int uiLimit = uiReliableLimit;
                if ((pNextPacket->m_uiTypeAndFlags & PACKET_FLAG_RELIABLE) == 0
                    && (pNextPacket->m_uiTypeAndFlags & PACKET_FLAG_ACK) == 0)
                    uiLimit = uiPlainLimit;
                if (uiBundled <= uiLimit)
                    break;
            }
            ++next;
        }
        pPacket->ReleaseRef();
        position = next;
    }
    if (!g_historyBundlingEnabled || pHistoryPacket == 0)
    {
        ++s_uiBuffersWithoutHistory;
    }
    else
    {
        unsigned int uiTrailingSize = pTrailing != 0 ? pTrailing->GetBuffer()->GetLength() : 0;
        unsigned int uiLimit = g_bundleBufferSize < uiMaxSize ? g_bundleBufferSize : uiMaxSize;
        int iRemaining = uiLimit - (output.GetBuffer()->GetLength() + uiTrailingSize);
        unsigned int uiHistoryBundled = 0;
        if (iRemaining > 0)
        {
            scratch.Clear();
            unsigned int uiPrefixSize = 0;
            if (m_bVDP)
            {
                unsigned short uiPrefix = 0;
                scratch.Append(reinterpret_cast<const unsigned char*>(&uiPrefix), 2, 1);
                uiPrefixSize = 2;
            }
            Time now(Time::GetTime());
            Time limit;
            if (static_cast<unsigned long long>(g_historyBundleAge) < static_cast<unsigned long long>(now))
                limit = now + -g_historyBundleAge;
            uiHistoryBundled = CreateBunldingStreamFromHistoryPacketQueue(pHistory, pHistoryPacket, limit, iRemaining,
                                                                          &scratch);
            if (uiPrefixSize < scratch.GetBuffer()->GetLength())
            {
                scratch.GetBuffer()->AppendData(output.GetBuffer()->GetData() + uiPrefixSize,
                                                output.GetBuffer()->GetLength() - uiPrefixSize, 0xFFFFFFFFu);
                pFinal = &scratch;
            }
        }
        InsertToHistoryPacketQueue(pHistory, &m_sendingQueue);
        if (pHistoryPacket->m_unknown130 != 2)
        {
            PacketOut* pFront = static_cast<PacketOut*>(pQueue->front());
            if (pFront == 0 || !pFront->ReadyToSend())
                RetransmitFromHistoryPacketQueue(pQueue, pHistory, pHistoryPacket->m_uiPort);
        }
        m_sendingQueue.Purge();
        if (uiHistoryBundled == 0)
            ++s_uiBundlesEmpty;
        else
            ++s_uiBundlesNonEmpty;
        if (g_historyPacketLimit / 2 <= uiHistoryBundled)
            ++s_uiBundlesHalfFull;
        if (g_historyPacketLimit <= uiHistoryBundled)
            ++s_uiBundlesFull;
    }
    if (m_bVDP)
    {
        unsigned short uiLength = static_cast<unsigned short>(pFinal->GetBuffer()->GetLength() - 2);
        pFinal->GetBuffer()->AppendData(&uiLength, 2, 0);
    }
    if (pTrailing != 0)
    {
        *pFinal << *pTrailing->GetBuffer();
        delete pTrailing;
    }
    Buffer* pResult = pFinal->GetBuffer();
    if (pResult == 0)
        return 0;
    ++s_uiBuffersCreated;
    pResult->SetTTL(uiTTL);
    pResult->AddRef();
    return pResult;
}

}
}

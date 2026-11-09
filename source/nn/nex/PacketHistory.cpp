#include "nn/nex/PacketHistory.hpp"

#include "nn/nex/ByteStream.hpp"
#include "nn/nex/ChainPolicyTemp.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/Packet.hpp"
#include "nn/nex/PacketOut.hpp"
#include "nn/nex/PacketQueue.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

namespace
{

typedef qChain<Packet*, ChainPolicyTemp<Packet*> > PacketBundleChain;

}

PacketHistoryChain::iterator PacketHistoryChainDequeue(PacketHistoryChain* pChain,
                                                       PacketHistoryChain::iterator position)
{
    Packet* pPacket = *position;
    position = pChain->erase(position);
    pPacket->ReleaseRef();
    return position;
}

void PacketHistoryChainPurge(PacketHistoryChain* pChain)
{
    PacketHistoryChain::iterator position = pChain->begin();
    while (position != pChain->end())
        position = PacketHistoryChainDequeue(pChain, position);
}

void PacketHistoryChainQueueFront(PacketHistoryChain* pChain, PacketOut* pPacket)
{
    pPacket->AddRef();
    if (pPacket->m_historyLink.m_pPrevious != 0 || pPacket->m_historyLink.m_pNext != 0
        || pChain->begin() == PacketHistoryChain::iterator(pPacket))
        PacketHistoryChainDequeue(pChain, PacketHistoryChain::iterator(pPacket));
    pChain->push_front(pPacket);
    pPacket->m_historyTime = Time::GetTime();
}

void InsertToHistoryPacketQueue(PacketHistoryChain* pChain, PacketQueue* pQueue)
{
    PacketQueue::Chain::iterator position = pQueue->begin();
    while (position != pQueue->end())
    {
        PacketOut* pPacket = static_cast<PacketOut*>(*position);
        if (!g_historyBundlingEnabled || !pPacket->CanHistoryBundle()
            || (!g_historyBundleWithoutFlag && !(pPacket->m_uiTypeAndFlags & PACKET_FLAG_HISTORY_BUNDLE))
            || pPacket->m_unknown129)
        {
            position = pQueue->Dequeue(position);
            continue;
        }

        pPacket->AddRef();
        position = pQueue->Dequeue(position);
        if (pPacket->m_unknown130 != 2)
            pPacket->m_unknown130 = 0;
        pPacket->m_unknown138 = Time(Time::GetTime()) + g_historyBundleDelay;
        PacketHistoryChainQueueFront(pChain, pPacket);
        if (pChain->size() > g_historyPacketLimit)
            PacketHistoryChainDequeue(pChain, PacketHistoryChain::iterator(pChain->back()));
        pPacket->ReleaseRef();
    }
}

unsigned int CreateBunldingStreamFromHistoryPacketQueue(PacketHistoryChain* pChain, PacketOut* pPacket, Time rLimit,
                                                        unsigned int uiMaxSize, ByteStream* pStream)
{
    if (pChain->size() == 0)
        return 0;
    const unsigned int uiStartSize = pStream->GetBuffer()->GetLength();
    unsigned int uiSize = uiStartSize;
    PacketBundleChain bundle;
    PacketHistoryChain::iterator position = pChain->begin();
    while (position != pChain->end())
    {
        PacketOut* pHistoryPacket = static_cast<PacketOut*>(*position);
        if (bundle.size() >= g_bundlePacketLimit)
            break;
        if (pHistoryPacket->m_unknown11a >= g_bundleSendLimit)
        {
            position = PacketHistoryChainDequeue(pChain, position);
            continue;
        }
        if (pHistoryPacket->m_unknown129)
        {
            position = PacketHistoryChainDequeue(pChain, position);
            continue;
        }
        if (pHistoryPacket->m_historyTime < rLimit)
            break;
        if (!g_historyBundlingEnabled || !pPacket->CanHistoryBundle() || pPacket->m_unknown129
            || !pHistoryPacket->IsBundlableWithPortTypeCheck(pPacket))
        {
            ++position;
            continue;
        }
        if (uiMaxSize < pHistoryPacket->m_unknown12c + uiSize)
            break;
        bundle.push_front(pHistoryPacket);
        ++pHistoryPacket->m_unknown11a;
        uiSize += pHistoryPacket->m_unknown12c;
        if (pHistoryPacket->m_unknown11a < g_bundleSendLimit)
        {
            ++position;
        }
        else
        {
            pHistoryPacket->AddRef();
            position = PacketHistoryChainDequeue(pChain, position);
        }
    }
    while (position != pChain->end())
    {
        if ((*position)->m_historyTime < rLimit)
            position = PacketHistoryChainDequeue(pChain, position);
        else
            ++position;
    }
    if (uiSize == uiStartSize)
        return 0;
    ByteStream* pPacked = new ByteStream();
    PRUDPMessageSelector& rSelector = Network::GetInstance()->GetMessageSelector();
    PacketBundleChain::iterator bundled = bundle.begin();
    while (bundled != bundle.end())
    {
        PacketOut* pBundledPacket = static_cast<PacketOut*>(*bundled);
        pPacked->Clear();
        rSelector.Pack(pBundledPacket, pPacked);
        if (pBundledPacket->m_unknown130 == 0)
            pBundledPacket->m_unknown130 = 1;
        if (pBundledPacket->m_unknown11a < g_bundleSendLimit)
        {
            ++bundled;
        }
        else
        {
            bundled = bundle.erase(bundled);
            pBundledPacket->ReleaseRef();
        }
        *pStream << *pPacked->GetBuffer();
    }
    delete pPacked;
    return bundle.size();
}

}
}

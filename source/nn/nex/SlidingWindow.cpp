#include "nn/nex/SlidingWindow.hpp"

#include "nn/nex/Core.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/PacketEncDec.hpp"
#include "nn/nex/PacketOut.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/Stream.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int WINDOW_TRACE_MASK = 0x20000000;

}

SlidingWindow::SlidingWindow(unsigned short uiWindowSize, unsigned int uiMaxBufferedPackets)
    : m_packets(),
      m_nextSequence(),
      m_nextToSend(),
      m_oldestUnacknowledged(),
      m_criticalSection(WINDOW_TRACE_MASK)
{
    m_uiWindowSize = uiWindowSize;
    m_uiMaxBufferedPackets = uiMaxBufferedPackets;
    m_uiDataPending = 0;
    m_nextSequence = 1;
    m_nextToSend = m_nextSequence;
    m_oldestUnacknowledged = m_nextSequence;
}

SlidingWindow::~SlidingWindow()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    Purge();
}

bool SlidingWindow::Push(PacketOut* pPacket, PacketEncDec* pEncDec, Stream* pStream)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_packets.find(m_nextSequence) != m_packets.end())
        return false;
    if (GetFreeBufferNum() <= 0)
        return false;
    pPacket->AddRef();
    if ((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_DATA)
        ++m_uiDataPending;
    m_packets[m_nextSequence] = pPacket;
    pPacket->m_sequenceID = m_nextSequence;
    ++m_nextSequence;
    if (pEncDec != 0)
        pEncDec->Encode(pPacket, pStream->GetType(), true);
    return true;
}

PacketOut* SlidingWindow::GetNextToSend()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    PacketOut* pPacket = 0;
    if (m_nextToSend - m_oldestUnacknowledged < m_uiWindowSize)
    {
        PacketMap::iterator found = m_packets.find(m_nextToSend);
        if (found != m_packets.end())
        {
            pPacket = found->second;
            ++m_nextToSend;
        }
    }
    return pPacket;
}

PacketOut* SlidingWindow::GetPacket(Clock clock)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    PacketMap::iterator found = m_packets.find(clock);
    if (found == m_packets.end())
        return 0;
    return found->second;
}

PacketOut* SlidingWindow::GetPacket(PacketMap::iterator position)
{
    if (position == m_packets.end())
        return 0;
    return position->second;
}

void SlidingWindow::Acknowledged(Clock clock, qMap<unsigned int, unsigned int>& rPendingCounts)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    PacketMap::iterator found = m_packets.find(clock);
    if (found != m_packets.end())
    {
        PacketOut* pPacket = found->second;
        unsigned int uiKey = pPacket->m_unknown140;
        if (uiKey != 0)
        {
            qMap<unsigned int, unsigned int>::iterator pending = rPendingCounts.find(uiKey);
            if (pending != rPendingCounts.end())
            {
                --pending->second;
                if (pending->second == 0)
                {
                    ScopedCS networkLock(&Network::GetInstance()->m_unknown3e0);
                    Network::GetInstance()->m_unknown3d0.push_back(uiKey);
                    rPendingCounts.erase(uiKey);
                }
            }
        }
        pPacket->m_unknown129 = true;
        PacketMap::iterator next = found;
        ++next;
        m_packets.erase(found);
        if (clock == m_oldestUnacknowledged)
        {
            if (next != m_packets.end())
                m_oldestUnacknowledged = next->first;
            else
                m_oldestUnacknowledged = m_nextSequence;
        }
        if ((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_DATA)
            --m_uiDataPending;
        pPacket->ReleaseRef();
    }
}

void SlidingWindow::Purge()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    PacketMap::iterator it = m_packets.begin();
    while (it != m_packets.end())
    {
        it->second->ReleaseRef();
        PacketMap::iterator current = it++;
        m_packets.erase(current);
    }
    m_nextToSend = m_nextSequence;
    m_oldestUnacknowledged = m_nextSequence;
    m_uiDataPending = 0;
}

bool SlidingWindow::Empty()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    return m_oldestUnacknowledged == m_nextSequence;
}

bool SlidingWindow::ReadyToSend()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    return m_nextToSend - m_oldestUnacknowledged < m_uiWindowSize && m_nextSequence != m_nextToSend;
}

bool SlidingWindow::DataPending()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    return m_uiDataPending != 0;
}

unsigned int SlidingWindow::NbDataPending()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    return m_uiDataPending;
}

int SlidingWindow::GetFreeBufferNum() const
{
    return m_uiMaxBufferedPackets - m_packets.size();
}

SlidingWindow::PacketMap::iterator SlidingWindow::AcquireIterator()
{
    Scheduler::GetInstance()->GetCriticalSection().Enter();
    return m_packets.begin();
}

void SlidingWindow::ReleaseIterator()
{
    Scheduler::GetInstance()->GetCriticalSection().Leave();
}

void SlidingWindow::Trace(unsigned long long)
{
}

}
}

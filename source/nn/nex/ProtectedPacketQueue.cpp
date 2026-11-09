#include "nn/nex/ProtectedPacketQueue.hpp"

namespace nn
{
namespace nex
{

ProtectedPacketQueue::~ProtectedPacketQueue()
{
    ScopedCS lock(m_pLock);
    PacketQueue::Purge();
}

void ProtectedPacketQueue::Purge()
{
    CriticalSection* pLock = m_pLock;
    ScopedCS lock(pLock);
    PacketQueue::Purge();
}

void ProtectedPacketQueue::Queue(Packet* pPacket, bool bStampTime)
{
    ScopedCS lock(m_pLock);
    PacketQueue::Queue(pPacket, bStampTime);
}

void ProtectedPacketQueue::QueueFront(Packet* pPacket, bool bStampTime)
{
    ScopedCS lock(m_pLock);
    PacketQueue::QueueFront(pPacket, bStampTime);
}

PacketQueue::Chain::iterator ProtectedPacketQueue::Dequeue(Chain::iterator position)
{
    ScopedCS lock(m_pLock);
    return PacketQueue::Dequeue(position);
}

CriticalSection* ProtectedPacketQueue::GetLock()
{
    return m_pLock;
}

}
}

#include "nn/nex/PacketQueue.hpp"

#include "nn/nex/Packet.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

PacketQueue::PacketQueue(const wchar_t* pName)
{
}

PacketQueue::~PacketQueue()
{
    Purge();
}

void PacketQueue::Purge()
{
    Chain::iterator position = m_packets.begin();
    while (position != m_packets.end())
        position = Dequeue(position);
}

void PacketQueue::Queue(Packet* pPacket, bool bStampTime)
{
    if (pPacket->m_link.m_pPrevious == 0 && pPacket->m_link.m_pNext == 0 && m_packets.front() != pPacket)
    {
        pPacket->AddRef();
        m_packets.push_back(pPacket);
        if (bStampTime)
            pPacket->m_queueTime = g_lastTime;
    }
}

void PacketQueue::QueueFront(Packet* pPacket, bool bStampTime)
{
    if (pPacket->m_link.m_pPrevious == 0 && pPacket->m_link.m_pNext == 0 && m_packets.front() != pPacket)
    {
        pPacket->AddRef();
        m_packets.push_front(pPacket);
        if (bStampTime)
            pPacket->m_queueTime = Time::GetTime();
    }
}

PacketQueue::Chain::iterator PacketQueue::Dequeue(Chain::iterator position)
{
    Packet* pPacket = *position;
    position = m_packets.erase(position);
    pPacket->m_queueTime = Time();
    pPacket->ReleaseRef();
    return position;
}

}
}

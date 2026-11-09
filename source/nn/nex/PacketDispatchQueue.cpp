#include "nn/nex/PacketDispatchQueue.hpp"

#include "nn/nex/PacketIn.hpp"

namespace nn
{
namespace nex
{

PacketDispatchQueue::PacketDispatchQueue() : m_packets(), m_nextToDispatch()
{
    m_nextToDispatch = 1;
}

PacketDispatchQueue::~PacketDispatchQueue()
{
    Purge();
}

void PacketDispatchQueue::Queue(PacketIn* pPacket)
{
    if (pPacket->m_sequenceID >= m_nextToDispatch)
    {
        PacketMap::iterator found = m_packets.find(pPacket->m_sequenceID.GetValue());
        if (found == m_packets.end())
        {
            pPacket->AddRef();
            m_packets[pPacket->m_sequenceID.GetValue()] = pPacket;
        }
    }
}

PacketIn* PacketDispatchQueue::GetNextToDispatch()
{
    PacketIn* pPacket = 0;
    PacketMap::iterator found = m_packets.find(m_nextToDispatch);
    if (found != m_packets.end())
    {
        pPacket = found->second;
        ++m_nextToDispatch;
    }
    return pPacket;
}

void PacketDispatchQueue::Dispatched(PacketIn* pPacket)
{
    m_packets.erase(pPacket->m_sequenceID);
    pPacket->ReleaseRef();
}

void PacketDispatchQueue::Purge()
{
    PacketMap::iterator it = m_packets.begin();
    while (it != m_packets.end())
    {
        it->second->ReleaseRef();
        PacketMap::iterator current = it;
        ++it;
        m_packets.erase(current);
    }
}

}
}

#include "nn/nex/TimeoutManager.hpp"

#include <vector>

#include "nn/nex/Core.hpp"
#include "nn/nex/PRUDPEndPoint.hpp"
#include "nn/nex/PacketOut.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/Timeout.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int QUICK_TIMEOUT_RTO = 1;
const unsigned long long IMMEDIATE_EXPIRATION_TIME = 1;

}

TimeoutManager::TimeoutManager() : m_bQuickTimeout(false)
{
}

TimeoutManager::~TimeoutManager()
{
}

void TimeoutManager::ServicePacketTimeouts()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    std::vector<PacketOut*, MemAllocator<PacketOut*> > expiredPackets;
    TimeoutMap::iterator timeout = m_timeouts.begin();
    while (timeout != m_timeouts.end()
           && (timeout->second->m_pTimeout->IsAwaited() || timeout->second->m_pTimeout->IsExpired()))
    {
        PacketOut* pPacket = timeout->second;
        m_timeouts.erase(timeout++);
        m_packets.erase(pPacket);
        expiredPackets.push_back(pPacket);
    }

    for (std::vector<PacketOut*, MemAllocator<PacketOut*> >::iterator packet = expiredPackets.begin();
         packet != expiredPackets.end(); ++packet)
    {
        PacketOut* pPacket = *packet;
        pPacket->m_pEndPoint->ServiceTimeout(pPacket);
        pPacket->ReleaseRef();
    }
}

void TimeoutManager::SetQuickTimeout()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_bQuickTimeout = true;
}

void TimeoutManager::SetPacketQuickTimeout(PacketType type)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    std::vector<PacketOut*, MemAllocator<PacketOut*> > matchingPackets;
    for (TimeoutMap::iterator timeout = m_timeouts.begin(); timeout != m_timeouts.end(); ++timeout)
    {
        PacketOut* pPacket = timeout->second;
        if ((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == type)
            matchingPackets.push_back(pPacket);
    }

    for (std::vector<PacketOut*, MemAllocator<PacketOut*> >::iterator packet = matchingPackets.begin();
         packet != matchingPackets.end(); ++packet)
    {
        PacketOut* pPacket = *packet;
        pPacket->m_pTimeout->SetRTO(QUICK_TIMEOUT_RTO);
        pPacket->m_pTimeout->SetExpirationTime(Time(IMMEDIATE_EXPIRATION_TIME));
        SchedulePacketTimeout(pPacket);
    }
}

void TimeoutManager::ForceSetPacketTimeouts(Time time)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    for (TimeoutMap::iterator timeout = m_timeouts.begin(); timeout != m_timeouts.end(); ++timeout)
    {
        PacketOut* pPacket = timeout->second;
        if (pPacket->m_pTimeout->GetExpirationTime() != 0 && pPacket->m_pTimeout->GetExpirationTime() > time)
            pPacket->m_pTimeout->SetExpirationTime(time);
    }
}

void TimeoutManager::SchedulePacketTimeout(PacketOut* pPacket)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_bQuickTimeout)
    {
        unsigned short uiType = pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK;
        if (uiType == PACKET_TYPE_SYN || uiType == PACKET_TYPE_CONNECT || uiType == PACKET_TYPE_DISCONNECT)
        {
            pPacket->m_pTimeout->SetRTO(QUICK_TIMEOUT_RTO);
            pPacket->m_pTimeout->SetExpirationTime(Time(IMMEDIATE_EXPIRATION_TIME));
        }
    }

    pPacket->m_pTimeout->Start();
    pPacket->AddRef();
    if (m_packets.find(pPacket) != m_packets.end())
    {
        for (TimeoutMap::iterator timeout = m_timeouts.begin(); timeout != m_timeouts.end(); ++timeout)
        {
            if (timeout->second == pPacket)
            {
                m_timeouts.erase(timeout);
                m_packets.erase(pPacket);
                pPacket->ReleaseRef();
                break;
            }
        }
    }

    m_packets.insert(pPacket);
    m_timeouts.insert(std::pair<const Time, PacketOut*>(pPacket->m_pTimeout->GetAwaitedTime(), pPacket));
}

bool TimeoutManager::CancelPacketTimeout(PacketOut* pPacket)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    Time awaitedTime = pPacket->m_pTimeout->GetAwaitedTime();
    std::pair<TimeoutMap::iterator, TimeoutMap::iterator> range = m_timeouts.equal_range(awaitedTime);
    for (TimeoutMap::iterator timeout = range.first; timeout != range.second; ++timeout)
    {
        if (timeout->second == pPacket)
        {
            m_timeouts.erase(timeout);
            m_packets.erase(pPacket);
            pPacket->ReleaseRef();
            return true;
        }
    }

    return false;
}

}
}

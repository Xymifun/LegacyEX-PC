#include "nn/nex/RoutingTable.hpp"

#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

bool RoutingTable::RTEntry::IsExpired(unsigned int uiElapsed) const
{
    bool bExpired = false;
    if (m_timeRemaining > uiElapsed)
    {
        m_timeRemaining -= uiElapsed;
    }
    else
    {
        bExpired = true;
    }
    return bExpired;
}

RoutingTable::RoutingTable(unsigned int uiTimeout) : m_entries(), m_timeout(uiTimeout), m_lastCleaning(Time::GetTime())
{
}

bool RoutingTable::Add(const InetAddress& rKey, const InetAddress& rAddress, bool bReplace)
{
    bool bAdded = false;
    if (rAddress.GetAddress() != 0)
    {
        if (!(rAddress == rKey))
        {
            std::pair<EntryMap::iterator, bool> result = m_entries.insert(
                std::make_pair(rKey, RTEntry(rAddress, m_timeout)));
            bAdded = result.second;
            if (!bAdded && bReplace)
            {
                result.first->second.m_address = rAddress;
                result.first->second.m_timeRemaining = m_timeout;
                bAdded = true;
            }
        }
    }
    return bAdded;
}

bool RoutingTable::Remove(const InetAddress& rKey)
{
    std::pair<EntryMap::iterator, EntryMap::iterator> range = m_entries.equal_range(rKey);
    unsigned int count = 0;
    for (EntryMap::iterator it = range.first; it != range.second; ++it)
        ++count;
    m_entries.erase(range.first, range.second);
    return count == 1;
}

bool RoutingTable::Find(const InetAddress& rKey, InetAddress& rAddress) const
{
    bool bFound = false;
    EntryMap::const_iterator found = m_entries.find(rKey);
    if (found != m_entries.end())
    {
        found->second.m_timeRemaining = m_timeout;
        rAddress = found->second.m_address;
        bFound = true;
    }
    return bFound;
}

void RoutingTable::Clean()
{
    m_entries.clear();
}

void RoutingTable::PeriodicHouseCleaning()
{
    Time now = Time::GetTime();
    unsigned int elapsed = static_cast<unsigned int>(now - m_lastCleaning);
    if (elapsed >= 1000)
    {
        m_lastCleaning = now;
        EntryMap::iterator it = m_entries.begin();
        while (it != m_entries.end())
        {
            if (!it->second.IsExpired(elapsed))
                ++it;
            else
                m_entries.erase(it++);
        }
    }
}

}
}

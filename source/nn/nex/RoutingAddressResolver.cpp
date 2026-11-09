#include "nn/nex/RoutingAddressResolver.hpp"

namespace nn
{
namespace nex
{

void RoutingAddressResolver::AddMapping(unsigned short uiID, const InetAddress& rAddress)
{
    m_idToAddress[uiID] = rAddress;
    m_addressToId[rAddress] = uiID;
}

bool RoutingAddressResolver::RemoveMappingFromID(unsigned short uiID)
{
    IdToAddressMap::iterator it = m_idToAddress.find(uiID);
    if (it != m_idToAddress.end())
    {
        m_addressToId.erase(it->second);
        m_idToAddress.erase(it);
        return true;
    }
    return false;
}

bool RoutingAddressResolver::RemoveMappingFromAddress(const InetAddress& rAddress)
{
    AddressToIdMap::iterator it = m_addressToId.find(rAddress);
    if (it != m_addressToId.end())
    {
        m_idToAddress.erase(it->second);
        m_addressToId.erase(it);
        return true;
    }
    return false;
}

bool RoutingAddressResolver::ResolveToID(const InetAddress& rAddress, unsigned short* pID) const
{
    AddressToIdMap::const_iterator it = m_addressToId.find(rAddress);
    if (it != m_addressToId.end())
    {
        *pID = it->second;
        return true;
    }
    return false;
}

bool RoutingAddressResolver::ResolveToAddress(unsigned short uiID, InetAddress* pAddress) const
{
    IdToAddressMap::const_iterator it = m_idToAddress.find(uiID);
    if (it != m_idToAddress.end())
    {
        *pAddress = it->second;
        return true;
    }
    return false;
}

void RoutingAddressResolver::Trace(unsigned long long) const
{
    for (IdToAddressMap::const_iterator it = m_idToAddress.begin(); it != m_idToAddress.end(); ++it)
    {
    }
}

}
}

#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ROUTINGADDRESSRESOLVER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ROUTINGADDRESSRESOLVER_HPP

#include <stdint.h>
#include <functional>
#include <map>
#include <utility>

#include "nn/nex/InetAddress.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class RoutingAddressResolver : public RootObject
{
public:
    typedef std::map<unsigned short, InetAddress, std::less<unsigned short>,
                     MemAllocator<std::pair<const unsigned short, InetAddress> > >
        IdToAddressMap;
    typedef std::map<InetAddress, unsigned short, std::less<InetAddress>,
                     MemAllocator<std::pair<const InetAddress, unsigned short> > >
        AddressToIdMap;

    void AddMapping(unsigned short uiID, const InetAddress& rAddress);
    bool RemoveMappingFromID(unsigned short uiID);
    bool RemoveMappingFromAddress(const InetAddress& rAddress);
    bool ResolveToID(const InetAddress& rAddress, unsigned short* pID) const;
    bool ResolveToAddress(unsigned short uiID, InetAddress* pAddress) const;
    void Trace(unsigned long long uiFilter) const;

private:
    uint32_t m_unresolved0;
    IdToAddressMap m_idToAddress;
    AddressToIdMap m_addressToId;
};

}
}

#endif

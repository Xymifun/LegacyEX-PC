#include "nn/nex/VirtualFilterDevice.hpp"

namespace nn
{
namespace nex
{

bool VirtualFilterDevice::Match(const InetAddress& rAddress)
{
    if (m_addresses.find(rAddress) == m_addresses.end())
    {
        std::set<InetAddress, std::less<InetAddress>, MemAllocator<InetAddress> >::iterator it = m_addresses.find(
            InetAddress(0u, rAddress.GetPortNumber()));
        if (it == m_addresses.end())
            return false;
    }
    return true;
}

}
}

#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_VIRTUALFILTERDEVICE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_VIRTUALFILTERDEVICE_HPP

#include "nn/nex/InetAddress.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/qSet.hpp"

namespace nn
{
namespace nex
{

class VirtualFilterDevice : public RootObject
{
public:
    bool Match(const InetAddress& rAddress);

    void Reset() { m_addresses.clear(); }

    void Add(const InetAddress& rAddress) { m_addresses.insert(rAddress); }

private:
    qSet<InetAddress> m_addresses;
};

}
}

#endif

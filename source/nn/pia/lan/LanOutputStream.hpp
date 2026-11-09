#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANOUTPUTSTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANOUTPUTSTREAM_HPP

#include "nn/pia.h"
#include "nn/pia/common/IPacketOutput.hpp"
#include "nn/pia/common/SocketStreamBase.hpp"
#include "nn/pia/common/SocketAddress.hpp"
#include "nn/pia/Transport.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class Packet;
class InetAddress;
struct SockAddrIn;
}
namespace lan
{
class LanProtocol;

class LanOutputStream : public common::SocketStreamBase, public common::IPacketOutput
{
public:
    LanOutputStream();
    virtual ~LanOutputStream();
    virtual Result OnStationConnectionEvent();
    virtual Result CalculateCryptoParameter(unsigned char* output, unsigned int size, common::Packet* packet);
    int CreateMulticastAddressArray(common::SockAddrIn* output, unsigned int mask);
    Result SendTo(const void* data, unsigned int size, const common::InetAddress& address, unsigned char ttl);
    bool IsDestinationAddressIndividual(common::SockAddrIn* addresses, unsigned int count) const;
    Result SendToMulti(const void* data, unsigned int size, const common::SockAddrIn* addresses, int count);
    virtual Result Write(const common::Packet& packet);
    void RegisterLanProtocol(LanProtocol* protocol);
    void UnregisterLanProtocol();
    virtual unsigned int GetMulticastDestinationNum() const;
    virtual unsigned int IsBroadcastSupported() const;

private:
    common::SockAddrIn m_stationAddresses[transport::cStationNumMax];
    LanProtocol* m_protocol;
};

}
}
}

#endif

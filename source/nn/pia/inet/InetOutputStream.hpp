#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETOUTPUTSTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETOUTPUTSTREAM_HPP

#include "nn/pia/common/IPacketOutput.hpp"
#include "nn/pia/common/SocketAddress.hpp"
#include "nn/pia/common/SocketStreamBase.hpp"
#include "nn/pia/Transport.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class InetAddress;
class Packet;
}
}
}

namespace nn
{
namespace pia
{
namespace inet
{

class InetOutputStream : public common::SocketStreamBase, public common::IPacketOutput
{
public:
    InetOutputStream();
    virtual ~InetOutputStream();
    virtual Result OnStationConnectionEvent();
    virtual Result CalculateCryptoParameter(unsigned char* parameter, unsigned int size, common::Packet* packet);
    int CreateMulticastAddressArray(common::SockAddrIn* addresses, unsigned int stationBitmap);
    Result SendTo(const void* data, unsigned int size, const common::InetAddress& address, unsigned char ttl);
    Result SendToRelay(const void* data, unsigned int size, const common::InetAddress& address, unsigned char ttl);
    Result SendToMulti(const void* data, unsigned int size, const common::SockAddrIn* addresses, int count);
    virtual Result Write(const common::Packet& packet);
    virtual unsigned int GetMulticastDestinationNum() const;
    virtual unsigned int IsBroadcastSupported() const;

private:
    common::SockAddrIn m_stationAddresses[transport::cStationNumMax];
};

}
}
}

#endif

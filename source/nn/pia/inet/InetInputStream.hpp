#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETINPUTSTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETINPUTSTREAM_HPP

#include "nn/pia/common/IPacketInput.hpp"
#include "nn/pia/common/SocketStreamBase.hpp"

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

class InetInputStream : public common::SocketStreamBase, public common::IPacketInput
{
public:
    enum
    {
        cIgnoreAddressNum = 2
    };

    InetInputStream();
    virtual ~InetInputStream();
    virtual Result Read(common::Packet* packet);
    virtual Result CalculateCryptoParameter(unsigned char* parameter, unsigned int size, common::Packet* packet);
    static void AddIgnoreAddress(const common::InetAddress& address1, const common::InetAddress& address2);

private:
    Result readFromSocket(common::Packet* packet);

    static common::InetAddress s_ignoreAddresses[cIgnoreAddressNum];
};

}
}
}

#endif

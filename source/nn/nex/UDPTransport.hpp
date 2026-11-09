#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_UDPTRANSPORT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_UDPTRANSPORT_HPP

#include "nn/nex/SocketTransport.hpp"

namespace nn
{
namespace nex
{

class UDPTransport : public SocketTransport
{
public:
    UDPTransport();
    virtual ~UDPTransport();
};

}
}

#endif

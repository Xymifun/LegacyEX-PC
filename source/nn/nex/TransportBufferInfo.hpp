#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTBUFFERINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTBUFFERINFO_HPP

#include "nn/nex/InetAddress.hpp"

namespace nn
{
namespace nex
{

class Buffer;

struct TransportBufferInfo
{
    Buffer* m_pBuffer;
    InetAddress m_address;
};

}
}

#endif

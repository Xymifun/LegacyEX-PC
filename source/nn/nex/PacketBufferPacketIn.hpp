#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PACKETBUFFERPACKETIN_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PACKETBUFFERPACKETIN_HPP

#include "nn/nex/PacketIn.hpp"

namespace nn
{
namespace nex
{

class PacketBufferPacketIn : public PacketIn
{
public:
    explicit PacketBufferPacketIn(unsigned int uiBufferSize);
};

}
}

#endif

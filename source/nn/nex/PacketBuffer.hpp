#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PACKETBUFFER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PACKETBUFFER_HPP

#include "nn/nex/Buffer.hpp"

namespace nn
{
namespace nex
{

class PacketBuffer : public Buffer
{
public:
    PacketBuffer(unsigned int uiRealSize, unsigned char uiFlags) : Buffer(uiRealSize, uiFlags) {}
};

}
}

#endif

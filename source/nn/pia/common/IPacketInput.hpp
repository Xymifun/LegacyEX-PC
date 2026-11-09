#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_IPACKET_INPUT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_IPACKET_INPUT_HPP

#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class Packet;

class IPacketInput
{
public:
    virtual ~IPacketInput() {}
    virtual Result Read(Packet* packet) = 0;
    virtual Result CalculateCryptoParameter(unsigned char* parameter, unsigned int size, Packet* packet) = 0;
};

}
}
}

#endif

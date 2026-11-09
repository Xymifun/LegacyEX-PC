#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_IPACKET_OUTPUT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_IPACKET_OUTPUT_HPP

#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class Packet;

class IPacketOutput
{
public:
    virtual ~IPacketOutput() {}
    virtual Result OnStationConnectionEvent() = 0;
    virtual Result CalculateCryptoParameter(unsigned char* parameter, unsigned int size, Packet* packet) = 0;
    virtual Result Write(const Packet& packet) = 0;
    virtual unsigned int GetMulticastDestinationNum() const;
    virtual unsigned int IsBroadcastSupported() const;
};

}
}
}

#endif

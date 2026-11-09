#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PACKETIN_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PACKETIN_HPP

#include "nn/nex/Packet.hpp"

namespace nn
{
namespace nex
{

class PacketIn : public Packet
{
public:
    PacketIn();
    explicit PacketIn(int);
};

}
}

#endif

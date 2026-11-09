#include "nn/nex/PacketBufferPacketIn.hpp"

#include "nn/nex/PacketBuffer.hpp"

namespace nn
{
namespace nex
{

PacketBufferPacketIn::PacketBufferPacketIn(unsigned int uiBufferSize) : PacketIn(0)
{
    SetPayload(new (RootObject::TARGET_POOL_PACKET_MANAGEMENT) PacketBuffer(uiBufferSize, 4));
}

}
}

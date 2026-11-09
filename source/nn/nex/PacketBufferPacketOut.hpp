#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PACKETBUFFERPACKETOUT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PACKETBUFFERPACKETOUT_HPP

#include "nn/nex/PacketOut.hpp"
#include "nn/nex/Timeout.hpp"

namespace nn
{
namespace nex
{

class PacketBufferPacketOut : public PacketOut
{
public:
    PacketBufferPacketOut(PRUDPEndPoint* pEndPoint, PacketType type, unsigned short uiFlags, Buffer* pPayload,
                          unsigned int uiSize)
        : PacketOut()
    {
        Initialize(pEndPoint, type, uiFlags, pPayload, uiSize,
                   (uiFlags & PACKET_FLAG_NEED_ACK) ? new (RootObject::TARGET_POOL_PACKET_MANAGEMENT) Timeout() : 0);
    }

    virtual ~PacketBufferPacketOut() {}
};

}
}

#endif

#include "nn/nex/PacketIn.hpp"

#include "nn/nex/Buffer.hpp"

namespace nn
{
namespace nex
{

PacketIn::PacketIn(int)
{
}

PacketIn::PacketIn()
{
    Buffer* pPayload = new Buffer;
    SetPayload(pPayload);
}

}
}

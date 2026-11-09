#include "nn/nex/_Type_buffer.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

bool _Type_buffer::Extract(Message* pMessage, Buffer* pDestination)
{
    unsigned int uiSize = 0;
    if (!pMessage->ExtractPrimitive(&uiSize) || !pMessage->ValidateBufferLimit(uiSize))
        return false;
    const Buffer* pBuffer = pMessage->GetBuffer();
    pDestination->AppendData(pBuffer->GetData() + pMessage->GetPosition(), uiSize, 0xFFFFFFFF);
    const unsigned int uiPosition = pMessage->GetPosition() + uiSize;
    if (uiPosition <= pMessage->GetLength())
        pMessage->SetPosition(uiPosition);
    return true;
}

}
}

#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__TYPE_BUFFER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__TYPE_BUFFER_HPP

#include "nn/nex/Buffer.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

class _Type_buffer
{
public:
    static void Add(Message* pMessage, const Buffer& rSource)
    {
        const unsigned int uiLength = rSource.GetLength();
        pMessage->Append(reinterpret_cast<const unsigned char*>(&uiLength), sizeof(uiLength), 1);
        pMessage->Append(rSource.GetData(), 1, rSource.GetLength());
    }

    static bool Extract(Message* pMessage, Buffer* pDestination);
};

}
}

#endif

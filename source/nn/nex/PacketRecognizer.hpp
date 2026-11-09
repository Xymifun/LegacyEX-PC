#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PACKETRECOGNIZER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PACKETRECOGNIZER_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class StationURL;

class PacketRecognizer : public RootObject
{
public:
    virtual ~PacketRecognizer() {}

    virtual bool Recognize(Buffer* pBuffer, StationURL* pURL) = 0;
};

}
}

#endif

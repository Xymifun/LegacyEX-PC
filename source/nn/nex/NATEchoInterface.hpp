#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NATECHOINTERFACE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NATECHOINTERFACE_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class StationURL;

class NATEchoInterface : public RootObject
{
public:
    virtual ~NATEchoInterface() {}

    virtual bool PrepareMessage(Buffer* pRequest, Buffer* pResponse, StationURL* pStationURL) = 0;
};

}
}

#endif

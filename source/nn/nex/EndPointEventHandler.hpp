#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ENDPOINTEVENTHANDLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ENDPOINTEVENTHANDLER_HPP

#include "nn/nex/RefCountedObject.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class EndPoint;

class EndPointEventHandler : public RefCountedObject
{
public:
    virtual ~EndPointEventHandler() {}

    virtual void Receive(EndPoint* pEndPoint, Buffer* pBuffer, unsigned char ucParameter) = 0;
    virtual void FaultDetection(EndPoint* pEndPoint, unsigned int uiError) = 0;
    virtual void PeerDisconnected(EndPoint* pEndPoint) = 0;
};

}
}

#endif

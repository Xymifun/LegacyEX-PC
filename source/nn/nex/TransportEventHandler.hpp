#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTEVENTHANDLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTEVENTHANDLER_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class ConnectionOrientedStream;
class EndPoint;
class StationURL;

class TransportEventHandler : public RootObject
{
public:
    TransportEventHandler();
    virtual ~TransportEventHandler();

    virtual void Receive(ConnectionOrientedStream* pStream, Buffer* pBuffer, const StationURL* pStationURL);
    virtual void ReceiveBroadcast(ConnectionOrientedStream* pStream, Buffer* pBuffer, const StationURL* pStationURL);
    virtual bool SwitchToRoutingConnection(EndPoint* pEndPoint);
    virtual void DeviceDiscovery(ConnectionOrientedStream* pStream, wchar_t* pDevice);
    virtual bool ConnectionRequest(ConnectionOrientedStream* pStream, const StationURL* pStationURL, Buffer* pBuffer,
                                   EndPoint* pEndPoint);
};

}
}

#endif

#include "nn/nex/TransportEventHandler.hpp"

namespace nn
{
namespace nex
{

TransportEventHandler::TransportEventHandler()
{
}

TransportEventHandler::~TransportEventHandler()
{
}

void TransportEventHandler::Receive(ConnectionOrientedStream*, Buffer*, const StationURL*)
{
}

void TransportEventHandler::ReceiveBroadcast(ConnectionOrientedStream*, Buffer*, const StationURL*)
{
}

bool TransportEventHandler::SwitchToRoutingConnection(EndPoint*)
{
    return false;
}

void TransportEventHandler::DeviceDiscovery(ConnectionOrientedStream*, wchar_t*)
{
}

bool TransportEventHandler::ConnectionRequest(ConnectionOrientedStream*, const StationURL*, Buffer*, EndPoint*)
{
    return false;
}

}
}

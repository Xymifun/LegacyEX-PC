#include "nn/pia/inet/InetMissingStationHandler.hpp"

#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{
enum
{
    cTraceStation = 0x800
};
}

InetMissingStationHandler::~InetMissingStationHandler()
{
}

void InetMissingStationHandler::Execute(transport::StationConnectionInfo* connectionInfo)
{
    NexFacade::GetInstance()->NotifyLeave(connectionInfo->m_stationLocation, true, false);
    connectionInfo->Trace(cTraceStation);
}

}
}
}

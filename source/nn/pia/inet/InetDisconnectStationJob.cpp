#include "nn/pia/inet/InetDisconnectStationJob.hpp"

#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/StationLocation.hpp"

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

InetDisconnectStationJob::InetDisconnectStationJob()
{
}

InetDisconnectStationJob::~InetDisconnectStationJob()
{
}

void InetDisconnectStationJob::Trace(unsigned long long) const
{
}

void InetDisconnectStationJob::ProcessDisconnection(transport::Station* station)
{
    transport::StationConnectionInfoTable* table = transport::g_stationConnectionInfoTable;
    transport::StationConnectionInfo connectionInfo;
    Result result = table->GetStationConnectionInfo(station, &connectionInfo);
    if (result == 0)
    {
        bool clearMonitoringData = true;
        session::Session::DisconnectionEventListener listener = transport::g_piaTransport->m_disconnectionEventListener;
        if (listener)
        {
            unsigned long long principalId;
            result = station->GetPrincipalId(&principalId);
            if (result != 0)
                return;
            transport::StationIdTable::Entry entry;
            result = transport::g_piaTransport->m_stationIdTable->FindByUniqueId(&entry, principalId);
            if (result == 0 && listener(&entry) == 1)
                clearMonitoringData = false;
        }
        transport::StationLocation location;
        location = connectionInfo.m_stationLocation;
        location.SetStationAddress(station->m_stationAddress);
        NexFacade::GetInstance()->NotifyLeave(location, clearMonitoringData, false);
        connectionInfo.Trace(cTraceStation);
    }
}

}
}
}

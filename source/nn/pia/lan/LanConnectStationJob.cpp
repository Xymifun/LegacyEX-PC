#include "nn/pia/lan/LanConnectStationJob.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

namespace
{
const unsigned long long cTraceStation = 0x0100000000000000ULL;
const unsigned long long cTraceStationLocation = 0x0200000000000000ULL;
}

LanConnectStationJob::LanConnectStationJob()
{
    m_traceFlag = cTraceStation;
}

LanConnectStationJob::~LanConnectStationJob()
{
}

Result LanConnectStationJob::StartupImpl(common::CallContext* context, transport::Station* station,
                                         const transport::StationConnectionInfo& info, bool secure)
{
    m_station = station;
    m_secure = secure;
    info.Trace(cTraceStation);
    m_connectionInfo = info;
    m_connectionInfo.m_stationLocation.m_field25 = 3;
    m_connectionInfo.m_localStationLocation.m_field25 = 0;
    m_connectionInfo.Trace(cTraceStation);
    station->GetStationAddress().Trace(cTraceStation);
    session::g_piaMesh->TraceStationConnectionInfoList(cTraceStationLocation);
    m_currentLocation.SetStationLocation(m_connectionInfo.m_localStationLocation);
    if (station->GetStationIndex() == cStationIndexSentinelFE)
    {
        transport::g_stationConnectionInfoTable->m_hostStationConnectionInfo = m_connectionInfo;
        transport::g_stationConnectionInfoTable->GetHostStationConnectionInfo().Trace(cTraceStation);
        transport::g_stationConnectionInfoTable->m_hostStationAddress = m_currentLocation.m_stationAddress;
        transport::g_stationConnectionInfoTable->GetHostStationAddress().Trace(cTraceStation);
    }
    transport::StationConnectionInfo currentInfo;
    currentInfo.m_stationLocation.SetStationLocation(m_currentLocation);
    Result result;
    result = transport::ConnectStationJob::StartupImpl(context, station, currentInfo, secure);
    if (result != 0)
        return result;
    Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LanConnectStationJob::TryCurrentAddress),
              "LanConnectStationJob::TryCurrentAddress");
    m_currentStep = step;
    return 0;
}

void LanConnectStationJob::Trace(unsigned long long) const
{
}

LanConnectStationJob::ExecuteResult LanConnectStationJob::TryCurrentAddress()
{
    m_station->GetStationAddress() = m_currentLocation.m_stationAddress;
    m_station->Trace(cTraceStationLocation);
    Step step(
        static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&transport::ConnectStationJob::SendConnectionRequest),
        "ConnectStationJob::SendConnectionRequest");
    m_currentStep = step;
    return MakeExecuteResult(cExecuteKindContinue);
}

}
}
}

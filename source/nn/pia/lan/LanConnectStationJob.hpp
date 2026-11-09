#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_CONNECT_STATION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_CONNECT_STATION_JOB_HPP

#include "nn/pia/transport/ConnectStationJob.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanConnectStationJob : public transport::ConnectStationJob
{
public:
    LanConnectStationJob();
    ~LanConnectStationJob();
    Result StartupImpl(common::CallContext* context, transport::Station* station,
                       const transport::StationConnectionInfo& info, bool secure);
    void Trace(unsigned long long value) const;
    ExecuteResult TryCurrentAddress();

private:
    transport::StationLocation m_currentLocation;
    transport::StationConnectionInfo m_connectionInfo;
};

}
}
}

#endif

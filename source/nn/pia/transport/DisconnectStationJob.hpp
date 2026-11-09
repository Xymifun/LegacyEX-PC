#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_DISCONNECT_STATION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_DISCONNECT_STATION_JOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class Station;

class DisconnectStationJob : public common::StepSequenceJob
{
public:
    DisconnectStationJob();
    virtual ~DisconnectStationJob();

    Result Startup(Station* station);
    ExecuteResult SendDisconnectionRequest();
    ExecuteResult WaitForDisconnection();
    ExecuteResult DisconnectionSucceeded();
    ExecuteResult CutRouteOfRelayConnection();
    void Cleanup();
    virtual void Trace(unsigned long long value) const;
    virtual void ProcessDisconnection(Station* station);
    virtual Result StartupImpl(Station* station);
    void DisconnectionFailed();

private:
    common::Time m_deadline;
    int m_timeoutMsec;
    Station* m_station;
    common::Time m_retryDeadline;
    int m_retryIntervalMsec;

public:
    bool m_waiting;
};

}
}
}

#endif

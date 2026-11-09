#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_CONNECT_STATION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_CONNECT_STATION_JOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class CallContext;
}
namespace transport
{

class Station;
class StationConnectionInfo;

class ConnectStationJob : public common::StepSequenceJob
{
public:
    ConnectStationJob();
    virtual ~ConnectStationJob();
    Result Startup(common::CallContext* context, Station* station, const StationConnectionInfo& info, bool secure,
                   int timeoutMsec);
    Result StartupRelayConnection(common::CallContext* context, Station* station, const StationConnectionInfo& info,
                                  bool secure, int timeoutMsec);
    void Cleanup();
    void Trace(unsigned long long value) const;
    virtual Result StartupImpl(common::CallContext* context, Station* station, const StationConnectionInfo& info,
                               bool secure);
    virtual void CleanupImpl();
    virtual bool CheckErrorOccurredImpl();
    bool CheckContextCallCancelled();
    bool CheckErrorOccurred();
    bool SendConnectionRequestMessage();
    ExecuteResult SendConnectionRequest();
    bool CheckTimeout();
    ExecuteResult WaitRequestAck();
    ExecuteResult WaitForConnection();
    ExecuteResult ConnectionFailed();
    ExecuteResult ConnectionSucceeded();
    bool SendRelayConnectionRequestMessage();
    ExecuteResult SendRelayConnectionRequest();

    friend class StationProtocol;
    friend class ProcessConnectionRequestJob;

protected:
    common::Time m_connectionDeadline;
    int m_timeoutMsec;
    unsigned int m_requestAckId;
    common::CallContext* m_context;
    Station* m_station;
    bool m_secure;
    unsigned char m_connectionRejection;
};

}
}
}

#endif

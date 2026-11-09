#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETCONNECTSTATIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETCONNECTSTATIONJOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/transport/ConnectStationJob.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"
#include "nn/pia/transport/StationLocation.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class InetConnectStationJob : public transport::ConnectStationJob
{
public:
    InetConnectStationJob();
    virtual ~InetConnectStationJob();
    virtual Result StartupImpl(common::CallContext* context, transport::Station* station,
                               const transport::StationConnectionInfo& info, bool secure);
    virtual void CleanupImpl();
    virtual bool CheckErrorOccurredImpl();
    virtual void Trace(unsigned long long value) const;
    ExecuteResult TestCurrentAddress();
    ExecuteResult PrepareNatTraversal();
    ExecuteResult WaitForNatTraversalStarted();
    ExecuteResult WaitForNatTraversalCompleted();
    ExecuteResult PrepareNatTraversalByRelay();
    ExecuteResult WaitForNatTraversalStartedByRelay();
    ExecuteResult WaitForNatTraversalCompletedByRelay();
    ExecuteResult ResolveCurrentAddress();
    ExecuteResult TryCurrentAddress();

private:
    transport::StationLocation m_stationLocation;
    transport::StationConnectionInfo m_stationConnectionInfo;
    common::Time m_natTraversalDeadline;
    common::CallContext m_natTraversalContext;
    int m_natTraversalRetryCount;
    int m_natTraversalStartRetryCount;
};

}
}
}

#endif

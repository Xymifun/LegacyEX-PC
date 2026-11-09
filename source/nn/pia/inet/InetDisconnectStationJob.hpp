#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETDISCONNECTSTATIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETDISCONNECTSTATIONJOB_HPP

#include "nn/pia/transport/DisconnectStationJob.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class InetDisconnectStationJob : public transport::DisconnectStationJob
{
public:
    InetDisconnectStationJob();
    virtual ~InetDisconnectStationJob();
    virtual void Trace(unsigned long long value) const;
    virtual void ProcessDisconnection(transport::Station* station);
};

}
}
}

#endif

#include "nn/pia/inet/MonitoringDataProtocol.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

MonitoringDataProtocol::MonitoringDataProtocol()
{
}

MonitoringDataProtocol::~MonitoringDataProtocol()
{
}

Result MonitoringDataProtocol::Startup()
{
    return 0;
}

void MonitoringDataProtocol::Cleanup()
{
}

Result MonitoringDataProtocol::Dispatch()
{
    return 0;
}

void MonitoringDataProtocol::Trace(unsigned long long) const
{
}

unsigned short MonitoringDataProtocol::GetProtocolType() const
{
    return cProtocolType;
}

}
}
}

#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_MONITORINGDATAPROTOCOL_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_MONITORINGDATAPROTOCOL_HPP

#include "nn/pia/Reconstruction.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class InetAddress;
}
}
}

namespace nn
{
namespace pia
{
namespace inet
{

class MonitoringDataProtocol : public transport::Protocol
{
public:
    enum
    {
        cProtocolType = 0xa4
    };

    MonitoringDataProtocol();
    virtual ~MonitoringDataProtocol();
    virtual Result Startup();
    virtual void Cleanup();
    virtual Result Dispatch();
    virtual void Trace(unsigned long long value) const;
    virtual unsigned short GetProtocolType() const;
    Result SendMonitoringData(common::InetAddress& address, unsigned char* data, unsigned int size);
};

}
}
}

#endif

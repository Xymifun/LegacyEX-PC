#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_IFACADE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_IFACADE_HPP

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
namespace transport
{
class StationLocation;
}
}
}

namespace nn
{
namespace pia
{
namespace inet
{

class IFacade
{
public:
    virtual ~IFacade() {}

    virtual unsigned int GetRandomPortRangeMin() const = 0;
    virtual unsigned int GetRandomPortRangeMax() const = 0;
    virtual unsigned short GetReservedPort() const = 0;
    virtual void RelayProbeRequest(const transport::StationLocation& source,
                                   const transport::StationLocation& destination) = 0;
    virtual void UpdateRelayTable() = 0;
    virtual const common::InetAddress& GetRelayServerAddress() const = 0;
    virtual unsigned int GetLocalServiceConnectionId() const = 0;
    virtual bool IsBeginMonitoringDataAvailable() const = 0;
};

}
}
}

#endif

#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_MISSINGSTATIONHANDLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_MISSINGSTATIONHANDLER_HPP

namespace nn
{
namespace pia
{
namespace transport
{

class StationConnectionInfo;

class MissingStationHandler
{
public:
    virtual ~MissingStationHandler() {}

    virtual void Execute(StationConnectionInfo* connectionInfo);
};

}
}
}

#endif

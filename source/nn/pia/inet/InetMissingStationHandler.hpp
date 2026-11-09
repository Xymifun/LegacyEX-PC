#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETMISSINGSTATIONHANDLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETMISSINGSTATIONHANDLER_HPP

#include "nn/pia/transport/MissingStationHandler.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class InetMissingStationHandler : public transport::MissingStationHandler
{
public:
    virtual ~InetMissingStationHandler();
    virtual void Execute(transport::StationConnectionInfo* connectionInfo);
};

}
}
}

#endif

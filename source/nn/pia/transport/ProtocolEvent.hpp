#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PROTOCOL_EVENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PROTOCOL_EVENT_HPP

#include "nn/pia/common/StationAddress.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class ProtocolEvent
{
public:
    unsigned int m_type;
    common::StationAddress m_stationAddress;
};

}
}
}

#endif

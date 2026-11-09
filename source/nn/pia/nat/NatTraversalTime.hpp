#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATTRAVERSALTIME_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATTRAVERSALTIME_HPP

#include "nn/pia/common/Time.hpp"
#include "nn/pia/transport/StationLocation.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

struct NatTraversalTime
{
    transport::StationLocation m_location;
    common::Time m_startTime;
    common::Time m_endTime;
    unsigned int m_count;
    unsigned short m_port;
};

}
}
}

#endif

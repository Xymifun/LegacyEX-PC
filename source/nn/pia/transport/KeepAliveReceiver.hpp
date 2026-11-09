#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_KEEPALIVERECEIVER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_KEEPALIVERECEIVER_HPP

#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class KeepAliveReceiver
{
public:
    KeepAliveReceiver();
    ~KeepAliveReceiver();
    void Update(unsigned int stationBitmap, const common::Time& time);
    void Trace(unsigned long long value) const;

    bool m_enabled;
};

}
}
}

#endif

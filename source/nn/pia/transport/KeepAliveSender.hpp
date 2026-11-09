#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_KEEPALIVESENDER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_KEEPALIVESENDER_HPP

#include "nn/pia/common/Time.hpp"
#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class KeepAliveSender
{
public:
    KeepAliveSender();
    ~KeepAliveSender();
    Result SetInterval(int interval);
    unsigned int Update(unsigned int stationBitmap, const common::Time& time);
    void Trace(unsigned long long value) const;

    unsigned int m_interval;
    bool m_enabled;
};

}
}
}

#endif

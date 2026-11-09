#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TIMEPROVIDER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TIMEPROVIDER_HPP

#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class TimeProvider
{
public:
    virtual ~TimeProvider();
    virtual Time GetTime() = 0;
};

}
}

#endif

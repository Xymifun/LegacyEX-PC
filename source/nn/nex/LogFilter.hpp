#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_LOGFILTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_LOGFILTER_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class LogFilter : public RootObject
{
public:
    virtual ~LogFilter() {}

    virtual bool OutputCurrentTrace() = 0;
};

}
}

#endif

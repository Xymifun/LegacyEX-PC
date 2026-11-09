#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PIDLOGFILTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PIDLOGFILTER_HPP

#include "nn/nex/LogFilter.hpp"

namespace nn
{
namespace nex
{

class PIDLogFilter : public LogFilter
{
public:
    virtual ~PIDLogFilter() {}

    virtual bool OutputCurrentTrace();

private:
    unsigned int m_lowerBound;
    unsigned int m_upperBound;
};

}
}

#endif

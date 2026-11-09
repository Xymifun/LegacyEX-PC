#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CIDLOGFILTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CIDLOGFILTER_HPP

#include "nn/nex/LogFilter.hpp"

namespace nn
{
namespace nex
{

class CIDLogFilter : public LogFilter
{
public:
    virtual ~CIDLogFilter() {}

    virtual bool OutputCurrentTrace();

private:
    unsigned int m_lowerBound;
    unsigned int m_upperBound;
};

}
}

#endif

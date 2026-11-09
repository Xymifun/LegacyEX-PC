#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STATISTICS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STATISTICS_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Statistics : public RootObject
{
public:
    Statistics();
    ~Statistics();

    void Reset();
    void AddValue(float fValue);
    float GetAverage() const;
    void Trace(unsigned long long uiFilter, wchar_t* pName);
};

}
}

#endif

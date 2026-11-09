#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TIMEDSIGNAL_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TIMEDSIGNAL_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class TimedSignal : public RootObject
{
public:
    TimedSignal();
    explicit TimedSignal(const wchar_t* pName);
    ~TimedSignal();

    void Reset();
    void SetOff();
    void SetOn(int iDuration);
    void SetPulse(int iDuration);
    void SetOnceOn(int iDuration);
    bool IsDeadline();
    bool IsSignaled();
    void Trace();

private:
    uint32_t m_unresolved0;
    uint32_t m_unresolved1;
    uint32_t m_unresolved2;
    uint32_t m_unresolved3;
    uint32_t m_unresolved4;
    String m_name;
};

}
}

#endif

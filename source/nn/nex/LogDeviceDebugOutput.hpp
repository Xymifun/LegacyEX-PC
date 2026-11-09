#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_LOGDEVICEDEBUGOUTPUT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_LOGDEVICEDEBUGOUTPUT_HPP

#include "nn/nex/LogDevice.hpp"

namespace nn
{
namespace nex
{

class LogDeviceDebugOutput : public LogDevice
{
public:
    virtual ~LogDeviceDebugOutput() {}
    virtual void Output(const wchar_t* pText);
};

}
}

#endif

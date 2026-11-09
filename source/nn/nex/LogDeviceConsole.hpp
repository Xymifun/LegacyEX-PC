#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_LOGDEVICECONSOLE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_LOGDEVICECONSOLE_HPP

#include "nn/nex/LogDevice.hpp"

namespace nn
{
namespace nex
{

class LogDeviceConsole : public LogDevice
{
public:
    explicit LogDeviceConsole(void* pHandle);
    virtual ~LogDeviceConsole();
    virtual void Output(const wchar_t* pText);

protected:
    void* m_pHandle;
};

}
}

#endif

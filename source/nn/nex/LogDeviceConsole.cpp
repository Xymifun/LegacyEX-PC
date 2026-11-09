#include "nn/nex/LogDeviceConsole.hpp"

#include <cstdio>

namespace nn
{
namespace nex
{

LogDeviceConsole::LogDeviceConsole(void* pHandle) : m_pHandle(pHandle)
{
    if (pHandle == 0)
    {
        m_pHandle = stdout;
    }
}

LogDeviceConsole::~LogDeviceConsole()
{
}

void LogDeviceConsole::Output(const wchar_t* pText)
{
    std::printf("%ls\n", pText);
}

}
}

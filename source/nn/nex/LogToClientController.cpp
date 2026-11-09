#include "nn/nex/LogToClientController.hpp"

#include "nn/nex/Core.hpp"

namespace nn
{
namespace nex
{

LogToClientController::LogDeviceInterface* LogToClientController::LogDeviceInterface::s_pInstance = 0;

LogToClientController::LogDeviceInterface::LogDeviceInterface()
{
    ScopedCS lock(&Core::GetInstance()->m_criticalSection);
    s_pInstance = this;
}

LogToClientController::LogDeviceInterface::~LogDeviceInterface()
{
    ScopedCS lock(&Core::GetInstance()->m_criticalSection);
    s_pInstance = 0;
}

LogToClientController::LogDeviceInterface* LogToClientController::LogDeviceInterface::GetInstance()
{
    return s_pInstance;
}

void LogToClientController::Accumulate()
{
    ScopedCS lock(&Core::GetInstance()->m_criticalSection);
    LogDeviceInterface* pInterface = LogDeviceInterface::GetInstance();
    if (pInterface != 0)
    {
        pInterface->Accumulate();
    }
}

void LogToClientController::Flush(EndPoint* pEndPoint)
{
    ScopedCS lock(&Core::GetInstance()->m_criticalSection);
    LogDeviceInterface* pInterface = LogDeviceInterface::GetInstance();
    if (pInterface != 0 && pInterface->CanFlush())
    {
        pInterface->Flush(pEndPoint);
    }
}

}
}

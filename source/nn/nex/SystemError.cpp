#include "nn/nex/SystemError.hpp"

#include "nn/nex/GlobalVariables.hpp"

namespace nn
{
namespace nex
{

SystemError::~SystemError()
{
}

void SystemError::SignalError(wchar_t*, unsigned int, unsigned int errorCode, unsigned int errorDetail)
{
    GlobalVariables::GetInstancePtr()->GetErrorCode().SetValue(errorCode);
    GlobalVariables::GetInstancePtr()->GetErrorDetail().SetValue(errorDetail);
}

void SystemError::EraseErrorElements(unsigned int threadId)
{
    g_globalVariables->m_errorCode.ClearValue(threadId);
    g_globalVariables->m_errorDetail.ClearValue(threadId);
}

}
}

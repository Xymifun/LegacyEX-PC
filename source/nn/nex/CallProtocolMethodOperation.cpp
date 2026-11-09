#include "nn/nex/CallProtocolMethodOperation.hpp"

namespace nn
{
namespace nex
{

enum
{
    CALL_RESULT_SUCCESS = 0x00010001
};

CallProtocolMethodOperation::CallProtocolMethodOperation() : Operation(0)
{
    m_attemptCount = 0;
    m_postponed = false;
    m_postponementDelay = 0;
}

unsigned int CallProtocolMethodOperation::GetType() const
{
    return 19;
}

const wchar_t* CallProtocolMethodOperation::GetClassNameString() const
{
    return L"CallProtocolMethod";
}

void CallProtocolMethodOperation::ForceImplOperationCommonMethodsMacro()
{
}

void CallProtocolMethodOperation::PostponeOperation(int delay)
{
    m_postponementDelay = delay;
    m_postponed = true;
}

void CallProtocolMethodOperation::InitializeForNewCallAttempt()
{
    ++m_attemptCount;
    m_postponed = false;
    m_result = qResult(CALL_RESULT_SUCCESS);
}

unsigned int CallProtocolMethodOperation::GetAttemptCount()
{
    return m_attemptCount;
}

void CallProtocolMethodOperation::AttachUserContext(const UserContext& context)
{
    m_callUserContext = context;
}

UserContext CallProtocolMethodOperation::GetUserContext()
{
    return m_callUserContext;
}

}
}

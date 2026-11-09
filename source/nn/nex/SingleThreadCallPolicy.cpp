#include "nn/nex/SingleThreadCallPolicy.hpp"

#include "nn/nex/ObjectThreadRoot.hpp"

namespace nn
{
namespace nex
{

SingleThreadCallPolicy::SingleThreadCallPolicy()
    : m_callDepth(0),
      m_threadId(0),
      m_maximumRecursionLevel(1),
      m_recursionLevel(0)
{
}

void SingleThreadCallPolicy::SetMaximumRecursionLevel(uint32_t maximumRecursionLevel)
{
    m_maximumRecursionLevel = maximumRecursionLevel;
}

bool SingleThreadCallPolicy::CurrentThreadIsDispatchingJobs() const
{
    return m_threadId == ObjectThreadRoot::GetCurrentThreadID();
}

void SingleThreadCallPolicy::BeginCallImpl()
{
    if (m_callDepth == 0)
        m_threadId = ObjectThreadRoot::GetCurrentThreadID();

    ++m_callDepth;
}

void SingleThreadCallPolicy::EndCallImpl()
{
    if (m_callDepth == 1)
        m_threadId = 0;

    --m_callDepth;
}

}
}

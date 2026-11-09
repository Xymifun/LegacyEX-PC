#include "nn/nex/IOCompletionNotifier.hpp"

#include "nn/nex/Socket.hpp"
#include "nn/nex/SocketDriver.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum
{
    TRACE_MASK = 0x80000,
    POLL_READ = 1,
    POLL_WRITE = 2,
    OPERATION_WRITE = 2,
    OPERATION_READ = 3
};

}

IOCompletionNotifier::IOCompletionNotifier() : m_criticalSection(TRACE_MASK)
{
    unsigned int i = 0;
    while (i < MAX_CONTEXTS)
    {
        m_aContexts[i] = 0;
        ++i;
    }
}

IOCompletionNotifier::~IOCompletionNotifier()
{
}

IOCompletionContext* IOCompletionNotifier::CreateIOCompletionContext()
{
    ScopedCS lock(&m_criticalSection);
    unsigned int i = 0;
    while (i < MAX_CONTEXTS && m_aContexts[i] != 0)
        ++i;

    m_aContexts[i] = new IOCompletionContext();
    return m_aContexts[i];
}

void IOCompletionNotifier::DeleteIOCompletionContext(IOCompletionContext* pContext)
{
    ScopedCS lock(&m_criticalSection);
    unsigned int i = 0;
    while (i < MAX_CONTEXTS && m_aContexts[i] != pContext)
        ++i;

    delete m_aContexts[i];
    m_aContexts[i] = 0;
}

bool IOCompletionNotifier::WaitForPollIOCompletion(unsigned int uiTimeout)
{
    SocketDriver::PollInfo aInfo[MAX_CONTEXTS];
    unsigned int aIndices[MAX_CONTEXTS];
    unsigned int uiCount = 0;
    {
        ScopedCS lock(&m_criticalSection);
        unsigned int i = 0;
        while (i < MAX_CONTEXTS)
        {
            IOCompletionContext* pContext = m_aContexts[i];
            if (pContext != 0 && pContext->m_uiState == IOCompletionContext::STATE_PENDING)
            {
                aInfo[uiCount].m_pSocket = pContext->m_pSocket->m_pDriverSocket;
                aIndices[uiCount] = i;
                if (pContext->m_uiOperation == OPERATION_WRITE)
                    aInfo[uiCount].m_uiEvents = POLL_WRITE;

                if (pContext->m_uiOperation == OPERATION_READ)
                    aInfo[uiCount].m_uiEvents = POLL_READ;

                if (pContext->m_uiOperation == IOCompletionContext::OPERATION_CONNECT)
                    aInfo[uiCount].m_uiEvents = POLL_WRITE;

                ++uiCount;
            }

            ++i;
        }
    }

    if (uiCount != 0 && g_socketDriver->Poll(aInfo, uiCount, uiTimeout))
    {
        ScopedCS lock(&m_criticalSection);
        unsigned int i = 0;
        while (i < uiCount)
        {
            IOCompletionContext* pContext = m_aContexts[aIndices[i]];
            if (pContext != 0 && pContext->m_uiState == IOCompletionContext::STATE_PENDING
                && pContext->m_uiOperation != OPERATION_WRITE)
            {
                if (pContext->m_uiOperation == IOCompletionContext::OPERATION_CONNECT
                    && (aInfo[i].m_uiResult & POLL_WRITE) != 0)
                {
                    pContext = m_aContexts[i];
                    pContext->m_uiState = IOCompletionContext::STATE_COMPLETE;
                    pContext->m_unknownC = 0;
                    pContext->m_uiOperation = 0;
                }
            }

            ++i;
        }

        return true;
    }

    return false;
}

bool IOCompletionNotifier::WaitForIOCompletion(unsigned int uiTimeout)
{
    {
        ScopedCS lock(&m_criticalSection);
        unsigned int i = 0;
        while (i < MAX_CONTEXTS)
        {
            IOCompletionContext* pContext = m_aContexts[i];
            if (pContext != 0 && pContext->m_uiState == IOCompletionContext::STATE_COMPLETE)
                return true;

            ++i;
        }
    }

    bool bResult = false;
    if (g_socketDriver != 0)
        bResult = WaitForPollIOCompletion(uiTimeout);

    return bResult;
}

}
}

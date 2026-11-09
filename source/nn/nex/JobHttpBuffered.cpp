#include "nn/nex/JobHttpBuffered.hpp"

#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"

namespace nn
{
namespace nex
{

JobHttpBuffered::JobHttpBuffered() : m_pStatusCode(0)
{
}

JobHttpBuffered::~JobHttpBuffered()
{
}

qResult JobHttpBuffered::Initialize(unsigned int uiCallId, const String& rUrl, HttpConnection::Method method,
                                    unsigned int uiMaxSize, qVector<unsigned char>* pBuffer, int* pStatusCode)
{
    m_pBuffer = pBuffer;
    m_uiMaxSize = uiMaxSize;
    m_pStatusCode = pStatusCode;
    return JobHttp::Initialize(uiCallId, rUrl, method, this, RECEIVE_BUFFER_SIZE);
}

void JobHttpBuffered::ProcessResponseHeader(HttpConnection* pConnection, int iStatusCode)
{
    *m_pStatusCode = iStatusCode;
}

bool JobHttpBuffered::ProcessResponse(const unsigned char* pData, unsigned int uiSize)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0 || m_pBuffer->size() + uiSize > m_uiMaxSize)
    {
        pCallContext->Cancel(CallContext::CallCanceled);
        return false;
    }

    m_pBuffer->insert(m_pBuffer->end(), pData, pData + uiSize);
    return true;
}

}
}

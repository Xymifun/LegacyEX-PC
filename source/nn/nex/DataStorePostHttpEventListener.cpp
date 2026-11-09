#include "nn/nex/DataStorePostHttpEventListener.hpp"

#include <algorithm>
#include <string.h>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/DataStorePostObjectEventListener.hpp"
#include "nn/nex/HttpConnection.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/StringStream.hpp"

namespace nn
{
namespace nex
{

DataStorePostHttpEventListener::DataStorePostHttpEventListener(
    unsigned int uiCallId, const qVector<DataStoreKeyValue>& rHeaders, const qVector<DataStoreKeyValue>& rFormFields,
    const qVector<unsigned char>& rRootCa, unsigned int uiThreadPriority, unsigned int uiTransferBufferSize,
    const unsigned char* pData, unsigned int uiDataSize, DataStorePostObjectEventListener* pListener)
    : m_uiCallId(uiCallId),
      m_rHeaders(rHeaders),
      m_rFormFields(rFormFields),
      m_rRootCa(rRootCa),
      m_uiThreadPriority(uiThreadPriority),
      m_uiTransferBufferSize(uiTransferBufferSize),
      m_pData(pData),
      m_uiDataSize(uiDataSize),
      m_pListener(pListener),
      m_uiPosted(0),
      m_formHeader(),
      m_formFooter(),
      m_bBodySent(false)
{
}

DataStorePostHttpEventListener::~DataStorePostHttpEventListener()
{
}

void DataStorePostHttpEventListener::PrepareRequest(HttpConnection* pConnection)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
        return;

    pConnection->SetThreadPriority(m_uiThreadPriority);
    pConnection->SetTransferBufferSize(m_uiTransferBufferSize);

    for (qVector<DataStoreKeyValue>::const_iterator it = m_rHeaders.begin(); it != m_rHeaders.end(); ++it)
    {
        char* pName;
        char* pValue;
        it->m_Key.CreateCopy(&pName);
        it->m_Value.CreateCopy(&pValue);
        pConnection->AddHeaderField(pName, pValue);
        String::ReleaseCopy(pName);
        String::ReleaseCopy(pValue);
    }

    if (!m_rRootCa.empty())
        pConnection->AddRootCa(&m_rRootCa[0], m_rRootCa.size());

    for (qVector<DataStoreKeyValue>::const_iterator it = m_rFormFields.begin(); it != m_rFormFields.end(); ++it)
    {
        String boundary(pConnection->GetBoundaryString());
        StringStream stream;
        stream << L"--" << boundary << L"\r\n"
               << L"Content-Disposition: form-data; name=\"" << it->m_Key << L"\"" << L"\r\n"
               << L"\r\n"
               << it->m_Value << L"\r\n";
        String text(stream.GetBuffer());
        char* pText;
        text.CreateCopy(&pText);
        m_formHeader.insert(m_formHeader.end(), pText, pText + strlen(pText));
        String::ReleaseCopy(pText);
    }

    {
        String boundary(pConnection->GetBoundaryString());
        StringStream stream;
        stream << L"--" << boundary << L"\r\n"
               << L"Content-Disposition: form-data; name=\"file\"" << L"\r\n"
               << L"\r\n";
        String text(stream.GetBuffer());
        char* pText;
        text.CreateCopy(&pText);
        m_formHeader.insert(m_formHeader.end(), pText, pText + strlen(pText));
        String::ReleaseCopy(pText);
    }

    {
        String boundary(pConnection->GetBoundaryString());
        StringStream stream;
        stream << L"\r\n" << L"--" << boundary << L"--" << L"\r\n";
        String text(stream.GetBuffer());
        char* pText;
        text.CreateCopy(&pText);
        m_formFooter.insert(m_formFooter.end(), pText, pText + strlen(pText));
        String::ReleaseCopy(pText);
    }

    pConnection->UseChunkedPost(m_formHeader.size() + m_uiDataSize + m_formFooter.size());
}

unsigned int DataStorePostHttpEventListener::PostChunkedBuffer(HttpConnection* pConnection, unsigned char* pBuffer,
                                                               unsigned int uiSize, bool* pbLastChunk)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
        return 0;

    unsigned int uiCopied;
    if (!m_bBodySent)
    {
        if (!m_formHeader.empty())
        {
            unsigned int uiAvailable = m_formHeader.size();
            uiCopied = std::min(uiAvailable, uiSize);
            std::copy(m_formHeader.begin(), m_formHeader.begin() + uiCopied, pBuffer);
            m_formHeader.erase(m_formHeader.begin(), m_formHeader.begin() + uiCopied);
            return uiCopied;
        }

        if (m_pData == 0)
        {
            uiCopied = m_pListener->PostChunkedBuffer(pBuffer, uiSize, &m_bBodySent);
        }
        else
        {
            unsigned int uiRemaining = m_uiDataSize - m_uiPosted;
            uiCopied = std::min(uiRemaining, uiSize);
            memcpy(pBuffer, m_pData + m_uiPosted, uiCopied);
            if (m_uiPosted + uiCopied == m_uiDataSize)
                m_bBodySent = true;
        }

        m_uiPosted += uiCopied;
        if (m_uiPosted <= m_uiDataSize)
            return uiCopied;
    }
    else
    {
        unsigned int uiAvailable = m_formFooter.size();
        uiCopied = std::min(uiAvailable, uiSize);
        std::copy(m_formFooter.begin(), m_formFooter.begin() + uiCopied, pBuffer);
        m_formFooter.erase(m_formFooter.begin(), m_formFooter.begin() + uiCopied);
        if (!m_formFooter.empty())
            return uiCopied;

        if (m_uiPosted == m_uiDataSize)
        {
            *pbLastChunk = true;
            return uiCopied;
        }
    }

    pCallContext->Cancel(CallContext::CallCanceled);
    return 0;
}

}
}

#include "nn/nex/HTTPEngineCurlBase.hpp"

#include <cstring>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/HTTPClientImpl.hpp"
#include "nn/nex/HTTPClientRequestData.hpp"
#include "nn/nex/HTTPData.hpp"
#include "nn/nex/HTTPModule.hpp"
#include "nn/nex/HTTPProxySettings.hpp"
#include "nn/nex/HTTPReader.hpp"
#include "nn/nex/HTTPResponse.hpp"
#include "nn/nex/HTTPWriter.hpp"
#include "nn/nex/MemoryManager.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/UserContext.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

namespace
{

CURLSH* s_pShare = 0;
int s_iEngineCount = 0;
bool s_bShareInitialized = false;
CriticalSection s_sharedDataLocks[HTTPEngineCurlBase::SHARED_LOCK_TYPE_COUNT];
CriticalSection s_sharedLock(1);

bool CheckCurlSHError(CURLSHcode code)
{
    return code == CURLSHE_OK;
}

}

HTTPEngineCurlBase::HTTPEngineCurlBase() : m_requestLock(1), m_curlLock(1), m_pMultiHandle(0)
{
    m_nextRequestID = 0;
    m_runningHandles = 0;
}

bool HTTPEngineCurlBase::CheckMultiResult(int iResult)
{
    return iResult == CURLM_OK;
}

HTTPEngineCurlBase::~HTTPEngineCurlBase()
{
    --s_iEngineCount;
    {
        ScopedCS lock(&s_sharedLock);
        if (s_bShareInitialized)
        {
            if (s_iEngineCount == 0)
            {
                CheckCurlSHError(curl_share_setopt(s_pShare, CURLSHOPT_UNSHARE, CURL_LOCK_DATA_DNS));
                curl_share_cleanup(s_pShare);
                s_pShare = 0;
                s_bShareInitialized = false;
            }
        }
        CheckMultiResult(curl_multi_cleanup(m_pMultiHandle));
        m_pMultiHandle = 0;
    }
}

namespace
{

unsigned int HTTPClientWriterCallback(char* pBuffer, unsigned int uiSize, unsigned int uiCount, void* pUserData)
{
    unsigned int uiWritten = 0;
    HTTPWriter* pWriter = static_cast<HTTPWriter*>(pUserData);
    if (pWriter->Write(pBuffer, uiSize * uiCount))
    {
        uiWritten = uiCount;
    }
    return uiWritten;
}

}

unsigned int HTTPEngineCurlBase::CreateGETRequest(const HTTPURL& rURL, const qVector<String>& rHeaders,
                                                  HTTPWriter& rWriter)
{
    ScopedCS lock(&m_requestLock);
    HTTPCurlRequest* pRequest = new HTTPCurlRequest();
    pRequest->Reset(s_pShare, HTTPCurlRequest::HTTP_METHOD_GET, rURL);
    pRequest->AllowRedirection(true);
    pRequest->SetWriter(HTTPClientWriterCallback, &rWriter);
    pRequest->SetHeaders(rHeaders);
    unsigned int uiRequestID = m_nextRequestID++;
    m_curlRequests[uiRequestID] = pRequest;
    return uiRequestID;
}

namespace
{

unsigned int HTTPClientReaderCallback(char* pBuffer, unsigned int uiSize, unsigned int uiCount, void* pUserData)
{
    HTTPData* pData = static_cast<HTTPData*>(pUserData);
    return pData->Read(pBuffer, uiCount * uiSize) / uiSize;
}

}

unsigned int HTTPEngineCurlBase::CreatePOSTRequest(const HTTPURL& rURL, const qVector<String>& rHeaders,
                                                   HTTPData& rData, HTTPWriter& rWriter)
{
    ScopedCS lock(&m_requestLock);
    HTTPCurlRequest* pRequest = new HTTPCurlRequest();
    pRequest->Reset(s_pShare, HTTPCurlRequest::HTTP_METHOD_POST, rURL);
    pRequest->SetReader(HTTPClientReaderCallback, &rData, rData.GetContentLength());
    pRequest->SetWriter(HTTPClientWriterCallback, &rWriter);
    pRequest->SetHeaders(rHeaders);
    unsigned int uiRequestID = m_nextRequestID++;
    m_curlRequests[uiRequestID] = pRequest;
    return uiRequestID;
}

namespace
{

unsigned int HTTPClientPutReaderCallback(char* pBuffer, unsigned int uiSize, unsigned int uiCount, void* pUserData)
{
    HTTPReader* pReader = static_cast<HTTPReader*>(pUserData);
    return pReader->Read(pBuffer, uiCount * uiSize) / uiSize;
}

}

unsigned int HTTPEngineCurlBase::CreatePUTRequest(const HTTPURL& rURL, const qVector<String>& rHeaders,
                                                  HTTPReader& rReader)
{
    ScopedCS lock(&m_requestLock);
    HTTPCurlRequest* pRequest = new HTTPCurlRequest();
    pRequest->Reset(s_pShare, HTTPCurlRequest::HTTP_METHOD_PUT, rURL);
    pRequest->SetReader(HTTPClientPutReaderCallback, &rReader, rReader.GetContentLength());
    pRequest->SetHeaders(rHeaders);
    unsigned int uiRequestID = m_nextRequestID++;
    m_curlRequests[uiRequestID] = pRequest;
    return uiRequestID;
}

int HTTPEngineCurlBase::CurlMultiAddHandle(void* pHandle)
{
    ScopedCS lock(&m_curlLock);
    return curl_multi_add_handle(m_pMultiHandle, pHandle);
}

bool HTTPEngineCurlBase::ScheduleRequest(CallContext& rCallContext, unsigned int uiRequestID, HTTPResponse& rResponse,
                                         HTTPClientImpl* pClient)
{
    ScopedCS schedulerLock(&Scheduler::GetInstance()->GetCriticalSection());
    ScopedCS lock(&m_requestLock);
    HTTPCurlRequest* pRequest = m_curlRequests[uiRequestID];
    if (pRequest == 0)
    {
        return false;
    }
    rCallContext.InitiateCall();
    ApplyConfiguration(uiRequestID);
    HTTPClientRequestData* pRequestData = new HTTPClientRequestData(rCallContext.GetCallID(), pClient, uiRequestID,
                                                                    &rResponse);
    rCallContext.RegisterCompletionCallback(HTTPClientImpl::OnCompletionCallback, UserContext(pRequestData), false);
    m_requests.push_back(pRequestData);
    CheckMultiResult(CurlMultiAddHandle(pRequest->GetHandle()));
    return true;
}

void HTTPEngineCurlBase::DestroyRequest(unsigned int uiRequestID)
{
    ScopedCS lock(&m_requestLock);
    HTTPCurlRequest* pRequest = m_curlRequests[uiRequestID];
    if (pRequest != 0)
    {
        delete pRequest;
        m_curlRequests.erase(uiRequestID);
    }
    for (unsigned int i = 0; i < m_requests.size(); ++i)
    {
        if (m_requests[i]->GetRequestID() == uiRequestID)
        {
            delete m_requests[i];
            m_requests.erase(m_requests.begin() + i);
            break;
        }
    }
}

int HTTPEngineCurlBase::CurlMultiRemoveHandle(void* pHandle)
{
    ScopedCS lock(&m_curlLock);
    return curl_multi_remove_handle(m_pMultiHandle, pHandle);
}

void HTTPEngineCurlBase::CancelRequest(unsigned int uiRequestID)
{
    ScopedCS lock(&m_requestLock);
    for (unsigned int i = 0; i < m_requests.size(); ++i)
    {
        if (m_requests[i]->GetRequestID() == uiRequestID && !m_requests[i]->IsCompleted())
        {
            int iResult = CurlMultiRemoveHandle(m_curlRequests[m_requests[i]->GetRequestID()]->GetHandle());
            m_requests[i]->SetCompleted();
            CheckMultiResult(iResult);
            return;
        }
    }
}

int HTTPEngineCurlBase::CurlMultiPerform()
{
    ScopedCS lock(&m_curlLock);
    return curl_multi_perform(m_pMultiHandle, &m_runningHandles);
}

CURLMsg* HTTPEngineCurlBase::CurlMultiInfoRead(int* pMessagesLeft)
{
    ScopedCS lock(&m_curlLock);
    return curl_multi_info_read(m_pMultiHandle, pMessagesLeft);
}

void HTTPEngineCurlBase::GetInfo()
{
    int iMessagesLeft = 0;
    bool bSuccess = true;
    unsigned int uiCallID = 0;
    LockEngineRequest(true);
    for (;;)
    {
        CURLMsg* pMessage = CurlMultiInfoRead(&iMessagesLeft);
        if (pMessage == 0)
        {
            LockEngineRequest(false);
            return;
        }
        if (pMessage->msg == CURLMSG_DONE)
        {
            if (pMessage->data.result != CURLE_OK)
            {
                bSuccess = false;
            }
            uiCallID = RequestCompleted(pMessage->easy_handle);
        }
        LockEngineRequest(false);
        {
            ScopedCS schedulerLock(&Scheduler::GetInstance()->GetCriticalSection());
            CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(uiCallID);
            if (pCallContext != 0)
            {
                if (bSuccess)
                {
                    pCallContext->SetStateImpl(CallContext::CallSucceeded, qResult(RESULT_SUCCESS), true);
                }
                else
                {
                    pCallContext->SignalFailure(qResult(RESULT_TRANSPORT_SERVER_REQUEST_ERROR));
                }
            }
        }
        LockEngineRequest(true);
        bSuccess = true;
        uiCallID = 0;
    }
}

int HTTPEngineCurlBase::PerformTransfer()
{
    unsigned int uiResult = CurlMultiPerform();
    if (uiResult != CURLM_OK && uiResult != CURLM_CALL_MULTI_PERFORM)
    {
        CheckMultiResult(uiResult);
    }
    GetInfo();
    return uiResult;
}

namespace
{

void* CurlMalloc(unsigned int uiSize)
{
    unsigned int* pBlock = static_cast<unsigned int*>(MemoryManager::Allocate(uiSize + sizeof(unsigned int)));
    *pBlock = uiSize;
    return pBlock + 1;
}

void CurlFree(void* pMemory)
{
    MemoryManager::Free(static_cast<unsigned int*>(pMemory) - 1);
}

void* CurlRealloc(void* pMemory, unsigned int uiSize)
{
    unsigned int uiOldSize = *(static_cast<unsigned int*>(pMemory) - 1);
    void* pNewMemory = CurlMalloc(uiSize);
    memcpy(pNewMemory, pMemory, uiOldSize > uiSize ? uiSize : uiOldSize);
    CurlFree(pMemory);
    return pNewMemory;
}

char* CurlStrDup(const char* pString)
{
    char* pCopy = static_cast<char*>(CurlMalloc(strlen(pString) + 1));
    memcpy(pCopy, pString, strlen(pString) + 1);
    return pCopy;
}

void* CurlCalloc(unsigned int uiCount, unsigned int uiSize)
{
    unsigned int uiTotal = uiCount * uiSize;
    void* pMemory = CurlMalloc(uiTotal);
    memset(pMemory, 0, uiTotal);
    return pMemory;
}

class CurlEngine
{
public:
    CurlEngine()
    {
        curl_global_init_mem(CURL_GLOBAL_NOTHING, CurlMalloc, CurlFree, CurlRealloc, CurlStrDup, CurlCalloc);
    }

    ~CurlEngine() { curl_global_cleanup(); }
};

}

void HTTPEngineCurlBase::LockSharedData(SharedLockType type, bool bLock)
{
    if (bLock)
    {
        s_sharedDataLocks[type].Enter();
    }
    else
    {
        s_sharedDataLocks[type].Leave();
    }
}

namespace
{

void HTTPLockData(curl_lock_data data, void* pUserData, bool bLock)
{
    HTTPEngineCurlBase* pEngine = static_cast<HTTPEngineCurlBase*>(pUserData);
    switch (data)
    {
    case CURL_LOCK_DATA_SHARE:
        pEngine->LockSharedData(HTTPEngineCurlBase::SHARED_LOCK_SHARE, bLock);
        break;
    case CURL_LOCK_DATA_DNS:
        pEngine->LockSharedData(HTTPEngineCurlBase::SHARED_LOCK_DNS, bLock);
        break;
    default:
        pEngine->LockSharedData(HTTPEngineCurlBase::SHARED_LOCK_SHARE, bLock);
        break;
    }
}

void CurlLockFunction(CURL*, curl_lock_data data, curl_lock_access, void* pUserData)
{
    HTTPLockData(data, pUserData, true);
}

void CurlUnlockFunction(CURL*, curl_lock_data data, void* pUserData)
{
    HTTPLockData(data, pUserData, false);
}

}

void HTTPEngineCurlBase::Initialize()
{
    ScopedCS lock(&s_sharedLock);
    if (!s_bShareInitialized)
    {
        ++s_iEngineCount;
        s_bShareInitialized = true;
        static CurlEngine s_curlEngine;
        s_pShare = curl_share_init();
        CheckCurlSHError(curl_share_setopt(s_pShare, CURLSHOPT_USERDATA, this));
        CheckCurlSHError(curl_share_setopt(s_pShare, CURLSHOPT_LOCKFUNC, CurlLockFunction));
        CheckCurlSHError(curl_share_setopt(s_pShare, CURLSHOPT_UNLOCKFUNC, CurlUnlockFunction));
        CheckCurlSHError(curl_share_setopt(s_pShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS));
    }
    m_pMultiHandle = curl_multi_init();
}

int HTTPEngineCurlBase::GetCurlTransferInfo(void* pHandle, unsigned int uiInfo, double* pValue)
{
    ScopedCS lock(&m_curlLock);
    return curl_easy_getinfo(pHandle, static_cast<CURLINFO>(uiInfo), pValue);
}

HTTPEngine::TransferProgress HTTPEngineCurlBase::GetDownloadProgress(CallContext* pCallContext,
                                                                     unsigned long long& rCurrent,
                                                                     unsigned long long& rTotal)
{
    double dCurrent = 0.0;
    double dTotal = 0.0;
    TransferProgress progress = TRANSFER_REQUEST_NOT_FOUND;
    ScopedCS lock(&m_requestLock);
    for (unsigned int i = 0; i < m_requests.size(); ++i)
    {
        if (m_requests[i]->GetCallID() == pCallContext->GetCallID())
        {
            GetCurlTransferInfo(m_curlRequests[m_requests[i]->GetRequestID()]->GetHandle(),
                                CURLINFO_CONTENT_LENGTH_DOWNLOAD, &dTotal);
            GetCurlTransferInfo(m_curlRequests[m_requests[i]->GetRequestID()]->GetHandle(), CURLINFO_SIZE_DOWNLOAD,
                                &dCurrent);
            progress = dCurrent > 0.0 ? TRANSFER_STARTED : TRANSFER_NOT_STARTED;
            break;
        }
    }
    if (dTotal < 0.0)
    {
        dTotal = 0.0;
    }
    if (dCurrent < 0.0)
    {
        dCurrent = 0.0;
    }
    rTotal = static_cast<unsigned long long>(dTotal);
    rCurrent = static_cast<unsigned long long>(dCurrent);
    return progress;
}

HTTPEngine::TransferProgress HTTPEngineCurlBase::GetUploadProgress(CallContext* pCallContext,
                                                                   unsigned long long& rCurrent,
                                                                   unsigned long long& rTotal)
{
    double dCurrent = 0.0;
    double dTotal = 0.0;
    TransferProgress progress = TRANSFER_REQUEST_NOT_FOUND;
    ScopedCS lock(&m_requestLock);
    for (unsigned int i = 0; i < m_requests.size(); ++i)
    {
        if (m_requests[i]->GetCallID() == pCallContext->GetCallID())
        {
            GetCurlTransferInfo(m_curlRequests[m_requests[i]->GetRequestID()]->GetHandle(),
                                CURLINFO_CONTENT_LENGTH_UPLOAD, &dTotal);
            GetCurlTransferInfo(m_curlRequests[m_requests[i]->GetRequestID()]->GetHandle(), CURLINFO_SIZE_UPLOAD,
                                &dCurrent);
            progress = dCurrent > 0.0 ? TRANSFER_STARTED : TRANSFER_NOT_STARTED;
            break;
        }
    }
    if (dTotal < 0.0)
    {
        dTotal = 0.0;
    }
    if (dCurrent < 0.0)
    {
        dCurrent = 0.0;
    }
    rTotal = static_cast<unsigned long long>(dTotal);
    rCurrent = static_cast<unsigned long long>(dCurrent);
    return progress;
}

void HTTPEngineCurlBase::ApplyConfiguration(unsigned int uiRequestID)
{
    HTTPCurlRequest* pRequest = m_curlRequests[uiRequestID];
    pRequest->SetTransferBufferSize(m_pConfiguration->GetEngineTransferBufferSize());
    if (HTTPModule::GetInstance()->GetProxyName().GetLength() != 0)
    {
        HTTPProxySettings proxySettings(
            HTTPModule::GetInstance()->GetProxyName(), HTTPModule::GetInstance()->GetProxyPort(),
            HTTPModule::GetInstance()->GetProxyUsername(), HTTPModule::GetInstance()->GetProxyPassword());
        pRequest->SetProxy(proxySettings);
    }
}

unsigned int HTTPEngineCurlBase::RequestCompleted(void* pHandle)
{
    CheckMultiResult(CurlMultiRemoveHandle(pHandle));
    for (unsigned int i = 0; i < m_requests.size(); ++i)
    {
        if (m_curlRequests[m_requests[i]->GetRequestID()]->GetHandle() == pHandle)
        {
            HTTPClientRequestData* pRequestData = m_requests[i];
            pRequestData->GetResponse()->OnResponseReceived(
                m_curlRequests[pRequestData->GetRequestID()]->GetResponseCode(),
                m_curlRequests[pRequestData->GetRequestID()]->GetHeaders());
            pRequestData->SetCompleted();
            return pRequestData->GetCallID();
        }
    }
    return 0;
}

}
}

#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPENGINECURLBASE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPENGINECURLBASE_HPP

#include <map>

#include <curl/curl.h>

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/HTTPEngine.hpp"
#include "nn/nex/HTTPCurlRequest.hpp"
#include "nn/nex/MemAllocator.hpp"

namespace nn
{
namespace nex
{

class HTTPEngineCurlBase : public HTTPEngine
{
public:
    HTTPEngineCurlBase();
    virtual ~HTTPEngineCurlBase();

    virtual void Initialize();
    virtual unsigned int CreateGETRequest(const HTTPURL& rURL, const qVector<String>& rHeaders, HTTPWriter& rWriter);
    virtual unsigned int CreatePOSTRequest(const HTTPURL& rURL, const qVector<String>& rHeaders, HTTPData& rData,
                                           HTTPWriter& rWriter);
    virtual unsigned int CreatePUTRequest(const HTTPURL& rURL, const qVector<String>& rHeaders, HTTPReader& rReader);
    virtual bool ScheduleRequest(CallContext& rCallContext, unsigned int uiRequestID, HTTPResponse& rResponse,
                                 HTTPClientImpl* pClient);
    virtual void DestroyRequest(unsigned int uiRequestID);
    virtual void CancelRequest(unsigned int uiRequestID);
    virtual TransferProgress GetDownloadProgress(CallContext* pCallContext, unsigned long long& rCurrent,
                                                 unsigned long long& rTotal);
    virtual TransferProgress GetUploadProgress(CallContext* pCallContext, unsigned long long& rCurrent,
                                               unsigned long long& rTotal);
    virtual void ApplyConfiguration(unsigned int uiRequestID);
    virtual unsigned int RequestCompleted(void* pHandle);

    enum SharedLockType
    {
        SHARED_LOCK_SHARE = 0,
        SHARED_LOCK_DNS = 1,
        SHARED_LOCK_COOKIE = 2,
        SHARED_LOCK_TYPE_COUNT = 3
    };

    int PerformTransfer();
    void LockSharedData(SharedLockType type, bool bLock);

protected:
    enum Result
    {
        RESULT_SUCCESS = 0x10001,
        RESULT_TRANSPORT_SERVER_REQUEST_ERROR = static_cast<int>(0x8005000Bu)
    };

    bool CheckMultiResult(int iResult);
    int CurlMultiAddHandle(void* pHandle);
    int CurlMultiRemoveHandle(void* pHandle);
    int CurlMultiPerform();
    CURLMsg* CurlMultiInfoRead(int* pMessagesLeft);
    int GetCurlTransferInfo(void* pHandle, unsigned int uiInfo, double* pValue);
    void GetInfo();

    void LockEngineRequest(bool bLock)
    {
        CriticalSection* pLock = &m_requestLock;
        if (bLock)
        {
            pLock->Enter();
        }
        else
        {
            pLock->Leave();
        }
    }

private:
    typedef std::map<unsigned int, HTTPCurlRequest*, std::less<unsigned int>,
                     MemAllocator<std::pair<const unsigned int, HTTPCurlRequest*> > >
        RequestMap;

    CriticalSection m_requestLock;
    CriticalSection m_curlLock;
    CURLM* m_pMultiHandle;
    RequestMap m_curlRequests;
    unsigned int m_nextRequestID;
    int m_runningHandles;
};

}
}

#endif

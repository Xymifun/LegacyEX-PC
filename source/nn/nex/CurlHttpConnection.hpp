#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CURLHTTPCONNECTION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CURLHTTPCONNECTION_HPP

#include <stddef.h>

#include <curl/curl.h>

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/HttpConnection.hpp"
#include "nn/nex/MemoryManager.hpp"
#include "nn/nex/ObjectThread.hpp"
#include "nn/nex/StringStream.hpp"
#include "nn/nex/qReservedQueue.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class CurlHttpConnection : public HttpConnection
{
public:
    class FixedBufferPool : public RootObject
    {
    public:
        explicit FixedBufferPool(unsigned int uiCapacity) : m_buffer(uiCapacity) {}

        virtual ~FixedBufferPool() {}

        char* Add(const void* pData, unsigned int uiSize);

    private:
        qReservedQueue<unsigned char> m_buffer;
    };

    CurlHttpConnection();
    virtual ~CurlHttpConnection();

    virtual qResult Initialize(const char* pUrl, Method method);
    virtual qResult AddHeaderField(const char* pName, const char* pValue);
    virtual qResult AddPostData(const char* pName, const char* pValue);
    virtual void UseChunkedPost(unsigned int uiContentLength);
    virtual void CompleteChunkedPost();
    virtual bool IsCompleteChunkedPost() const;
    virtual qResult ConnectAsync();
    virtual qResult Cancel();
    virtual bool GetStatusCode(int* pStatusCode) const;
    virtual qResult Read(unsigned char* pBuffer, unsigned int* pReadSize, unsigned int uiCapacity);
    virtual qResult WritePostChunk(unsigned char* pData, unsigned int uiSize);
    virtual unsigned int GetWritableSize() const;
    virtual qResult GetError() const;
    virtual qResult IsReceivingHeaderCompleted() const;
    virtual bool IsFinished() const;
    virtual qResult AddFormData(const char* pName, const void* pData, unsigned int uiSize);
    virtual bool GetResponseHeader(const char* pName, String* pValue) const;
    virtual bool AddRootCa(const unsigned char* pCertificate, int iSize);
    virtual void SetTransferBufferSize(unsigned int uiSize);

    size_t ReadCallback(void* pBuffer, unsigned int uiSize, unsigned int uiCount);
    int HeaderCallback(void* pBuffer, unsigned int uiSize, unsigned int uiCount);
    size_t WriteCallback(void* pBuffer, unsigned int uiSize, unsigned int uiCount);
    int ProgressCallback(double dDownloadTotal, double dDownloaded, double dUploadTotal, double dUploaded);

    static size_t StaticReadCallback(void* pBuffer, unsigned int uiSize, unsigned int uiCount, void* pUser);
    static size_t StaticHeaderCallback(void* pBuffer, unsigned int uiSize, unsigned int uiCount, void* pUser);
    static size_t StaticWriteCallback(void* pBuffer, unsigned int uiSize, unsigned int uiCount, void* pUser);
    static int StaticProgressCallback(void* pUser, double dDownloadTotal, double dDownloaded, double dUploadTotal,
                                      double dUploaded);

    void SetupCurlParameter();
    void RunThread(void* pParameter);

private:
    class BlockList : public RootObject
    {
    public:
        ~BlockList()
        {
            for (qVector<void*>::iterator it = m_blocks.begin(); it != m_blocks.end(); ++it)
                MemoryManager::Free(*it);
        }

    private:
        qVector<void*> m_blocks;
    };

    StringStream m_postData;
    char* m_pPostFields;
    curl_slist* m_pRequestHeaders;
    int m_iStatusCode;
    CURL* m_pCurl;
    ObjectThread<CurlHttpConnection, void*>* m_pThread;
    qResult m_result;
    mutable ForcedCriticalSection m_criticalSection;
    bool m_bFinished;
    bool m_bCancelled;
    bool m_bResponseStarted;
    bool m_bPostBodyComplete;
    bool m_bPostCompleted;
    unsigned int m_uiContentLength;
    bool m_bExpectHeader;
    bool m_bSecure;
    qReservedQueue<unsigned char>* m_pTransferQueue;
    curl_httppost* m_pFormPost;
    curl_httppost* m_pFormPostLast;
    BlockList m_blockList;
    unsigned int m_uiTransferBufferSize;
    FixedBufferPool m_headerPool;
    qReservedQueue<std::pair<char*, char*> > m_headers;
};

}
}

#endif

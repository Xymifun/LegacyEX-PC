#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPCONNECTION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPCONNECTION_HPP

#include <string.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class CriticalSection;
class String;

class HttpConnection : public RootObject
{
public:
    enum Method
    {
        HTTP_GET = 0,
        HTTP_POST = 2
    };

    enum
    {
        BOUNDARY_CAPACITY = 41
    };

    virtual ~HttpConnection() {}

    virtual qResult Initialize(const char* pUrl, Method method) = 0;
    virtual qResult AddHeaderField(const char* pName, const char* pValue) = 0;
    virtual qResult AddPostData(const char* pName, const char* pValue) = 0;
    virtual void UseChunkedPost(unsigned int uiContentLength) = 0;
    virtual bool IsUseChunkedPost() const { return m_bUseChunkedPost; }

    virtual void CompleteChunkedPost() = 0;
    virtual bool IsCompleteChunkedPost() const = 0;
    virtual qResult ConnectAsync() = 0;
    virtual qResult Cancel() = 0;
    virtual bool GetStatusCode(int* pStatusCode) const = 0;
    virtual qResult Read(unsigned char* pBuffer, unsigned int* pReadSize, unsigned int uiCapacity) = 0;
    virtual qResult WritePostChunk(unsigned char* pData, unsigned int uiSize) = 0;
    virtual unsigned int GetWritableSize() const = 0;
    virtual qResult GetError() const = 0;
    virtual qResult IsReceivingHeaderCompleted() const = 0;
    virtual bool IsFinished() const = 0;
    virtual qResult AddFormData(const char* pName, const void* pData, unsigned int uiSize) = 0;
    virtual bool GetResponseHeader(const char* pName, String* pValue) const = 0;
    virtual void NoMoreResponseHeader() {}

    virtual bool AddRootCa(const unsigned char* pCertificate, int iSize) = 0;
    virtual void SetTransferBufferSize(unsigned int uiSize) = 0;

    const char* GetBoundaryString() const;

    void SetThreadPriority(unsigned int uiPriority) { m_uiThreadPriority = uiPriority; }

    void SetUnknown9(bool bValue) { m_bUnknown9 = bValue; }

    void SetUnknown10(bool bValue) { m_bUnknown10 = bValue; }

    static HttpConnection* GetHttpConnection();
    static void ReleaseHttpConnection(HttpConnection* pConnection);
    static int GetConnectionCount();

protected:
    HttpConnection()
        : m_method(HTTP_POST),
          m_uiThreadPriority(16),
          m_bUseChunkedPost(false),
          m_bUnknown9(true),
          m_bUnknown10(true)
    {
        memset(m_aBoundary, 0, sizeof(m_aBoundary));
    }

    Method m_method;
    unsigned int m_uiThreadPriority;
    bool m_bUseChunkedPost;
    bool m_bUnknown9;
    bool m_bUnknown10;
    mutable char m_aBoundary[BOUNDARY_CAPACITY];

private:
    static int s_iConnectionCount;
    static CriticalSection* s_pCriticalSection;
};

}
}

#endif

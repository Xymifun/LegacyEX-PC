#include "nn/nex/CurlHttpConnection.hpp"

#include <algorithm>
#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <vector>

#include <nsysnet/nssl.h>

#include "nn/nex/EqualFirst.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/MemoryManager.hpp"
#include "nn/nex/Platform.hpp"
#include "nn/nex/String.hpp"

extern "C"
{
CURL* mw_curl_easy_init(void);
NSSLError NSSLAddServerPKIGroups(NSSLContextHandle context, int iGroups, int* pFirst, int* pSecond);
}

namespace
{

enum
{
    SOCKET_BUFFER_SIZE = 0xFFFF,
    SOCKET_OPTION_ENABLED = 1
};

}

void ToLower(char* pString)
{
    for (; *pString != 0; ++pString)
        *pString += (static_cast<unsigned int>(*pString - 'A') <= 'Z' - 'A') * ('a' - 'A');
}

void StaticSocketOptCallback(void* pUser, int iSocket, curlsocktype purpose)
{
    int iSendBufferSize = SOCKET_BUFFER_SIZE;
    if (setsockopt(iSocket, SOL_SOCKET, SO_SNDBUF, &iSendBufferSize, sizeof(iSendBufferSize)) == 0)
    {
        int iReceiveBufferSize = SOCKET_BUFFER_SIZE;
        if (setsockopt(iSocket, SOL_SOCKET, SO_RCVBUF, &iReceiveBufferSize, sizeof(iReceiveBufferSize)) == 0)
        {
            int iEnabled = SOCKET_OPTION_ENABLED;
            setsockopt(iSocket, SOL_SOCKET, SO_TCPSACK, &iEnabled, sizeof(iEnabled));
        }
    }
}

namespace nn
{
namespace nex
{

namespace
{

enum
{
    RESULT_SUCCESS = 0x10001,
    RESULT_FAILURE = static_cast<int>(0x80010001u),
    RESULT_FORM_DATA_FAILURE = static_cast<int>(0x80010002u),
    RESULT_INVALID_BUFFER_SIZE = static_cast<int>(0x8001000au),
    RESULT_HTTP_FAILURE = static_cast<int>(0x80050001u),
    RESULT_HTTP_IO_FAILURE = static_cast<int>(0x80050009u),
    RESULT_HTTP_RESOLVE_HOST_FAILURE = static_cast<int>(0x80050014u),
    RESULT_HTTP_RESOLVE_PROXY_FAILURE = static_cast<int>(0x80050015u)
};

enum
{
    MINIMUM_TRANSFER_BUFFER_SIZE = 0x4000,
    DEFAULT_TRANSFER_BUFFER_SIZE = 0x4000,
    HEADER_POOL_CAPACITY = 0x2000,
    HEADER_QUEUE_CAPACITY = 0x20,
    HEADER_LINE_CAPACITY = 128,
    TRANSFER_RETRY_COUNT = 100,
    TRANSFER_RETRY_MILLISECONDS = 10,
    THREAD_STACK_SIZE = 0x8000,
    LOW_SPEED_LIMIT = 1,
    LOW_SPEED_TIME = 120,
    PKI_GROUPS = 3,
    CURL_ERROR_COULDNT_RESOLVE_PROXY = 5,
    CURL_ERROR_COULDNT_RESOLVE_HOST = 6,
    CURL_ERROR_SEND = 55,
    CURL_ERROR_RECV = 56
};

const CURLoption OPTION_NSSL_CONTEXT = static_cast<CURLoption>(210);
const CURLoption OPTION_NSSL_PKI_GROUPS = static_cast<CURLoption>(211);
const unsigned int LOCK_TRACE_MASK = 0x20000000;

char s_szProxy[256];
int s_iNsslContext = NSSL_ERROR_INVALID_NSSL_CONTEXT;
int s_iNsslContextUsers = 0;
ForcedCriticalSection s_nsslLock(LOCK_TRACE_MASK);

}

char* CurlHttpConnection::FixedBufferPool::Add(const void* pData, unsigned int uiSize)
{
    if (uiSize == 0)
        uiSize = 1;
    if (m_buffer.capacity() - m_buffer.size() < uiSize)
        return 0;
    m_buffer.push(static_cast<const unsigned char*>(pData), uiSize);
    return reinterpret_cast<char*>(&m_buffer[m_buffer.size() - uiSize]);
}

CurlHttpConnection::CurlHttpConnection()
    : m_postData(),
      m_pPostFields(0),
      m_pRequestHeaders(0),
      m_iStatusCode(0),
      m_pCurl(0),
      m_pThread(0),
      m_result(true),
      m_criticalSection(LOCK_TRACE_MASK),
      m_bFinished(false),
      m_bCancelled(false),
      m_bResponseStarted(false),
      m_bPostBodyComplete(true),
      m_bPostCompleted(true),
      m_uiContentLength(0),
      m_bExpectHeader(false),
      m_bSecure(false),
      m_pTransferQueue(0),
      m_pFormPost(0),
      m_pFormPostLast(0),
      m_blockList(),
      m_uiTransferBufferSize(DEFAULT_TRANSFER_BUFFER_SIZE),
      m_headerPool(HEADER_POOL_CAPACITY),
      m_headers(HEADER_QUEUE_CAPACITY)
{
}

CurlHttpConnection::~CurlHttpConnection()
{
    CurlHttpConnection::Cancel();
    if (m_pThread)
    {
        delete m_pThread;
        m_pThread = 0;
    }
    if (m_pCurl)
    {
        curl_easy_cleanup(m_pCurl);
        m_pCurl = 0;
    }
    if (m_pRequestHeaders)
    {
        curl_slist_free_all(m_pRequestHeaders);
        m_pRequestHeaders = 0;
    }
    m_postData.Clear();
    if (m_pTransferQueue)
    {
        delete m_pTransferQueue;
        m_pTransferQueue = 0;
    }
    if (m_pPostFields)
    {
        String::ReleaseCopy(m_pPostFields);
        m_pPostFields = 0;
    }
    if (m_pFormPost)
    {
        curl_formfree(m_pFormPost);
        m_pFormPost = 0;
    }
}

qResult CurlHttpConnection::Cancel()
{
    bool bWait = false;
    {
        ScopedCS lock(&m_criticalSection);
        m_bCancelled = true;
        if (m_pThread && m_pThread->IsLaunched())
            bWait = true;
    }
    if (bWait)
        m_pThread->Wait(0xFFFFFFFF);
    return qResult(RESULT_SUCCESS);
}

qResult CurlHttpConnection::Initialize(const char* pUrl, Method method)
{
    ScopedCS lock(&m_criticalSection);
    m_method = method;
    m_pCurl = mw_curl_easy_init();
    if (!m_pCurl)
        return qResult(RESULT_FAILURE);
    curl_easy_setopt(m_pCurl, CURLOPT_URL, pUrl);
    if (strcmp(s_szProxy, "") != 0)
        curl_easy_setopt(m_pCurl, CURLOPT_PROXY, s_szProxy);
    curl_easy_setopt(m_pCurl, CURLOPT_FOLLOWLOCATION, 1);
    if (strncmp(pUrl, "https://", 8) == 0)
        m_bSecure = true;
    return qResult(RESULT_SUCCESS);
}

qResult CurlHttpConnection::AddHeaderField(const char* pName, const char* pValue)
{
    ScopedCS lock(&m_criticalSection);
    String name(pName);
    String value(pValue);
    StringStream line;
    line << name << L": " << value;
    if (strcmp(pName, "expect") == 0)
        m_bExpectHeader = true;
    String header(line.GetBuffer());
    char* pHeader = 0;
    header.CreateCopy(&pHeader);
    m_pRequestHeaders = curl_slist_append(m_pRequestHeaders, pHeader);
    String::ReleaseCopy(pHeader);
    return qResult(RESULT_SUCCESS);
}

qResult CurlHttpConnection::AddPostData(const char* pName, const char* pValue)
{
    ScopedCS lock(&m_criticalSection);
    if (m_pFormPost)
        return qResult(RESULT_FAILURE);
    if (m_postData.GetLength() != 0)
        m_postData << L"&";
    String name(pName);
    String value(pValue);
    m_postData << name << L"=" << value;
    return qResult(RESULT_SUCCESS);
}

void CurlHttpConnection::UseChunkedPost(unsigned int uiContentLength)
{
    ScopedCS lock(&m_criticalSection);
    m_bUseChunkedPost = true;
    m_uiContentLength = uiContentLength;
}

void CurlHttpConnection::CompleteChunkedPost()
{
    ScopedCS lock(&m_criticalSection);
    m_bPostBodyComplete = true;
}

bool CurlHttpConnection::IsCompleteChunkedPost() const
{
    return m_bPostCompleted;
}

size_t CurlHttpConnection::ReadCallback(void* pBuffer, unsigned int uiSize, unsigned int uiCount)
{
    unsigned int uiRequested = uiSize * uiCount;
    {
        ScopedCS lock(&m_criticalSection);
        if (m_bCancelled)
            return uiRequested;
        if (m_bPostBodyComplete && m_pTransferQueue->size() == 0)
        {
            m_bPostCompleted = true;
            return 0;
        }
    }
    unsigned int uiAttempt = 0;
    do
    {
        {
            ScopedCS lock(&m_criticalSection);
            if (m_pTransferQueue->size() != 0)
                break;
        }
        Platform::Sleep(TRANSFER_RETRY_MILLISECONDS);
        ++uiAttempt;
    } while (uiAttempt < TRANSFER_RETRY_COUNT);
    ScopedCS lock(&m_criticalSection);
    unsigned int uiAvailable = std::min(static_cast<unsigned int>(m_pTransferQueue->size()), uiRequested);
    if (uiAvailable == 0)
        return CURL_READFUNC_PAUSE;
    m_pTransferQueue->front(static_cast<unsigned char*>(pBuffer), uiAvailable);
    m_pTransferQueue->pop(uiAvailable);
    return uiAvailable;
}

size_t CurlHttpConnection::StaticReadCallback(void* pBuffer, unsigned int uiSize, unsigned int uiCount, void* pUser)
{
    return static_cast<CurlHttpConnection*>(pUser)->ReadCallback(pBuffer, uiSize, uiCount);
}

int CurlHttpConnection::HeaderCallback(void* pBuffer, unsigned int uiSize, unsigned int uiCount)
{
    ScopedCS lock(&m_criticalSection);
    unsigned int uiTotal = uiSize * uiCount;
    if (m_bCancelled)
        return uiTotal;
    char* pLine = static_cast<char*>(pBuffer);
    if (strncmp(pLine, "HTTP/", 5) == 0)
    {
        char* pSpace = strchr(pLine, ' ');
        if (pSpace)
            m_iStatusCode = atoi(pSpace + 1);
    }
    char* pColon = strstr(pLine, ":");
    if (!pColon)
        return uiTotal;
    char* pName = m_headerPool.Add(pLine, pColon - pLine);
    if (!pName || !m_headerPool.Add("", 1))
        return uiTotal;
    ToLower(pName);
    char* pValue = pColon + strcspn(pColon, ": ") + 2;
    if (!pValue)
        return uiTotal;
    size_t uiLength = strlen(pValue);
    const char* pTerminators = "\r\n";
    size_t uiTerminators = strlen(pTerminators);
    for (unsigned int i = 0; i < uiLength; ++i)
    {
        for (unsigned int j = 0; j < uiTerminators; ++j)
        {
            if (pValue[i] == pTerminators[j])
            {
                char* pText = m_headerPool.Add(pValue, i);
                if (!pText || !m_headerPool.Add("", 1))
                    return uiTotal;
                if (m_headers.size() == m_headers.capacity())
                    return uiTotal;
                m_headers.push(std::make_pair(pName, pText));
                return uiTotal;
            }
        }
    }
    return uiTotal;
}

size_t CurlHttpConnection::StaticHeaderCallback(void* pBuffer, unsigned int uiSize, unsigned int uiCount, void* pUser)
{
    return static_cast<CurlHttpConnection*>(pUser)->HeaderCallback(pBuffer, uiSize, uiCount);
}

size_t CurlHttpConnection::WriteCallback(void* pBuffer, unsigned int uiSize, unsigned int uiCount)
{
    unsigned int uiTotal = uiSize * uiCount;
    {
        ScopedCS lock(&m_criticalSection);
        if (m_bCancelled)
            return uiTotal;
        if (!m_bPostCompleted)
            return CURL_WRITEFUNC_PAUSE;
        m_bResponseStarted = true;
    }
    unsigned int uiAttempt = 0;
    while (true)
    {
        {
            ScopedCS lock(&m_criticalSection);
            if (m_pTransferQueue->capacity() - m_pTransferQueue->size() >= uiTotal)
                break;
            if (uiAttempt >= TRANSFER_RETRY_COUNT)
                return CURL_WRITEFUNC_PAUSE;
        }
        Platform::Sleep(TRANSFER_RETRY_MILLISECONDS);
        ++uiAttempt;
    }
    ScopedCS lock(&m_criticalSection);
    m_pTransferQueue->push(static_cast<const unsigned char*>(pBuffer), uiTotal);
    return uiTotal;
}

size_t CurlHttpConnection::StaticWriteCallback(void* pBuffer, unsigned int uiSize, unsigned int uiCount, void* pUser)
{
    return static_cast<CurlHttpConnection*>(pUser)->WriteCallback(pBuffer, uiSize, uiCount);
}

int CurlHttpConnection::ProgressCallback(double dDownloadTotal, double dDownloaded, double dUploadTotal,
                                         double dUploaded)
{
    ScopedCS lock(&m_criticalSection);
    curl_easy_pause(m_pCurl, CURLPAUSE_CONT);
    if (m_bCancelled)
        return 1;
    return 0;
}

int CurlHttpConnection::StaticProgressCallback(void* pUser, double dDownloadTotal, double dDownloaded,
                                               double dUploadTotal, double dUploaded)
{
    return static_cast<CurlHttpConnection*>(pUser)->ProgressCallback(dDownloadTotal, dDownloaded, dUploadTotal,
                                                                     dUploaded);
}

void CurlHttpConnection::SetupCurlParameter()
{
    ScopedCS lock(&m_criticalSection);
    if (m_method == HTTP_POST)
    {
        curl_easy_setopt(m_pCurl, CURLOPT_POST, -1);
        if (m_bUseChunkedPost)
        {
            if (!m_bExpectHeader)
                m_pRequestHeaders = curl_slist_append(m_pRequestHeaders, "expect:");
            char acHeader[HEADER_LINE_CAPACITY];
            sprintf(acHeader, "Content-Length: %d", m_uiContentLength);
            m_pRequestHeaders = curl_slist_append(m_pRequestHeaders, acHeader);
            sprintf(acHeader, "Content-Type: multipart/form-data; boundary=%s", GetBoundaryString());
            m_pRequestHeaders = curl_slist_append(m_pRequestHeaders, acHeader);
            curl_easy_setopt(m_pCurl, CURLOPT_READFUNCTION, StaticReadCallback);
            curl_easy_setopt(m_pCurl, CURLOPT_READDATA, this);
            m_bPostCompleted = false;
            m_bPostBodyComplete = false;
        }
        else if (m_postData.GetLength() != 0)
        {
            String body(m_postData.GetBuffer());
            body.CreateCopy(&m_pPostFields);
            curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, m_pPostFields);
            curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDSIZE, m_postData.GetLength());
        }
        else
        {
            curl_easy_setopt(m_pCurl, CURLOPT_HTTPPOST, m_pFormPost);
        }
    }
    curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, m_pRequestHeaders);
    curl_easy_setopt(m_pCurl, CURLOPT_HEADERFUNCTION, StaticHeaderCallback);
    curl_easy_setopt(m_pCurl, CURLOPT_HEADERDATA, this);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, StaticWriteCallback);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(m_pCurl, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(m_pCurl, CURLOPT_PROGRESSFUNCTION, StaticProgressCallback);
    curl_easy_setopt(m_pCurl, CURLOPT_PROGRESSDATA, this);
    curl_easy_setopt(m_pCurl, CURLOPT_SOCKOPTFUNCTION, StaticSocketOptCallback);
    curl_easy_setopt(m_pCurl, CURLOPT_LOW_SPEED_LIMIT, LOW_SPEED_LIMIT);
    curl_easy_setopt(m_pCurl, CURLOPT_LOW_SPEED_TIME, LOW_SPEED_TIME);
}

qResult CurlHttpConnection::ConnectAsync()
{
    ScopedCS lock(&m_criticalSection);
    unsigned int uiBufferSize = m_uiTransferBufferSize;
    if (uiBufferSize < MINIMUM_TRANSFER_BUFFER_SIZE)
        return qResult(RESULT_INVALID_BUFFER_SIZE);
    m_pTransferQueue = new qReservedQueue<unsigned char>(uiBufferSize);
    SetupCurlParameter();
    m_pThread = new ObjectThread<CurlHttpConnection, void*>(String(L"CurlHttpConnection Thread"), m_uiThreadPriority);
    m_pThread->SetObjectMethod(&CurlHttpConnection::RunThread, this, 0);
    m_pThread->Launch(THREAD_STACK_SIZE);
    return qResult(RESULT_SUCCESS);
}

bool CurlHttpConnection::GetStatusCode(int* pStatusCode) const
{
    ScopedCS lock(&m_criticalSection);
    qResult result = IsReceivingHeaderCompleted();
    if (!result)
        return false;
    *pStatusCode = m_iStatusCode;
    return true;
}

qResult CurlHttpConnection::Read(unsigned char* pBuffer, unsigned int* pReadSize, unsigned int uiCapacity)
{
    ScopedCS lock(&m_criticalSection);
    qReservedQueue<unsigned char>* pQueue = m_pTransferQueue;
    unsigned int uiAvailable = std::min(uiCapacity, static_cast<unsigned int>(pQueue->size()));
    if (!m_result)
    {
        return m_result;
    }
    if (uiAvailable != 0)
    {
        pQueue->front(pBuffer, uiAvailable);
        m_pTransferQueue->pop(uiAvailable);
    }
    *pReadSize = uiAvailable;
    return qResult(RESULT_SUCCESS);
}

qResult CurlHttpConnection::WritePostChunk(unsigned char* pData, unsigned int uiSize)
{
    ScopedCS lock(&m_criticalSection);
    if (!m_result)
    {
        return m_result;
    }
    if (uiSize == 0)
        return qResult(RESULT_SUCCESS);
    if (GetWritableSize() < uiSize)
        return qResult(RESULT_HTTP_FAILURE);
    m_pTransferQueue->push(pData, uiSize);
    return qResult(RESULT_SUCCESS);
}

unsigned int CurlHttpConnection::GetWritableSize() const
{
    ScopedCS lock(&m_criticalSection);
    if (!m_pTransferQueue)
        return 0;
    return m_pTransferQueue->capacity() - m_pTransferQueue->size();
}

qResult CurlHttpConnection::GetError() const
{
    ScopedCS lock(&m_criticalSection);
    return m_result;
}

qResult CurlHttpConnection::IsReceivingHeaderCompleted() const
{
    ScopedCS lock(&m_criticalSection);
    if (!m_result)
    {
        return m_result;
    }
    return m_bResponseStarted || m_bFinished ? qResult(RESULT_SUCCESS) : qResult(RESULT_FAILURE);
}

bool CurlHttpConnection::IsFinished() const
{
    ScopedCS lock(&m_criticalSection);
    return m_bFinished && !m_result.IsError() && m_pTransferQueue->size() == 0;
}

qResult CurlHttpConnection::AddFormData(const char* pName, const void* pData, unsigned int uiSize)
{
    ScopedCS lock(&m_criticalSection);
    if (m_postData.GetLength() != 0)
        return qResult(RESULT_FAILURE);
    return qResult(RESULT_FORM_DATA_FAILURE);
}

bool CurlHttpConnection::GetResponseHeader(const char* pName, String* pValue) const
{
    ScopedCS lock(&m_criticalSection);
    std::vector<char, MemAllocator<char> > name(strlen(pName) + 1);
    strcpy(&name[0], pName);
    ToLower(&name[0]);
    qReservedQueue<std::pair<char*, char*> >::iterator found = std::find_if(m_headers.begin(), m_headers.end(),
                                                                            std::bind1st(EqualFirst(), &name[0]));
    if (found == m_headers.end())
        return false;
    *pValue = (*found).second;
    return true;
}

bool CurlHttpConnection::AddRootCa(const unsigned char* pCertificate, int iSize)
{
    return true;
}

void CurlHttpConnection::RunThread(void* pParameter)
{
    int iContext = NSSL_ERROR_INVALID_NSSL_CONTEXT;
    if (m_bSecure)
    {
        CriticalSection* pSslLock = &s_nsslLock;
        pSslLock->Enter();
        if (s_iNsslContextUsers == 0)
        {
            s_iNsslContext = NSSLCreateContext(0);
            if (s_iNsslContext < 0)
            {
                s_iNsslContext = NSSL_ERROR_INVALID_NSSL_CONTEXT;
                pSslLock->Leave();
                m_result = qResult(RESULT_FAILURE);
                m_bFinished = true;
                return;
            }
            int iFirst = 0;
            int iSecond = 0;
            iContext = NSSLAddServerPKIGroups(s_iNsslContext, PKI_GROUPS, &iFirst, &iSecond);
            if (iContext < 0)
            {
                NSSLDestroyContext(s_iNsslContext);
                s_iNsslContext = NSSL_ERROR_INVALID_NSSL_CONTEXT;
                pSslLock->Leave();
                m_result = qResult(RESULT_FAILURE);
                m_bFinished = true;
                return;
            }
        }
        iContext = s_iNsslContext;
        ++s_iNsslContextUsers;
        pSslLock->Leave();
        if (iContext == NSSL_ERROR_INVALID_NSSL_CONTEXT)
        {
            m_result = qResult(RESULT_FAILURE);
            m_bFinished = true;
            return;
        }
        CriticalSection* pLock = &m_criticalSection;
        pLock->Enter();
        curl_easy_setopt(m_pCurl, OPTION_NSSL_CONTEXT, iContext);
        curl_easy_setopt(m_pCurl, OPTION_NSSL_PKI_GROUPS, PKI_GROUPS);
        pLock->Leave();
    }
    CURLcode code = curl_easy_perform(m_pCurl);
    curl_easy_cleanup(m_pCurl);
    m_pCurl = 0;
    if (iContext != NSSL_ERROR_INVALID_NSSL_CONTEXT)
    {
        CriticalSection* pSslLock = &s_nsslLock;
        pSslLock->Enter();
        --s_iNsslContextUsers;
        if (s_iNsslContextUsers == 0)
        {
            NSSLDestroyContext(s_iNsslContext);
            s_iNsslContext = NSSL_ERROR_INVALID_NSSL_CONTEXT;
        }
        pSslLock->Leave();
    }
    CriticalSection* pLock = &m_criticalSection;
    pLock->Enter();
    if (code == CURLE_OK)
    {
        m_result = qResult(RESULT_SUCCESS);
        m_bFinished = true;
        pLock->Leave();
    }
    else if (code == CURL_ERROR_COULDNT_RESOLVE_PROXY)
    {
        m_result = qResult(RESULT_HTTP_RESOLVE_PROXY_FAILURE);
        m_bFinished = true;
        pLock->Leave();
    }
    else if (code == CURL_ERROR_COULDNT_RESOLVE_HOST)
    {
        m_result = qResult(RESULT_HTTP_RESOLVE_HOST_FAILURE);
        m_bFinished = true;
        pLock->Leave();
    }
    else if (code == CURL_ERROR_SEND || code == CURL_ERROR_RECV)
    {
        m_result = qResult(RESULT_HTTP_IO_FAILURE);
        m_bFinished = true;
        pLock->Leave();
    }
    else
    {
        m_result = qResult(RESULT_HTTP_FAILURE);
        m_bFinished = true;
        pLock->Leave();
    }
}

void CurlHttpConnection::SetTransferBufferSize(unsigned int uiSize)
{
    m_uiTransferBufferSize = uiSize;
}

}
}

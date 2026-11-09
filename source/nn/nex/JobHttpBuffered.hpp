#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBHTTPBUFFERED_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBHTTPBUFFERED_HPP

#include "nn/nex/HttpConnection.hpp"
#include "nn/nex/HttpEventListener.hpp"
#include "nn/nex/JobHttp.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class JobHttpBuffered : public JobHttp, public HttpEventListener
{
public:
    JobHttpBuffered();
    virtual ~JobHttpBuffered();

    virtual qResult Initialize(unsigned int uiCallId, const String& rUrl, HttpConnection::Method method,
                               HttpEventListener* pListener, unsigned int uiBufferSize)
    {
        return qResult(RESULT_CORE_NOT_IMPLEMENTED);
    }

    virtual qResult Initialize(unsigned int uiCallId, const String& rUrl, HttpConnection::Method method,
                               unsigned int uiMaxSize, qVector<unsigned char>* pBuffer, int* pStatusCode);

    virtual void ProcessResponseHeader(HttpConnection* pConnection, int iStatusCode);
    virtual bool ProcessResponse(const unsigned char* pData, unsigned int uiSize);

private:
    enum
    {
        RECEIVE_BUFFER_SIZE = 16384
    };

    qVector<unsigned char>* m_pBuffer;
    int* m_pStatusCode;
    unsigned int m_uiMaxSize;
};

}
}

#endif

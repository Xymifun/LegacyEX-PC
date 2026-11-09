#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPEVENTLISTENER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPEVENTLISTENER_HPP

namespace nn
{
namespace nex
{

class HttpConnection;

class HttpEventListener
{
public:
    virtual ~HttpEventListener() {}

    virtual void PrepareRequest(HttpConnection* pConnection) {}

    virtual unsigned int PostChunkedBuffer(HttpConnection* pConnection, unsigned char* pBuffer, unsigned int uiSize,
                                           bool* pbLastChunk)
    {
        *pbLastChunk = true;
        return 0;
    }

    virtual void ProcessResponseHeader(HttpConnection* pConnection, int iStatusCode) {}

    virtual bool ProcessResponse(const unsigned char* pData, unsigned int uiSize) { return true; }
};

}
}

#endif

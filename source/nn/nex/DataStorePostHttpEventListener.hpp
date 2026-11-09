#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPOSTHTTPEVENTLISTENER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPOSTHTTPEVENTLISTENER_HPP

#include "nn/nex/DataStoreKeyValue.hpp"
#include "nn/nex/HttpEventListener.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class DataStorePostObjectEventListener;
class HttpConnection;

class DataStorePostHttpEventListener : public HttpEventListener, public RootObject
{
public:
    DataStorePostHttpEventListener(unsigned int uiCallId, const qVector<DataStoreKeyValue>& rHeaders,
                                   const qVector<DataStoreKeyValue>& rFormFields, const qVector<unsigned char>& rRootCa,
                                   unsigned int uiThreadPriority, unsigned int uiTransferBufferSize,
                                   const unsigned char* pData, unsigned int uiDataSize,
                                   DataStorePostObjectEventListener* pListener);
    virtual ~DataStorePostHttpEventListener();

    virtual void PrepareRequest(HttpConnection* pConnection);
    virtual unsigned int PostChunkedBuffer(HttpConnection* pConnection, unsigned char* pBuffer, unsigned int uiSize,
                                           bool* pbLastChunk);

private:
    unsigned int m_uiCallId;
    const qVector<DataStoreKeyValue>& m_rHeaders;
    const qVector<DataStoreKeyValue>& m_rFormFields;
    const qVector<unsigned char>& m_rRootCa;
    unsigned int m_uiThreadPriority;
    unsigned int m_uiTransferBufferSize;
    const unsigned char* m_pData;
    unsigned int m_uiDataSize;
    DataStorePostObjectEventListener* m_pListener;
    unsigned int m_uiPosted;
    qVector<unsigned char> m_formHeader;
    qVector<unsigned char> m_formFooter;
    bool m_bBodySent;
    unsigned int m_uiUnknown50;
};

}
}

#endif

#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MESSAGE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MESSAGE_HPP

#include "nn/nex/ByteStream.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class EndPoint;
class ClientProtocol;

class Message : public ByteStream
{
public:
    Message();
    explicit Message(Buffer* pBuffer);
    virtual ~Message();

    void InitCommon();
    void InitBuffer();
    void SetSize();
    void SetToBegining();

    Buffer* GetBuffer();
    unsigned int GetSize();
    qResult GetLastError();

    void AddDataFromMessage(const Message& rMessage);
    void AppendMessage(Message& rMessage);
    void ExtractMessage(Message& rMessage);
    void TruncateMessage(Message& rMessage);

    void SetSourceEndPoint(EndPoint* pEndPoint);

    bool IsCompactFormat() const { return m_bCompactFormat; }

protected:
    friend class ClientProtocol;
    friend class AnyDataAdapter;
    friend class ServerProtocol;
    friend class ProtocolRequestBroker;

    qResult m_Result;
    unsigned long long m_uiBufferState;
    unsigned int m_uiCommonStateA;
    unsigned int m_uiCommonStateB;
    unsigned int m_uiCommonStateC;
    EndPoint* m_pSourceEndPoint;
    unsigned char m_uiBufferFlag;
    unsigned int m_uiCommonStateD;
    unsigned char m_uiCommonFlag;
    bool m_bCompactFormat;
};

}
}

#endif

#ifndef SOURCE_NN_NEX_CLIENTPROTOCOL_HPP
#define SOURCE_NN_NEX_CLIENTPROTOCOL_HPP

#include "nn/nex/Protocol.hpp"

namespace nn
{
namespace nex
{

class Credentials;

class ClientProtocol : public Protocol
{
public:
    explicit ClientProtocol(unsigned int uiFlags);
    virtual ~ClientProtocol();

    virtual const wchar_t* GetType() const { return L"ClientProtocol"; }

    virtual bool IsAKindOf(const wchar_t* pType) const
    {
        return std::wcscmp(L"ClientProtocol", pType) == 0 || Protocol::IsAKindOf(pType);
    }

    virtual void EnforceDeclareSysComponentMacro() {}

    virtual _Type GetProtocolType() const { return TYPE_CLIENT; }

    virtual void ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext) = 0;
    virtual Protocol* CreateResponder() const = 0;
    virtual void SetDefaultCredentials(Credentials* pCredentials);

    bool SendRMCMessage(ProtocolCallContext* pCallContext, Message* pMessage);
    bool SendOverLocalLoopback(ProtocolCallContext* pCallContext, Message* pMessage);
    void ProcessResponse(Message* pMessage, EndPoint* pEndPoint);

    static void SetCallError(qResult result);

protected:
    Credentials* m_pDefaultCredentials;
};

}
}

#endif

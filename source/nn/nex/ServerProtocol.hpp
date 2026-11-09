#ifndef SOURCE_NN_NEX_SERVERPROTOCOL_HPP
#define SOURCE_NN_NEX_SERVERPROTOCOL_HPP

#include "nn/nex/Protocol.hpp"

namespace nn
{
namespace nex
{

class ServerProtocol : public Protocol
{
public:
    explicit ServerProtocol(unsigned int uiFlags);
    virtual ~ServerProtocol();

    virtual const wchar_t* GetType() const { return L"ServerProtocol"; }

    virtual bool IsAKindOf(const wchar_t* pType) const
    {
        return std::wcscmp(L"ServerProtocol", pType) == 0 || Protocol::IsAKindOf(pType);
    }

    virtual void EnforceDeclareSysComponentMacro() {}

    virtual _Type GetProtocolType() const { return TYPE_SERVER; }

    virtual void DispatchProtocolMessage(Message* pRequest, Message* pResponse, bool* pResponseRequired,
                                         EndPoint* pEndPoint) = 0;
    virtual void DispatchProtocolMessageWithAttemptCount(unsigned int uiAttemptCount, Message* pRequest,
                                                         Message* pResponse, bool* pResponseRequired, int* pAttempt,
                                                         EndPoint* pEndPoint)
    {
    }

    virtual bool UseAttemptCountMethod() { return false; }

    void DispatchCallRequest(unsigned int uiAttemptCount, Message* pRequest, Message* pResponse,
                             bool* pResponseRequired, int* pAttempt, EndPoint* pEndPoint);
    void SetCallError(qResult result);
};

}
}

#endif

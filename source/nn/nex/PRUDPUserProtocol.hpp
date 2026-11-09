#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PRUDPUSERPROTOCOL_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PRUDPUSERPROTOCOL_HPP

#include "nn/nex/EndPointEventHandler.hpp"
#include "nn/nex/InstanceTable.hpp"
#include "nn/nex/PseudoSingleton.hpp"
#include "nn/nex/ThreadVariable.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class EndPoint;

class PRUDPUserProtocol : public EndPointEventHandler
{
public:
    enum
    {
        ClassID = 13
    };

    typedef void (*UserReceiveCallback)(unsigned int uiConnectionID, unsigned char* pData, unsigned int uiLength);
    typedef void (*UserPrivateReceiveCallback)(unsigned int uiConnectionID, Buffer* pBuffer, unsigned int uiFlags,
                                               unsigned char uiSubStreamID);

    PRUDPUserProtocol();
    virtual ~PRUDPUserProtocol();

    virtual void Receive(EndPoint* pEndPoint, Buffer* pBuffer, unsigned char uiSubStreamID);
    virtual void FaultDetection(EndPoint* pEndPoint, unsigned int uiError);
    virtual void PeerDisconnected(EndPoint* pEndPoint);
    virtual void Receive(EndPoint* pEndPoint, Buffer* pBuffer, unsigned int uiFlags, unsigned char uiSubStreamID);

    static PRUDPUserProtocol* GetInstance()
    {
        unsigned int uiContextId = g_usingInstantiationContext;
        if (g_usingInstantiationContext)
            uiContextId = g_instantiationContextVariable->GetRef();

        InstanceControl* pInstanceControl = g_instanceTable.FindInstance(ClassID, uiContextId);
        void* pInstance = pInstanceControl;
        if (pInstance != 0)
            pInstance = pInstanceControl->GetDelegatorInstance();

        return static_cast<PRUDPUserProtocol*>(pInstance);
    }

    static void AcquireInstance();
    static void ReleaseInstance();

    void RegisterUserReceiveCallback(UserReceiveCallback pCallback);
    void UnregisterUserReceiveCallback();
    void RegisterUserPrivateReceiveCallback(UserPrivateReceiveCallback pCallback);
    void UnregisterUserPrivateReceiveCallback();
    qResult Send(EndPoint* pEndPoint, const unsigned char* pData, unsigned int uiSize, unsigned int uiFlags,
                 unsigned char uiSubStreamID, unsigned int uiKey);

private:
    PseudoSingleton m_instanceControl;
    UserReceiveCallback m_pUserReceiveCallback;
    UserPrivateReceiveCallback m_pUserPrivateReceiveCallback;
};

}
}

#endif

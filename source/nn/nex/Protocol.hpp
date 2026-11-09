#ifndef SOURCE_NN_NEX_PROTOCOL_HPP
#define SOURCE_NN_NEX_PROTOCOL_HPP

#include "nn/nex/SystemComponent.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class CallProtocolMethodOperation;
class Buffer;
class EndPoint;
class Message;
class OperationManager;
class Operation;
class ProtocolCallContext;
class ProtocolRequestBrokerInterface;

class Protocol : public SystemComponent
{
public:
    enum _Type
    {
        TYPE_CLIENT = 0,
        TYPE_SERVER = 1
    };

    enum _Command
    {
        COMMAND_RESPONSE = 0,
        COMMAND_REQUEST = 1
    };

    explicit Protocol(unsigned int uiFlags);
    virtual ~Protocol();

    virtual const wchar_t* GetType() const;
    virtual bool IsAKindOf(const wchar_t* pType) const
    {
        return std::wcscmp(L"Protocol", pType) == 0 || SystemComponent::IsAKindOf(pType);
    }

    virtual void EnforceDeclareSysComponentMacro();
    virtual bool BeginInitialization();
    virtual bool BeginTermination();
    virtual _Type GetProtocolType() const = 0;
    virtual void EndPointDisconnected(EndPoint* pEndPoint) {}

    virtual void FaultDetected(EndPoint* pEndPoint, unsigned int uiReason) {}

    virtual Protocol* Clone() const { return 0; }


    bool FlagIsSet(unsigned int uiFlag) const;
    void SetFlag(unsigned int uiFlag);
    void ClearFlag(unsigned int uiFlag);

    void SetProtocolID(unsigned short uiProtocolID);

    unsigned short GetProtocolID() const { return m_uiProtocolID; }

    OperationManager* GetOperationManager() const;
    EndPoint* GetIncomingConnection() const;
    void SetIncomingConnection(EndPoint* pEndPoint);
    EndPoint* GetOutgoingConnection() const;
    void SetOutgoingConnection(EndPoint* pEndPoint);
    void UseLocalLoopback(unsigned int uiCallerPID, unsigned int uiCallerCID);
    unsigned int GetCallerPID() const;
    unsigned int GetCallerCID() const;
    unsigned int GetOwnPID() const;
    Operation* GetCurrentOperation();
    void AssociateProtocolRequestBroker(ProtocolRequestBrokerInterface* pRequestBroker);
    void CopyMembers(const Protocol* pProtocol);

    static void AddCallOutcome(Message* pMessage, qResult result);
    static void AddCallContextID(Message* pMessage, unsigned int uiCallContextID);
    static void AddCallResponse(Message* pMessage, CallProtocolMethodOperation* pOperation,
                                unsigned int uiCallContextID, const Message& rResponse);
    static void AddProtocolKey(Message* pMessage, _Command command, unsigned short uiProtocolID);
    static void ExtractProtocolKey(Message* pMessage, _Command& rCommand, unsigned short& rProtocolID);
    static void ExtractCallOutcome(Message* pMessage, qResult* pResult);
    static bool RegisterCallContext(Message* pMessage, ProtocolCallContext* pCallContext);
    static void AddSignature(Message* pMessage, const Buffer& rSignature);
    static void ExtractSignature(Message* pMessage, Buffer& rSignature);
    static void AddOriginalMessage(Message* pMessage, Message* pOriginalMessage);
    static void ExtractOriginalMessageHeader(Message* pMessage, Buffer& rHeader);
    static void AddAlternativePrincipalID(Message* pMessage, unsigned int uiPrincipalID);
    static unsigned int ExtractAlternativePrincipalID(Message* pMessage);
    static unsigned int ExtractCallContextID(Message* pMessage);
    static ProtocolCallContext* RetrieveCallContext(Message* pMessage);
    static unsigned int ExtractMethodID(Message* pMessage);
    static void AddMethodID(Message* pMessage, unsigned int uiMethodID);
    static bool IsOldRVDDLVersion(EndPoint* pEndPoint);

protected:
    friend class ClientProtocol;
    friend class ProtocolRequestBroker;

    unsigned short m_uiProtocolID;
    EndPoint* m_pOutgoingConnection;
    ProtocolRequestBrokerInterface* m_pProtocolRequestBroker;
    unsigned int m_uiFlags;
    EndPoint* m_pIncomingConnection;
    bool m_bLocalLoopback;
    unsigned int m_uiCallerPID;
    unsigned int m_uiCallerCID;
};

}
}

#endif

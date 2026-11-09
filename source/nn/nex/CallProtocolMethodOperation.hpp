#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CALLPROTOCOLMETHODOPERATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CALLPROTOCOLMETHODOPERATION_HPP

#include "nn/nex/Operation.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class ServerProtocol;
class JobProcessProtocolMessage;
class Protocol;

class CallProtocolMethodOperation : public Operation
{
public:
    CallProtocolMethodOperation();
    virtual ~CallProtocolMethodOperation() {}
    virtual unsigned int GetType() const;
    virtual const wchar_t* GetClassNameString() const;
    virtual void ForceImplOperationCommonMethodsMacro();

    void PostponeOperation(int delay);
    void InitializeForNewCallAttempt();
    unsigned int GetAttemptCount();
    void AttachUserContext(const UserContext& context);
    UserContext GetUserContext();

private:
    friend class ClientProtocolRequestBroker;
    friend class ProtocolRequestBroker;
    friend class ServerProtocol;
    friend class JobProcessProtocolMessage;
    friend class Protocol;

    unsigned int m_attemptCount;
    bool m_postponed;
    UserContext m_callUserContext;
    int m_postponementDelay;
    qResult m_result;
};

}
}

#endif

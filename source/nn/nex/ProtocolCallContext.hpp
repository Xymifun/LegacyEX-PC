#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PROTOCOLCALLCONTEXT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PROTOCOLCALLCONTEXT_HPP

#include "nn/nex/CallContext.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class AccountManagementClient;
class Credentials;

class ProtocolCallContext : public CallContext
{
public:
    explicit ProtocolCallContext(Credentials* pCredentials = 0);
    virtual ~ProtocolCallContext();

    void SetCredentials(Credentials* pCredentials);
    void AddReturnValuePtr(void* pValue);
    void ClearReturnValuePtr();
    void* GetReturnValuePtr(unsigned int uiIndex);

    Credentials* GetCredentials() const { return m_credentials; }

protected:
    friend class AccountManagementClient;
    friend class JobManageAccount;

    virtual void BeginTransition(State state, qResult result, bool publishResult);

    qVector<void*> m_returnValues;
    bool* m_completionFlag;
    qResult* m_completionResult;
    Credentials* m_credentials;
};

}
}

#endif

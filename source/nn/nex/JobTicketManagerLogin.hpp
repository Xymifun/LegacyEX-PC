#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBTICKETMANAGERLOGIN_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBTICKETMANAGERLOGIN_HPP

#include <stdint.h>

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/Buffer.hpp"
#include "nn/nex/CallContext.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class Data;
class RVConnectionData;
class TicketManager;

class JobTicketManagerLogin : public StepSequenceJob
{
public:
    JobTicketManagerLogin(unsigned int uiCallId, TicketManager* pTicketManager, qResult* pResult,
                          const String& rUserName, const char* pPassword, unsigned int* pPrincipalID,
                          RVConnectionData* pConnectionData, String* pReturnMessage,
                          AnyObjectHolder<Data, String>* pCustomData);
    virtual ~JobTicketManagerLogin();

    void CompleteJob(CallContext::State state, qResult result);
    void InitiateLogin();
    void ProcessLoginResult();
    void DeriveKey();
    void ValidateKey();

private:
    unsigned int m_uiCallId;
    TicketManager* m_pTicketManager;
    uint32_t m_unknown78;
    uint32_t m_unknown7C;
    CallContext m_keyContext;
    Key m_key;
    String m_userName;
    char* m_pPassword;
    unsigned int* m_pPrincipalID;
    String* m_pReturnMessage;
    RVConnectionData* m_pConnectionData;
    AnyObjectHolder<Data, String>* m_pCustomData;
    ProtocolCallContext m_loginContext;
    Buffer m_ticketData;
    qResult* m_pResult;
};

}
}

#endif

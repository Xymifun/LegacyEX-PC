#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBTICKETMANAGERLOGINWITHDATA_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBTICKETMANAGERLOGINWITHDATA_HPP

#include <stdint.h>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/CallContext.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class LoginData;
class RVConnectionData;
class TicketManager;

template <typename T>
class CustomDataHolder;

class JobTicketManagerLoginWithData : public StepSequenceJob
{
public:
    JobTicketManagerLoginWithData(unsigned int uiCallId, TicketManager* pTicketManager, qResult* pResult,
                                  const char* pPassword, unsigned int* pPrincipalID, RVConnectionData* pConnectionData,
                                  CustomDataHolder<LoginData>* pLoginData);
    virtual ~JobTicketManagerLoginWithData();

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
    char* m_pPassword;
    unsigned int* m_pPrincipalID;
    RVConnectionData* m_pConnectionData;
    CustomDataHolder<LoginData>* m_pLoginData;
    ProtocolCallContext m_loginContext;
    Buffer m_ticketData;
    qResult* m_pResult;
};

}
}

#endif

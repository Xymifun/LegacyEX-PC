#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBBACKENDSERVICESLOGOUT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBBACKENDSERVICESLOGOUT_HPP

#include "nn/nex/CallContext.hpp"
#include "nn/nex/StepSequenceJob.hpp"

namespace nn
{
namespace nex
{

class BackEndServices;
class Credentials;
class StreamManager;

class JobBackEndServicesLogout : public StepSequenceJob
{
public:
    JobBackEndServicesLogout(unsigned int uiCallId, BackEndServices* pBackEndServices, Credentials* pCredentials);
    virtual ~JobBackEndServicesLogout();

    void DisconnectFromAuthenticationService();
    void ProcessAuthDisconnectionResult();
    void DisconnectFromSecureConnectionService();
    void ProcessSecConnDisconnectionResult();
    void MarkCredentialsInvalid();
    void TerminateStreamManager();
    void WaitForStreamManagerTermination();
    void CompleteLogout();
    void CompleteLogoutFailure();

private:
    unsigned int m_uiCallId;
    BackEndServices* m_pBackEndServices;
    Credentials* m_pCredentials;
    StreamManager* m_pStreamManager;
    CallContext m_callContext;
};

}
}

#endif

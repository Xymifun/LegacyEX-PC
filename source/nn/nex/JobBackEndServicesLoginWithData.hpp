#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBBACKENDSERVICESLOGINWITHDATA_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBBACKENDSERVICESLOGINWITHDATA_HPP

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/Buffer.hpp"
#include "nn/nex/CallContext.hpp"
#include "nn/nex/CustomDataHolder.hpp"
#include "nn/nex/LoginData.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/RVConnectionData.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class BackEndServices;
class Credentials;
class Data;
class EndPoint;
class JobConnectEndPoint;

class JobBackEndServicesLoginWithData : public StepSequenceJob
{
public:
    JobBackEndServicesLoginWithData(unsigned int uiCallId, BackEndServices* pBackEndServices, qResult* pResult,
                                    const char* pPassword, const wchar_t* pHost, unsigned short uiPort,
                                    const CustomDataHolder<LoginData>& rLoginData,
                                    const AnyObjectHolder<Data, String>& rExtraData, Credentials** ppCredentials,
                                    int iTimeout);
    virtual ~JobBackEndServicesLoginWithData();

    virtual void CancelJob();

    void CompleteJob(qResult result);
    void ValidateArguments();
    void ConnectToAuthenticationService();
    void ProcessAuthConnectionResult();
    void Authenticate();
    void ProcessAuthenticationResult();
    void ConnectToSecureConnectionService();
    void ProcessSecConnConnectionResult();
    void RegisterURLs();
    void CompleteLogin();
    void CompleteLogout();
    void DisconnectFromAuthenticationService();
    void ProcessAuthDisconnectionResult();

private:
    unsigned int m_uiCallId;
    char* m_pPassword;
    String m_host;
    unsigned short m_uiPort;
    Credentials** m_ppCredentials;
    CallContext m_connectContext;
    ProtocolCallContext m_protocolContext;
    qResult m_authResult;
    qResult m_jobResult;
    Buffer m_buffer;
    BackEndServices* m_pBackEndServices;
    StationURL m_publicURL;
    RVConnectionData m_connectionData;
    qResult* m_pResult;
    unsigned int m_reserved0;
    CustomDataHolder<LoginData> m_loginData;
    AnyObjectHolder<Data, String> m_extraData;
    JobConnectEndPoint* m_pConnectJob;
    EndPoint* m_pAuthenticationEndPoint;
    EndPoint* m_pSecureEndPoint;
    Time m_deadline;
    bool m_bCancelled;
};

}
}

#endif

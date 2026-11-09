#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBBACKENDSERVICESLOGIN_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBBACKENDSERVICESLOGIN_HPP

#include "nn/nex/Buffer.hpp"
#include "nn/nex/CallContext.hpp"
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
class StreamManager;

template <typename T, typename KeyType>
class AnyObjectHolder;

extern AnyObjectHolder<Data, String> g_registerURLsExtraData;

class JobBackEndServicesLogin : public StepSequenceJob
{
public:
    typedef StreamManager* (*StreamManagerFactory)();

    JobBackEndServicesLogin(unsigned int uiCallId, BackEndServices* pBackEndServices, qResult* pResult,
                            const String& rUserName, const char* pPassword, const wchar_t* pHost, unsigned short uiPort,
                            Credentials** ppCredentials, String* pErrorMessage,
                            AnyObjectHolder<Data, String>* pExtraData, int iTimeout,
                            StreamManagerFactory pStreamManagerFactory);
    virtual ~JobBackEndServicesLogin();

    virtual void CancelJob();

    void CompleteJob(qResult result);
    void ValidateArguments();
    StreamManager* CreateStreamManager() const;
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
    String m_userName;
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
    String* m_pErrorMessage;
    AnyObjectHolder<Data, String>* m_pExtraData;
    JobConnectEndPoint* m_pConnectJob;
    EndPoint* m_pAuthenticationEndPoint;
    EndPoint* m_pSecureEndPoint;
    Time m_deadline;
    bool m_bCancelled;
    StreamManagerFactory m_pStreamManagerFactory;
};

}
}

#endif

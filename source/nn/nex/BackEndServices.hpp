#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_BACKENDSERVICES_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_BACKENDSERVICES_HPP

#include "nn/nex/CallContext.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/UserContext.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class AnyDataAdapter;
class AuthenticationClient;
class Credentials;
class Data;
class Job;
class JobBackEndServicesLogin;
class JobBackEndServicesLoginWithData;
class JobBackEndServicesLogout;
class JobBackEndServicesTerminate;
class LoginData;
class NintendoNotificationEventHandler;
class NintendoNotificationEventManager;
class NotificationEventHandler;
class NotificationEventManager;
class ProtocolRegistry;
class RendezVousLoginOperation;
class RendezVousLogoutOperation;
class RendezVousOperation;
class SecureConnectionClient;
class SecureStream;
class StreamManager;

template <typename T, typename KeyType>
class AnyObjectHolder;

template <typename T>
class CustomDataHolder;

class BackEndServices : public RootObject
{
public:
    typedef StreamManager* (*StreamManagerFactory)();

    class InternalData
    {
    public:
        InternalData() : m_userContext(), m_bIsSet(false) {}

        ~InternalData() { m_bIsSet = false; }

        void Set(const CallContext::CompletionCallback& rCallback, const UserContext& rUserContext);

        CallContext::CompletionCallback m_cbCallback;
        UserContext m_userContext;
        bool m_bIsSet;
    };

    class EffectiveLoginTimeout
    {
    public:
        EffectiveLoginTimeout();

        EffectiveLoginTimeout& operator=(int iTimeout)
        {
            m_iTimeout = iTimeout;
            return *this;
        }

        operator int() const { return m_iTimeout; }

    private:
        int m_iTimeout;
    };

    BackEndServices();
    virtual ~BackEndServices();

    virtual void PostLogoutCleanup();
    virtual void RegisterServerProtocols();
    virtual void UnregisterServerProtocols();

    bool TerminateJobIsInProgress() const;
    bool TerminateImpl();
    static void TerminateCB(CallContext* pCallContext, const UserContext* pUserContext);
    bool Terminate(CallContext* pCallContext);

    StreamManager* GetStreamManager() const;
    SecureStream* GetSecureStream() const;
    SecureConnectionClient* GetSecureConnectionClient() const;
    AuthenticationClient* GetAuthenticationClient() const;
    ProtocolRegistry* GetProtocolRegistry() const;

    qResult IsConnected() const;
    bool Disconnect(CallContext* pCallContext);

    void RegisterOperationOnCallContext(CallContext& rCallContext, RendezVousOperation& rOperation);
    void OnLogoutStart(CallContext& rCallContext);
    void OnLoginStart(CallContext& rCallContext);

    bool LogoutImpl(CallContext* pCallContext, Credentials* pCredentials);
    bool Logout_(CallContext* pCallContext, Credentials* pCredentials);

    void RegisterPreTerminateCallback(CallContext::CompletionCallback pCallback, const UserContext& rUserContext);
    void ReleaseCredentials();

    bool RegisterNotificationEventHandler(NotificationEventHandler* pHandler);
    bool UnregisterNotificationEventHandler(NotificationEventHandler* pHandler);

    StreamManager* CreateStreamManager() const;
    static void SetSandboxAccessKey(const String& rKey);

    bool LoginJobIsInProgress() const;
    bool LogoutJobIsInProgress() const;
    void CancelLoginJob();

    void SetEffectiveLoginTimeout(int iTimeout);
    bool EffectiveTimeoutIsSet() const;

    bool LoginImpl(CallContext* pCallContext, qResult* pResult, const wchar_t* pUserName, const char* pPassword,
                   const wchar_t* pHost, unsigned short uiPort, Credentials** ppCredentials, String* pErrorMessage,
                   AnyObjectHolder<Data, String>* pExtraData);
    bool LoginImplWithData(CallContext* pCallContext, qResult* pResult, const char* pPassword, const wchar_t* pHost,
                           unsigned short uiPort, const CustomDataHolder<LoginData>& rLoginData,
                           const AnyObjectHolder<Data, String>& rExtraData, Credentials** ppCredentials);

    void InitStreamManagerFactory(StreamManagerFactory pFactory) { m_pStreamManagerFactory = pFactory; }

    unsigned short GetWellKnownPortNumber() const { return m_uiPort; }

    Credentials* GetCredentials() { return m_pCredentials; }

protected:
    friend class JobBackEndServicesLogin;
    friend class JobBackEndServicesLoginWithData;
    friend class JobBackEndServicesLogout;
    friend class JobBackEndServicesTerminate;

    InternalData m_preTerminate;
    CallContext m_terminateCallContext;
    CallContext* m_pTerminateContext;
    int m_iDefaultLoginTimeout;
    Credentials* m_pCredentials;
    unsigned short m_uiPort;
    unsigned char m_uiStreamFlags;
    unsigned int m_uiStreamParameter;
    NotificationEventManager* m_pNotificationEventManager;
    NintendoNotificationEventManager* m_pNintendoNotificationEventManager;
    bool m_reserved0;
    StreamManagerFactory m_pStreamManagerFactory;
    Job* m_pLoginJob;
    Job* m_pLogoutJob;
    Job* m_pTerminateJob;
    EffectiveLoginTimeout m_effectiveLoginTimeout;
    RendezVousLoginOperation* m_pLoginOperation;
    RendezVousLogoutOperation* m_pLogoutOperation;
    bool m_reserved1;
    StationURL m_stationURL;
};

}
}

#endif

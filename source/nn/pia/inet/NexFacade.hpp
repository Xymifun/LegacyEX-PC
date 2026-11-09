#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_FACADE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_FACADE_HPP

#include "nn/pia.h"
#include "nn/pia/String.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/nex/NotificationEvents.hpp"
#include "nn/pia/inet/IFacade.hpp"

namespace nn
{
namespace nex
{
class CallContext;
class InetAddress;
class MatchmakeExtensionClient;
class MatchmakeSession;
class NATTraversalRelayClient;
class NgsFacade;
class NgsBridgeInterface;
class NotificationEvent;
class NotificationEventHandler;
class ProtocolCallContext;
class StationURL;
class UserContext;
template <typename T> class qList;
}
}

namespace nn
{
namespace pia
{
namespace common
{
class InetAddress;
class Socket;
class SocketStreamBase;
}
namespace nat
{
class NatTraverser;
}
namespace transport
{
class StationLocation;
class StationConnectionInfo;
}
}
}

namespace nn
{
namespace pia
{
namespace inet
{

class NexNatRelay;
class NexNotificationDataJob;
class NexNatServerAddressResolveJob;

void ReplaceURLCompletionCallback(nex::CallContext* callContext, const nex::UserContext* userContext);

class NexFacade : public IFacade
{
public:
    struct LoginInfo
    {
        nex::NgsFacade* ngsFacade;
        unsigned int gameServerId;
        const common::String* accessKey;
    };

    NexFacade();
    virtual unsigned int GetRandomPortRangeMin() const;
    virtual unsigned int GetRandomPortRangeMax() const;
    virtual void RelayProbeRequest(const transport::StationLocation& source,
                                   const transport::StationLocation& destination);
    virtual void UpdateRelayTable();
    virtual unsigned short GetReservedPort() const;
    virtual const common::InetAddress& GetRelayServerAddress() const;
    virtual unsigned int GetLocalServiceConnectionId() const;
    virtual bool IsBeginMonitoringDataAvailable() const;
    static Result CreateInstance();
    static void DestroyInstance();
    static NexFacade* GetInstance() { return s_instance; }
    Result Initialize();
    void Finalize();
    Result CreateProtocols();
    Result Bind(const LoginInfo* loginInfo);
    void Unbind();
    Result Startup();
    void Cleanup();
    Result StartNatSessionAsync();
    bool IsStartNatSessionCompleted() const;
    Result GetStartNatSessionResult() const;
    Result CancelStartNatSessionAsync();
    void StopNatSession();
    void UpdateGatheringID(unsigned int gatheringId);
    unsigned int GetNatPropertyMapping() const;
    unsigned long long GetLocalPrincipalId() const;
    bool RegisterNexNotificationEventHandler4Pia(nex::NotificationEventHandler* handler);
    bool UnregisterNexNotificationEventHandler4Pia(nex::NotificationEventHandler* handler);
    void SetPersistentGatheringCodeKey();
    void ClearPersistentGatheringCodeKey();
    Result UpdateNotificationDataAsync(nex::NotificationEvents::NotificationEvents type, unsigned int param1,
                                       unsigned int param2, const common::String& text);
    Result RetrieveNotificationDataAsync(unsigned char typeMask, nex::qList<nex::NotificationEvent>* events);
    bool IsRelayServiceEnabled() const;
    void SetIgnoreNatProbeRequest(bool ignore);
    void NotifyLocalServiceConnectionId(unsigned int connectionId);
    void NotifyProbeRequest(const transport::StationLocation& location);
    void NotifyLeave(const transport::StationLocation& location, bool clearMonitoringData, bool relayLeave);
    bool IsTraversed(const transport::StationLocation& location) const;
    bool IsTraversed(unsigned int index) const;
    bool PrepareNatTraversal(const transport::StationLocation& location, const transport::StationConnectionInfo& info,
                             common::CallContext* context, bool relay, bool passive, StationIndex stationIndex);
    bool CheckLatestStationLocation(transport::StationLocation& location);
    void ResetProbeStatus(const transport::StationLocation& location, bool relay);
    void ReportNatTraversalResult(const transport::StationLocation& location, Result result, bool relay);
    void CancelCleanupNatServerAddressResolution();
    void CompletedNatServerAddressResolution();
    void CompleteStartNatSession();
    Result UpdateNatServerAddress();
    void suspendBossDaemon();
    void resumeBossDaemon();
    static void NatServerAddressResolveCompletionCallback(Result result, void* param);
    static void NatTraverserStartupCompletionCallback(Result result, void* param);
    static void ConvertNexInetAddressToInetAddress(const nex::InetAddress& source, common::InetAddress* destination);
    static void ConvertInetAddressToNexInetAddress(const common::InetAddress& source, nex::InetAddress* destination);
    static void ConvertNexStationUrlToStationLocation(const nex::StationURL& url, transport::StationLocation* location);
    static Result ConvertNexStationUrlToStationConnectionInfo(const nex::qList<nex::StationURL>& urls,
                                                              transport::StationConnectionInfo* info);

private:
    friend class InetConnectStationJob;
    friend class InetCreateMeshJob;
    friend class InetInputStream;
    friend class InetJoinMeshJob;
    friend class InetOutputStream;
    friend class InetProcessHostMigrationJob;
    friend class MonitoringDataSender;
    friend class NexMatchMeshLayerController;
    friend class NexNatRelay;
    friend class NexNatServerAddressResolveJob;
    friend class NexNotificationDataJob;
    static NexFacade* s_instance;
    static unsigned char* s_instanceMemory;
    unsigned int m_gameServerId;
    common::FixedString<8> m_accessKey;
    nex::NgsBridgeInterface* m_ngsBridge;
    unsigned int m_gatheringId;
    unsigned char* m_matchmakeExtensionMemory;
    nex::MatchmakeExtensionClient* m_matchmakeExtensionClient;
    nex::MatchmakeSession* m_matchmakeSession;
    unsigned char* m_natRelayClientMemory;
    nex::NATTraversalRelayClient* m_natRelayClient;
    NexNatRelay* m_natRelay;
    nat::NatTraverser* m_natTraverser;
    common::Socket* m_socket;
    common::SocketStreamBase* m_outputStream;
    common::SocketStreamBase* m_inputStream;
    bool m_started;
    bool m_natSessionActive;
    bool m_relayServiceEnabled;
    unsigned int m_natTraversalTimeout;
    unsigned int m_natTraversalTimeoutEdmExtension;
    bool m_bossDaemonSuspendEnabled;
    int m_bossDaemonSuspendCount;
    nex::NotificationEventHandler* m_notificationEventHandler;
    NexNotificationDataJob* m_notificationDataJob;
    unsigned char m_asyncProcess;
    common::CallContext m_natSessionContext;
    common::CallContext m_natTraverserStartupContext;
    common::CallContext m_natServerAddressResolveContext;
    NexNatServerAddressResolveJob* m_natServerAddressResolveJob;
    nex::StationURL* m_localStationUrl;
    nex::StationURL* m_publicStationUrl;
    nex::ProtocolCallContext* m_replaceUrlContext;
    bool m_ignoreNatProbeRequest;
    unsigned int m_localServiceConnectionId;
};

}
}
}

#endif

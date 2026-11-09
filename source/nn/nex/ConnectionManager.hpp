#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CONNECTIONMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CONNECTIONMANAGER_HPP

#include "nn/nex/ComponentState.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/Job.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/StreamTable.hpp"
#include "nn/nex/TransportEventHandler.hpp"
#include "nn/nex/UserContext.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class CallContext;
class ConnectionOrientedStream;
class EndPoint;
class EndPointEventHandler;
class JobConnectEndPoint;

class ConnectionManager : public TransportEventHandler
{
public:
    typedef bool (*ConnectionFilter)(const UserContext& rUserContext, Buffer* pRequest, Buffer* pResponse,
                                     EndPoint* pEndPoint);

    ConnectionManager(EndPointEventHandler* pEndPointEventHandler, Job::JobType jobType, bool bFlag,
                      bool bRoutingEnabled);
    virtual ~ConnectionManager();

    virtual bool ConnectionRequest(ConnectionOrientedStream* pStream, const StationURL* pStationURL, Buffer* pBuffer,
                                   EndPoint* pEndPoint);
    virtual bool FilterConnectionRequest(ConnectionOrientedStream* pStream, const StationURL* pStationURL,
                                         Buffer* pBuffer, EndPoint* pEndPoint);

    void Terminate();
    bool IsTerminated() const;
    void AttachStream(ConnectionOrientedStream* pStream, bool bFlag);
    void RegisterConnectionFilter(ConnectionFilter pFilter, UserContext userContext);
    EndPoint* OpenEndPoint(const StationURL& rStationURL);
    bool Disconnect(CallContext* pCallContext, EndPoint* pEndPoint);
    void SetRouterAddress(const InetAddress& rAddress);
    void ConfigureEndPointForRouting(EndPoint* pEndPoint);
    void ConstructAndAssociateRoutingURL(unsigned short uiPort, StationURL oStationURL, StationURL* pRoutingURL);
    bool ConnectImpl(CallContext* pCallContext, Buffer* pRequestBuffer, Buffer* pResponseBuffer,
                     const qList<StationURL>& rURLs, EndPoint** ppEndPoint, unsigned int uiTimeout, bool bFlag,
                     JobConnectEndPoint** ppJob);

    StreamTable& GetStreamTable() { return m_streamTable; }

    static int s_iNATTraversalTimeout;
    static unsigned int s_uiDisconnectTimeout;

private:
    friend class JobConnectEndPoint;
    friend class StreamManager;
    unsigned int m_uiContextId;
    ComponentState m_component;
    InetAddress m_routerAddress;
    ConnectionFilter m_pConnectionFilter;
    UserContext m_connectionFilterContext;
    bool m_bFilterEnabled;
    EndPointEventHandler* m_pEndPointEventHandler;
    StreamTable m_streamTable;
    bool m_flag64;
    bool m_bRoutingEnabled;
    Job::JobType m_jobType;
};

}
}

#endif

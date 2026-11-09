#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NATTRAVERSALENGINE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NATTRAVERSALENGINE_HPP

#include <functional>
#include <map>
#include <utility>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/NATProperties.hpp"
#include "nn/nex/NATTraversalResult.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/URLProbeList.hpp"
#include "nn/nex/UnsupportedNATReferee.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class ConnectivityTester;
class InetAddress;
class Job;
class NATEchoInterface;
class NATRelayInterface;
class NATTransportAdapter;
class NATTraversalStream;
class RootTransport;

class NATTraversalEngine : public RootObject
{
public:
    NATTraversalEngine();
    virtual ~NATTraversalEngine();

    virtual bool RegisterRelay(NATRelayInterface* pRelay);
    virtual bool UnregisterRelay();
    virtual bool RegisterEcho(NATEchoInterface* pEcho);
    virtual bool UnregisterEcho();
    virtual bool RegisterConnectivityTester(ConnectivityTester* pConnectivityTester);
    virtual bool UnregisterConnectivityTester();

    bool Terminate();
    bool RegisterTransportAdapter(NATTransportAdapter* pTransportAdapter);
    bool UnregisterTransportAdapter();
    unsigned int GetNumOfMaxProbes();
    bool CanUseRelay(const StationURL& rStationURL, bool bCheckURL) const;
    bool ShouldUseRelay(const StationURL& rStationURL, bool bCheckURL) const;
    bool PrepareNATTraversal(StationURL stationURL);
    bool PrepareNATTraversal(const qList<StationURL>& rStationURLs);
    void ReportNATTraversalResult(unsigned int uiConnectionID, bool bSucceeded, unsigned int uiRVConnectionID);
    void ReportNATProperties(NATProperties::MappingProperties mapping, NATProperties::FilteringProperties filtering,
                             unsigned int uiPortIncrease);
    void TimeoutProbeRequest(const Time& rTime);
    void ProcessRequestProbe(const StationURL& rStationURL);
    void StopProbes();
    void StartSendDummyPacket();
    void StopSendDummyPacket();
    int GetNumOfSendingProbes();
    void ClearProbeList();
    int GetURLPingTime(const StationURL& rStationURL);
    int GetCIDPingTime(unsigned int uiConnectionID);
    bool GetUpdatedURL(const StationURL& rStationURL, StationURL* pUpdatedURL);
    bool GetProbeFailed(const StationURL& rStationURL);
    NATTraversalResult GetNATTraversalResult(unsigned int uiConnectionID);
    void Execute();
    void ReceiveMessage(const StationURL& rStationURL, const unsigned char* pData, unsigned int uiSize);

private:
    friend class ConnectivityManager;
    friend class JobCheckAndRelay;
    friend class JobResolveRelayInfo;
    friend class Relay;
    friend class RVNATRelay;

    typedef std::map<unsigned int, std::pair<Time, StationURL>, std::less<unsigned int>,
                     MemAllocator<std::pair<const unsigned int, std::pair<Time, StationURL> > > >
        ProbeRequestMap;

    enum Msg
    {
        MSG_PROBE = 0,
        MSG_PROBE_REPLY = 1
    };

    bool StartNATStream(RootTransport* pTransport);
    bool StopNATStream();
    void StartSendProbe(const StationURL& rStationURL);
    void SendProbeRequests();
    void SendProbes();
    void SendProbe(Msg msg, const StationURL& rStationURL, Time time, unsigned char uiFlags);
    void SendDummyPacket(unsigned char uiFlags);
    void SendDummyPacket(const StationURL& rStationURL, unsigned char uiFlags);
    void SendDummyPacket(const InetAddress& rAddress);
    void CheckPortAndRelay(const qList<StationURL>& rLocalURLs, const StationURL& rStationURL);

    UnsupportedNATReferee m_unsupportedNATReferee;
    NATRelayInterface* m_pRelay;
    NATEchoInterface* m_pEcho;
    NATTraversalStream* m_pStream;
    unsigned int m_uiConnectionID;
    NATTransportAdapter* m_pTransportAdapter;
    qList<StationURL> m_pendingURLs;
    URLProbeList m_activeProbes;
    URLProbeList m_completedProbes;
    Time m_nextProbeTime;
    Job* m_pCheckAndRelayJob;
    CallContext m_callContext;
    bool m_bSendingDummyPackets;
    Time m_nextDummyPacketTime;
    unsigned int m_uiProbeRequestCount;
    ProbeRequestMap m_probeRequests;
    String m_relayServerAddress;
    unsigned short m_uiRelayServerPort;
    bool m_bUseRelayServer;
};

}
}

#endif

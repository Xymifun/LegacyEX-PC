#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATTRAVERSALPROTOCOL_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATTRAVERSALPROTOCOL_HPP

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/transport/ProtocolMessageReader.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"
#include "nn/pia/transport/StationLocation.hpp"
#include "nn/pia/nat/NatProbeList.hpp"
#include "nn/pia/nat/NatProbeRequestList.hpp"
#include "nn/pia/nat/NatTraversalTimeList.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

class NatTraverser;
class NatProbe;
class NatProbeRequest;

class NatTraversalProtocol : public transport::Protocol
{
public:
    enum
    {
        cProtocolType = 0x34
    };

    enum ProbeType
    {
        cProbeType0 = 0,
        cProbeType1 = 1
    };

    NatTraversalProtocol();
    virtual ~NatTraversalProtocol();

    Result StartupNatTraversal(NatTraverser* traverser);
    void CancelPortDetection();
    void CleanupNatTraversal();
    void AddProbeRequest(const NatProbeRequest& request);
    bool PrepareNatTraversal(const transport::StationLocation& location,
                             const transport::StationConnectionInfo& connection, common::CallContext* context,
                             bool passive, bool force);
    bool EraseProbeRequestByConnectionId(unsigned int connectionId);
    void ClearNatTraversal(const transport::StationLocation& location, bool relay);
    void ProcessProbeRequest(const transport::StationLocation& location);
    Result SendMessage(transport::ProtocolId protocolId, const common::StationAddress& address, const void* payload,
                       unsigned int payloadSize, bool relay, unsigned char type);
    void SendProbe(ProbeType type, const transport::StationLocation& location, common::Time time, unsigned char count);
    void ReceiveMessage(const transport::ProtocolMessageReader& reader);
    void SendDummyPacket(const transport::StationLocation& location, unsigned char type, unsigned char count);
    NatProbeRequest* FindProbeRequestByConnectionId(unsigned int connectionId);
    unsigned short GetPeer2PeerPort(const transport::StationLocation& location);
    void CompletedNatPortDetection(Result result);
    void DetectNatPort(const transport::StationLocation& local, const transport::StationLocation& remote);
    void RelayProbeRequest(const transport::StationLocation& source, const transport::StationLocation& destination);
    void SendProbeRequest(transport::StationLocation location, const NatProbeRequest& request);
    void ScheduleProbeRequest(const NatProbeRequest& request);
    bool SendProbeRequests();
    void SendProbes();
    Result dispatchProbe(common::Time& now);
    virtual Result Dispatch();
    bool GetLatestStationLocation(const transport::StationLocation& location, transport::StationLocation* latest);
    void StartServerKeepAlive();
    void StopServerKeepAlive();
    virtual void Trace(unsigned long long value) const;
    NatProbe* GetNatProbe(const transport::StationLocation& location);
    NatProbeList* GetNatProbeList() { return &m_probeList; }
    void ResetProbeStatus(const transport::StationLocation& location, bool relay);
    void ResetNextProbeTime();
    virtual unsigned short GetProtocolType() const;
    virtual unsigned int IsProtocolFilteringEnabled() const;

private:
    friend class NatTraverser;

    unsigned int m_localServiceConnectionId;
    transport::StationLocation m_localLocation;
    transport::StationLocation m_serverLocation;
    common::CallContext m_callContext;
    NatProbeList m_probeList;
    NatProbeRequestList m_edmProbeRequests;
    NatProbeRequestList m_probeRequests;
    NatTraversalTimeList m_traversalTimes;
    common::Time m_nextDispatchTime;
    common::Time m_pendingDetectionDeadline;
    bool m_fieldC58;
    bool m_fieldC59;
    bool m_fieldC5A;
    bool m_fieldC5B;
    bool m_fieldC5C;
    unsigned int m_pendingConnectionId;
    transport::StationLocation m_portDetectionLocation;
    bool m_serverKeepAliveEnabled;
    common::Time m_serverKeepAliveTime;
    NatTraverser* m_ownerNatTraverser;
    pead::Random m_random;
};

}
}
}

#endif

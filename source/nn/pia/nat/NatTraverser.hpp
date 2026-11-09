#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATTRAVERSER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATTRAVERSER_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/common/SimpleContainer.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class CallContext;
}
namespace inet
{
class IFacade;
class NexFacade;
}
namespace transport
{
class StationLocation;
class StationConnectionInfo;
}
namespace nat
{
class NatDetecter;
class NatPortDetecter;
class NatPropertyDetecter;
class NatTraversalProtocol;

class NatTraverser
{
public:
    NatTraverser();
    virtual ~NatTraverser();
    virtual Result Startup(inet::IFacade* facade, common::CallContext* context);
    virtual void Cleanup();
    virtual void Trace(unsigned long long value) const;
    void CompleteNatDetectionCommon();
    void CompletedNatPropertyDetection(bool success);
    Result StartNatPropertyDetection();
    void CompletedNatPortDetection(bool success);
    Result StartNatPortDetection();
    unsigned int GetNatMonitoringDataIndex(const transport::StationLocation& location);
    void IncrementNatTraversalSuccessCount(unsigned int index);
    void IncrementNatTraversalFailureCount(unsigned int index);
    void ReportNatTraversalResult(const transport::StationLocation& location, Result result, bool relay);
    bool PrepareNatTraversal(const transport::StationLocation& location,
                             const transport::StationConnectionInfo& connection, common::CallContext* context,
                             bool relay, bool passive, bool force);
    void ClearNatTraversal(const transport::StationLocation& location, bool relay);
    bool CheckLatestStationLocation(transport::StationLocation& location);
    Result CreateProtocols();
    void ClearNatMonitoringData(const transport::StationLocation& location);
    bool IsStartupCancelled();
    void RelayProbeRequest(const transport::StationLocation& source, const transport::StationLocation& destination,
                           bool relay, unsigned short port);
    void ProcessProbeRequest(const transport::StationLocation& location);
    bool IsTraversed(const transport::StationLocation& location) const;
    bool IsTraversed(unsigned int index) const;
    void SetLocalPublicLocation(const transport::StationLocation& location);
    NatPortDetecter* GetNatPortDetecter();
    unsigned int GetLocalServiceConnectionId() const;
    unsigned int GetRandomPortRangeMin() const;
    unsigned int GetRandomPortRangeMax() const;
    bool IsNatDetectionCompleted() const;
    void ResetProbeStatus(const transport::StationLocation& location, bool relay);
    void UpdateRelayTable();
    const common::InetAddress& GetRelayServerAddress() const;

private:
    friend class NatDetecter;
    friend class inet::NexFacade;
    friend class NatPortDetecter;
    friend class NatPropertyDetecter;
    friend class NatTraversalProtocol;

    struct NatProperty
    {
        NatProperty()
            : m_privatePort(0),
              m_publicPort(0),
              m_mappingType(0),
              m_filteringType(0),
              m_portDelta(0),
              m_flag(0)
        {
        }

        unsigned short m_privatePort;
        unsigned short m_publicPort;
        int m_mappingType;
        int m_filteringType;
        int m_portDelta;
        unsigned char m_flag;
    };

    common::CallContext* m_callContext;
    NatProperty m_natProperty;
    common::SimpleContainer<common::InetAddress, 4> m_servers;
    unsigned short m_protocolId;
    NatTraversalProtocol* m_protocol;
    unsigned char m_state;
    unsigned short m_publicPort;
    NatPortDetecter* m_natPortDetecter;
    NatPropertyDetecter* m_natPropertyDetecter;
    common::CallContext m_propertyContext;
    common::CallContext m_portContext;
    inet::IFacade* m_facade;
};

}
}
}
#endif

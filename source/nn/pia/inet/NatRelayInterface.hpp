#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NAT_RELAY_INTERFACE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NAT_RELAY_INTERFACE_HPP

#include "nn/pia/Common.hpp"

namespace nn
{
namespace nex
{
class RelaySignatureKey;
}
}

namespace nn
{
namespace pia
{
namespace common
{
class InetAddress;
}
namespace transport
{
class StationLocation;
}
}
}

namespace nn
{
namespace pia
{
namespace inet
{

class NatRelayInterface
{
public:
    virtual ~NatRelayInterface() {}
    virtual void RequestProbe(const transport::StationLocation& location) = 0;
    virtual void RelayProbeRequest(const transport::StationLocation& source,
                                   const transport::StationLocation& destination) = 0;
    virtual void ReportNatTraversalResult(unsigned int connectionId, bool success, unsigned int rtt) = 0;
    virtual void ReportNatProperties(unsigned int mapping, unsigned int filtering, unsigned int portIncrement) = 0;
    virtual void SetLocalCID(unsigned int connectionId) = 0;
    virtual void RequestRelayServerSettingAsync() = 0;
    virtual bool IsRequestRelayServerSettingCompleted() const = 0;
    virtual Result GetRequestRelayServerSettingResult() const = 0;
    virtual void ResolveRelayServerSetting() = 0;
    virtual const nex::RelaySignatureKey* GetRelayServerSetting() const = 0;
    virtual unsigned int IsRelayServerAvailable() const = 0;
    virtual void SetRelayServerAddress(const common::InetAddress& address) = 0;
    virtual const common::InetAddress& GetRelayServerAddress() const = 0;
    virtual unsigned int GetCurrentUTCEpochTime() const = 0;
    virtual unsigned int GetGameServerId() const = 0;
    virtual void SetRelayGatheringId(unsigned int gatheringId) = 0;
    virtual unsigned int GetRelayGatheringId() const = 0;
    virtual void SetRelayServerSignatureKey(unsigned char* key) = 0;
    virtual const unsigned char* GetRelayServerSignatureKey() const = 0;
    virtual void UpdateRelayTable() = 0;
    virtual bool RemoveRelayEntry(const transport::StationLocation& location) = 0;
    virtual void Cleanup() = 0;
    virtual void Trace(unsigned long long value) = 0;
    virtual Result Pack(void* buffer, const void* payload, unsigned int payloadSize, const common::InetAddress& source,
                        const common::InetAddress& destination) = 0;
    virtual Result Unpack(void* buffer, int* size, common::InetAddress* source) = 0;
};

}
}
}

#endif

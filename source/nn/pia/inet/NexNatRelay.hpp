#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNATRELAY_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNATRELAY_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/common/PooledList.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/inet/NatRelayInterface.hpp"
#include "nn/pia/inet/NexNatRelayInterface.hpp"

namespace nn
{
namespace nex
{
class InetAddress;
class ProtocolCallContext;
class RelaySignatureKey;
class StationURL;
template <typename T> class qList;
}
}

namespace nn
{
namespace pia
{
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

class NexNatRelay : public NatRelayInterface
{
public:
    enum
    {
        cRelayEntryNum = 64,
        cSignatureKeySize = 16
    };

    struct RelayEntry
    {
        unsigned int m_connectionId;
        common::Time m_expireTime;
        common::InetAddress m_stationAddress;
        common::InetAddress m_relayAddress;
    };

    NexNatRelay();
    virtual ~NexNatRelay();
    virtual void RequestProbe(const transport::StationLocation& location);
    virtual void RelayProbeRequest(const transport::StationLocation& source,
                                   const transport::StationLocation& destination);
    virtual void ReportNatTraversalResult(unsigned int connectionId, bool success, unsigned int rtt);
    virtual void ReportNatProperties(unsigned int mapping, unsigned int filtering, unsigned int portIncrement);
    virtual void SetLocalCID(unsigned int connectionId);
    virtual void RequestRelayServerSettingAsync();
    virtual bool IsRequestRelayServerSettingCompleted() const;
    virtual Result GetRequestRelayServerSettingResult() const;
    virtual void ResolveRelayServerSetting();
    virtual const nex::RelaySignatureKey* GetRelayServerSetting() const;
    virtual unsigned int IsRelayServerAvailable() const;
    virtual void SetRelayServerAddress(const common::InetAddress& address);
    virtual const common::InetAddress& GetRelayServerAddress() const;
    virtual unsigned int GetCurrentUTCEpochTime() const;
    virtual unsigned int GetGameServerId() const;
    virtual void SetRelayGatheringId(unsigned int gatheringId);
    virtual unsigned int GetRelayGatheringId() const;
    virtual void SetRelayServerSignatureKey(unsigned char* key);
    virtual const unsigned char* GetRelayServerSignatureKey() const;
    virtual void UpdateRelayTable();
    virtual bool RemoveRelayEntry(const transport::StationLocation& location);
    virtual void Cleanup();
    virtual void Trace(unsigned long long value);
    virtual Result Pack(void* buffer, const void* payload, unsigned int payloadSize, const common::InetAddress& source,
                        const common::InetAddress& destination);
    virtual Result Unpack(void* buffer, int* size, common::InetAddress* source);
    const common::InetAddress& GetRelayServerAddress(const common::InetAddress& stationAddress) const;

private:
    friend class NexFacade;

    typedef common::PooledList<RelayEntry> RelayTable;

    nex::qList<nex::StationURL>* m_targetUrlList;
    nex::StationURL* m_probeTargetUrl;
    nex::InetAddress* m_nexInetAddress;
    nex::ProtocolCallContext* m_relayCallContext;
    NexNatRelayInterface m_relayInterface;
    nex::ProtocolCallContext* m_signatureKeyCallContext;
    nex::RelaySignatureKey* m_relaySignatureKey;
    common::InetAddress m_relayServerAddress;
    common::Time m_relayServerAddressTime;
    unsigned int m_relayGatheringId;
    unsigned char m_relayServerSignatureKey[cSignatureKeySize];
    common::FixedPooledList<RelayEntry, cRelayEntryNum> m_relayTable;
};

}
}
}

#endif

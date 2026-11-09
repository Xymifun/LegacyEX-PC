#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SYNC_CLOCK_PROTOCOL_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SYNC_CLOCK_PROTOCOL_HPP

#include "nn/pia.h"
#include "nn/pia/common/ISerializable.hpp"
#include "nn/pia/common/LatestMedian.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/session/SyncClock.hpp"

namespace nn
{
namespace pia
{
namespace transport
{
class ProtocolMessageReader;
}
}
}

namespace nn
{
namespace pia
{
namespace session
{

class SyncClockProtocol : public transport::Protocol
{
public:
    enum
    {
        cProtocolType = 0x1c
    };

    class Data : public common::ISerializable
    {
    public:
        enum
        {
            cSerializedSize = 16,
            cResultInvalidArgument = 0x10c07
        };
        virtual ~Data();
        virtual Result Serialize(unsigned char* buffer, unsigned int* size) const;
        virtual Result Deserialize(const unsigned char* buffer);
        virtual unsigned int GetSerializedSize() const { return cSerializedSize; }

        unsigned long long m_requestTime;
        unsigned long long m_syncTime;
    };

    SyncClockProtocol();
    virtual unsigned short GetProtocolType() const;
    virtual Result StartupWithStationAddress(const common::StationAddress& address);
    virtual void CleanupWithStationAddress();
    virtual Result Dispatch();
    virtual Result UpdateProtocolEvent(const transport::ProtocolEvent& event);
    virtual void Trace(unsigned long long value) const;

    Result Request();
    Result Response(unsigned long long stationId, unsigned long long requestTime);
    void ProcessByHost(const transport::ProtocolMessageReader* reader);
    void ProcessByClient(const transport::ProtocolMessageReader* reader);

    SyncClock m_syncClock;
    StationIndex m_hostStationIndex;
    int m_pulseInterval;
    common::Time m_lastRequestTime;
    bool m_hostConnected;
    bool m_requestSent;
    common::LatestMedian<int, 10> m_latencyMedian;
};

}
}
}

#endif

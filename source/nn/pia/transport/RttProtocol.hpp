#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RTT_PROTOCOL_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RTT_PROTOCOL_HPP

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/common/ISerializable.hpp"
#include "nn/pia/transport/RttCalculator.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class StationAddress;
}
}
namespace pia
{
namespace transport
{

class ProtocolEvent;
class ProtocolMessageReader;

class RttProtocol : public Protocol
{
public:
    class Data : public common::ISerializable
    {
    public:
        virtual ~Data() {}

        virtual Result Serialize(unsigned char* buffer, unsigned int* size) const;
        virtual Result Deserialize(const unsigned char* buffer);
        virtual unsigned int GetSerializedSize() const;

        unsigned int m_type;
        unsigned int m_sequence;
        unsigned long long m_timestamp;
    };

    enum
    {
        cProtocolType = 0x58
    };

    RttProtocol();
    virtual ~RttProtocol();
    Result Initialize(unsigned short stationCount, int initialPulseInterval);
    void Finalize();
    virtual Result StartupWithStationAddress(const common::StationAddress& address);
    virtual void CleanupWithStationAddress();
    Result send(StationIndex stationIndex, const Data& data);
    Result request(StationIndex stationIndex);
    void UpdateLastRequestedTimeAll();
    void process(const ProtocolMessageReader* reader);
    virtual Result Dispatch();
    virtual Result UpdateProtocolEvent(const ProtocolEvent& event);
    int GetRtt(StationIndex stationIndex) const;
    int GetRtt(StationIndex stationIndex, unsigned int count) const;
    virtual void Trace(unsigned long long value) const;
    void ResetRttMinMax(StationIndex stationIndex);
    int GetRttMin(StationIndex stationIndex) const;
    int GetRttMax(StationIndex stationIndex) const;
    virtual unsigned short GetProtocolType() const;

    int GetLatestRtt(StationIndex stationIndex) const;
    int GetRttSamplingNum(StationIndex stationIndex) const;
    static Result SetPulseInterval(int interval);
    static Result SetInitialPulseInterval(int interval);
    static int GetPulseInterval();
    static int GetInitialPulseInterval();

private:
    unsigned char m_data[16];
    unsigned int m_stationIndex;
    unsigned long long m_stationId;
    RttCalculator* m_rttCalculators;
    unsigned short m_stationCount;
    unsigned char m_isBroadcastSupported;
};

}
}
}

#endif

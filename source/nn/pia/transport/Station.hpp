#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/String.hpp"
#include "nn/pia/transport/SequenceIdController.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class NetworkFactory;
class StationProtocol;
class RttProtocol;
class ReliableSlidingWindow;
class ConnectStationJob;
class DisconnectStationJob;
class ProcessConnectionRequestJob;
class StationLocation;
class StationConnectionInfo;
class StationConnectionInfoTable;

class Station
{
public:
    struct IdentificationToken;
    struct PlayerInfo;
    enum StationState
    {
    };
    Station();
    virtual ~Station();
    Result Initialize(NetworkFactory* networkFactory, StationProtocol* stationProtocol, RttProtocol* rttProtocol);
    void Finalize();
    void SetStationIdAndIndex(unsigned long long id, ::nn::pia::StationIndex index);
    void helperStartupCleanup(bool startup, unsigned long long id, ::nn::pia::StationIndex index,
                              const common::StationAddress& address, StationState state);
    bool Startup(unsigned long long id, ::nn::pia::StationIndex index, const common::StationAddress& address);
    bool Startup(const common::StationAddress& address);
    bool Startup();
    void Cleanup();
    int GetRtt() const;
    int GetRtt(unsigned int index) const;
    int GetLatestRtt() const;
    void ResetRttMinMax();
    int GetRttMin() const;
    int GetRttMax() const;
    void CleanupJobs();
    static void SetRttPulseInterval(int interval);
    static int GetRttPulseInterval();
    static void SetInitialRttPulseInterval(int interval);
    static int GetInitialRttPulseInterval();
    int GetRttSamplingNum() const;
    void GetIdentificationToken(IdentificationToken* token);
    void GetPlayerInfo(unsigned char index, PlayerInfo* info);
    Result GetPrincipalId(unsigned long long* principalId);
    Result GetPlayerCount(unsigned char* count);
    Result GetParticipantCount(unsigned char* count);
    bool IsConnectionRouteRelay() const;
    bool IsConnectionRouteDirect() const;
    common::StationAddress& GetStationAddress() { return m_stationAddress; }
    StationIndex GetStationIndex() const { return static_cast<StationIndex>(m_field10); }
    unsigned char m_field0;
    common::StationAddress m_stationAddress;
    unsigned int m_field10;
    unsigned int m_field14;
    unsigned long long m_stationId;
    unsigned int m_field20;
    ReliableSlidingWindow* m_reliableSlidingWindow;
    StationProtocol* m_field28;
    RttProtocol* m_rttProtocol;
    ConnectStationJob* m_connectStationJob;
    DisconnectStationJob* m_disconnectStationJob;
    ProcessConnectionRequestJob* m_processConnectionRequestJob;
    SequenceIdController m_sequenceIdController;
    unsigned char m_field54;
    unsigned char m_field55;
    unsigned char m_field56;
    common::Time m_lastSendTime;
    common::Time m_lastReceiveTime;
    unsigned char m_field68;
    virtual void Trace(unsigned long long value) const;
};

struct Station::IdentificationToken
{
    char m_token[32];
};

struct Station::PlayerInfo
{
    enum
    {
        cLanguageUnset = 0xff
    };

    PlayerInfo()
    {
        m_name.Clear();
        m_language = cLanguageUnset;
        m_field70 = 0;
        m_field78 = 0;
    }
    ~PlayerInfo();

    common::FixedString<20> m_name;
    unsigned char m_language;
    unsigned long long m_field70;
    unsigned long long m_field78;
};

}
}
}

#endif

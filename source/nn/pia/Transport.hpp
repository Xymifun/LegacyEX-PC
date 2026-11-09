#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_HPP
namespace nn
{
namespace pia
{
namespace transport
{

enum TransportResultCode
{
    cResultOutOfMemory = 0x10c01,
    cResultAlreadyInitialized = 0x10c02,
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cResultNotInitialized = 0x10c0c,
    cResultAlreadyExists = 0x10c0f,
    cResultTransportUnavailable = 0x4c11,
    cResultWindowOverflow = 0x4c0d,
    cResultDataSizeInvalid = 0x2c03,
    cResultDataNotReady = 0x2c09,
    cResultTimeout = 0x6c0e,
    cResultConnectionFailed = 0xc23,
    cResultConnectionRefused = 0xc24,
    cResultConnectionFailed10c25 = 0x10c25
};

enum TransportStationIndex
{
    cStationIndexInvalid = 0xfd,
    cStationIndexReserved = 0xfe,
    cStationIndexBroadcast = 0xff
};

enum TransportStationNum
{
    cStationNumMin = 2,
    cStationNumMax = 32,
    cStationNumLimit = 24
};

}
}
}
#include <stdint.h>
#include "nn/pia.h"
#include "nn/pia/ProtocolManager.hpp"
#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/common/CryptoSetting.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/session/Session.hpp"
namespace nn
{
namespace pia
{
namespace common
{

class Job;
class IPacketInput;
class IPacketOutput;
}
}
}
namespace nn
{
namespace pia
{
namespace transport
{
class NetworkFactory;
class PacketHandler;
class RelayRouteManager;
class Transport
{
public:
    class Setting
    {
    public:
        Setting(NetworkFactory* networkFactory, unsigned short stationNum, unsigned int sendPacketNum,
                unsigned int receivePacketNum, int analysisInterval, bool keepAliveEnabled, bool monitoringEnabled)
            : m_networkFactory(networkFactory),
              m_stationNum(stationNum),
              m_sendPacketNum(sendPacketNum),
              m_receivePacketNum(receivePacketNum),
              m_analysisInterval(analysisInterval),
              m_keepAliveEnabled(keepAliveEnabled),
              m_monitoringEnabled(monitoringEnabled)
        {
        }

        NetworkFactory* m_networkFactory;
        unsigned short m_stationNum;
        unsigned int m_sendPacketNum;
        unsigned int m_receivePacketNum;
        int m_analysisInterval;
        bool m_keepAliveEnabled;
        bool m_monitoringEnabled;
    };
    Transport();
    ~Transport();
    static Result CreateInstance(const Setting& setting);
    static void DestroyInstance();
    Result initialize(const Setting& setting);
    void finalize();
    Result Startup(unsigned int payloadSize, const common::CryptoSetting* cryptoSetting);
    Result UpdateSetting(const common::CryptoSetting* cryptoSetting);
    Result dispatch();
    void updateTransportAnalyzer();
    common::IPacketInput* GetInputStream();
    common::IPacketOutput* GetOutputStream();
    NetworkFactory* GetNetworkFactory();
    unsigned int GenerateConnectionId() const;
    void OutputStreamUpdateEvent();
    void Cleanup();
    Result Restartup(const common::CryptoSetting* cryptoSetting);
    Result SetKeepAliveInterval(int interval);
    int GetKeepAliveInterval() const;
    void EnableKeepAlive();
    void DisableKeepAlive();
    bool IsKeepAliveEnabled() const;
    Result GetLocalStationConnectionInfo(StationConnectionInfo* stationConnectionInfo);
    Result GetHostStationConnectionInfo(StationConnectionInfo* stationConnectionInfo);
    void SetMonitoringNetworkRtt(bool isSessionBegin);
    Result ConvertToStationIndex(StationIndex*, uint64_t) const;
    StationIndex ConvertToStationIndexUnsafe(uint64_t stationId) const;
    Result ConvertToStationId(uint64_t* stationId, StationIndex stationIndex) const;
    uint64_t ConvertToStationIdUnsafe(StationIndex stationIndex) const;
    Result SetTransportStatus(Result transportStatus);
    void Trace(unsigned long long value) const;

    common::Time m_time;
    NetworkFactory* m_networkFactory;
    ProtocolManager m_protocolManager;
    PacketHandler* m_packetHandler;
    unsigned short m_stationNum;
    bool m_startedUp;
    bool m_connectionRouteRelay;
    Result m_connectionError;
    common::Job* m_dispatchJob;
    unsigned int m_connectionState;
    RelayRouteManager* m_relayRouteManager;
    int m_analysisInterval;
    common::Time m_lastAnalysisTime;
    bool m_keepAliveEnabled;
    bool m_monitoringEnabled;
    unsigned int m_dispatchCount;
    session::Session::DisconnectionEventListener m_disconnectionEventListener;
    StationIdTable* m_stationIdTable;
    unsigned int m_stationConnectionState;
    Result m_transportStatus;
    uint64_t m_localStationId;
};
extern Transport* g_piaTransport;
StationIndex Conv2StationIndex(uint64_t);
}
}
}
#endif

#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_CONNECTION_INFO_TABLE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_CONNECTION_INFO_TABLE_HPP

#include "nn/pia.h"
#include "nn/pia/Common.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class StationConnectionInfoTable
{
public:
    StationConnectionInfoTable();
    ~StationConnectionInfoTable();
    static Result CreateInstance();
    static void DestroyInstance();
    void Finalize();
    Result Initialize(unsigned int count);
    Result AddToTable(const Station* station, const StationConnectionInfo& info);
    Result GetStationConnectionInfo(const Station* station, StationConnectionInfo* info) const;
    Result GetStationConnectionInfo(unsigned long long principalId, StationConnectionInfo* info) const;
    Station* GetStation(const StationConnectionInfo& info) const;
    Station* GetStation(const StationLocation& location) const;
    Station* GetStation(const common::StationAddress& address) const;
    Station* GetStation(unsigned int stationIndex) const;
    Station* GetStation(unsigned long long principalId) const;
    Station* GetReconnectingStation(const StationConnectionInfo& info) const;
    unsigned int GetStationIndexByPrincipalID(unsigned long long principalId) const;
    unsigned long long GetPrincipalIdByStation(const Station* station) const;
    void EraseFromTable(const Station* station);
    void ClearTable();
    void Trace(unsigned long long value) const;
    StationConnectionInfo& GetLocalStationConnectionInfo() { return m_localStationConnectionInfo; }
    StationConnectionInfo& GetHostStationConnectionInfo() { return m_hostStationConnectionInfo; }
    common::StationAddress& GetHostStationAddress() { return m_hostStationAddress; }

    StationConnectionInfo m_localStationConnectionInfo;
    StationConnectionInfo m_hostStationConnectionInfo;
    common::StationAddress m_hostStationAddress;

private:
    StationConnectionInfo* m_infos;
    Station* m_stations[32];
    unsigned int m_count;
    mutable common::CriticalSection m_criticalSection;
};

extern StationConnectionInfoTable* g_stationConnectionInfoTable;

}
}
}

#endif

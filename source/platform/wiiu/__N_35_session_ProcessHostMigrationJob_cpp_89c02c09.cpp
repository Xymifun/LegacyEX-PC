#include "platform/wiiu/__N_35_session_ProcessHostMigrationJob_cpp_89c02c09.hpp"
#include <nn/pia.h>
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"

unsigned int __N_35_session_ProcessHostMigrationJob_cpp_89c02c09::UpdateHostStationConnectionInfo(
    nn::pia::transport::Station* station)
{
    nn::pia::transport::StationConnectionInfoTable* table = nn::pia::transport::g_stationConnectionInfoTable;
    nn::pia::transport::StationConnectionInfo connectionInfo;
    if (table->GetStationConnectionInfo(station, &connectionInfo) != 0)
    {
        return 0;
    }
    table->m_hostStationConnectionInfo = connectionInfo;
    table->m_hostStationAddress = station->m_stationAddress;
    return 1;
}

#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATIONMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATIONMANAGER_HPP

#include "nn/pia.h"
#include "nn/pia/common/PooledList.hpp"
#include "nn/pia/common/SimpleContainer.hpp"
#include <cstddef>

namespace nn
{
namespace pia
{
namespace transport
{
class StationManager
{
public:
    enum
    {
        cStationNumMax = 32
    };
    StationManager();
    ~StationManager();
    static Result CreateInstance();
    void Finalize();
    static void DestroyInstance();
    Result Initialize(NetworkFactory*, unsigned int, StationProtocol*, RttProtocol*);
    Station* CreateStation();
    Station* CreateLocalStation();
    bool isInStationList(const Station*) const;
    bool isInPtrArray(const common::SimpleContainer<Station*, cStationNumMax>&, const Station*) const;
    void DestroyStation(Station*);
    void DestroyStation(StationIndex);
    Station* GetStation(StationIndex);
    const Station* GetStation(StationIndex) const;
    Station* GetStation(unsigned long long);
    const Station* GetStation(unsigned long long) const;
    Station* GetStation(const common::StationAddress&);
    const Station* GetStation(const common::StationAddress&) const;
    Station* GetStationByAddress(const common::InetAddress&);
    const Station* GetStationByAddress(const common::InetAddress&) const;
    Station* GetStationByStationIdWithoutConvert(unsigned long long);
    const Station* GetStationByStationIdWithoutConvert(unsigned long long) const;
    Result GetStationAddress(common::StationAddress*, StationIndex) const;
    Result GetStationAddress(common::StationAddress*, unsigned long long) const;
    unsigned short GetValidStationNum() const;
    unsigned int GetParticipatingStationBitmap(bool) const;
    void Trace(unsigned long long) const;

public:
    typedef common::PooledList<Station> StationList;

    StationList m_stationList;
    unsigned char* m_stationStorage;
    common::SimpleContainer<Station*, cStationNumMax> m_stations;
    common::SimpleContainer<Station*, cStationNumMax> m_freeStations;
    Station* m_localStation;
    StationIndex m_invalidStationIndex;

    static StationManager* s_pInstance;
};
}
}
}
#endif

#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RELAY_ROUTE_MANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RELAY_ROUTE_MANAGER_HPP

#include "nn/pia.h"

namespace nn
{
namespace pia
{
namespace transport
{

class RelayRouteManager
{
public:
    enum RelayFailureReason
    {
        cRelayFailureReasonNoRtt = 1,
        cRelayFailureReasonRttTooLarge = 2,
        cRelayFailureReasonRelayLimit = 3
    };

    RelayRouteManager();
    ~RelayRouteManager();
    Result Initialize(unsigned int stationNum, bool relayEmulationEnabled);
    void Finalize();
    void CalcStationType(unsigned int stationBitmap, unsigned int* stayStationBitmap, unsigned int* leaveStationBitmap);
    void ProcLeaveStation(unsigned int leaveStationBitmap);
    void CalcRelayNumTable();
    bool IsRelayEnabled(unsigned int source, unsigned int relay, unsigned int destination,
                        RelayFailureReason* failureReason);
    bool SwitchRelay(unsigned int relayStation, unsigned int stationBitmap, unsigned short rttLimit,
                     unsigned short relayNumLimit);
    bool CalcRelayRouteWithSwitchingRelay(unsigned int source, unsigned int destination, unsigned int stationBitmap);
    bool DecideBrokenRouteRelayStation(unsigned int stationBitmap, unsigned int* refusedStationBitmap);
    void ProcStayStation(unsigned int stayStationBitmap, unsigned int* refusedStationBitmap);
    void Trace(unsigned long long value) const;
    bool ProcNewStationOne(unsigned int station, unsigned int stationBitmap);
    void ProcNewStation(unsigned int newStationBitmap, unsigned int* refusedStationBitmap,
                        unsigned int* acceptedStationBitmap);
    bool RelayRouteOptimizationRTTOne(unsigned int stationBitmap);
    void RelayRouteOptimizationRTT(unsigned int stationBitmap);
    bool RelayRouteOptimizationRelayNumOne(unsigned int stationBitmap);
    void RelayRouteOptimizationRelayNum(unsigned int stationBitmap);
    void RelayRouteOptimization(unsigned int stationBitmap);
    void ProcRefusedStation(unsigned int refusedStationBitmap, unsigned int acceptedStationBitmap);
    void UpdateJoiningStationBitmap();
    Result UpdateDirectConnectionData(const unsigned char* rttBuffer, unsigned int rttBufferSize,
                                      unsigned int stationBitmap, unsigned int* refusedStationBitmap,
                                      ::nn::pia::StationIndex stationIndex, unsigned char* failureReasons);
    Result WriteRelayRouteDirectionsToBuffer(unsigned char* buffer, unsigned int size, unsigned int* writtenSize) const;
    Result ReadRelayRouteDirectionsFromBuffer(const unsigned char* buffer, unsigned int size);
    unsigned int GetRelayRouteDirectionsSize() const;
    Result GetRelayRoute(::nn::pia::StationIndex source, ::nn::pia::StationIndex destination,
                         ::nn::pia::StationIndex* relay) const;
    Result SetRelayRoute(::nn::pia::StationIndex source, ::nn::pia::StationIndex destination,
                         ::nn::pia::StationIndex relay);
    Result GetDestStationList(::nn::pia::StationIndex source, ::nn::pia::StationIndex relay,
                              unsigned int* destinationBitmap) const;
    Result GetDirectStationList(::nn::pia::StationIndex source, unsigned int* directBitmap) const;
    void SetRttMax(unsigned short rttMax);
    void SetRelayNumMax(unsigned short relayNumMax);
    void UpdateRelayTable(unsigned int stationBitmap);
    Result SearchRefugeRelayRoute(::nn::pia::StationIndex station);
    void InitRelayRouteManagerData();
    Result GetOriginalRelayRoute(::nn::pia::StationIndex source, ::nn::pia::StationIndex destination,
                                 ::nn::pia::StationIndex* relay) const;
    Result GetOriginalDirectStationList(::nn::pia::StationIndex source, unsigned int* directBitmap) const;

    unsigned short* m_rttTable;
    unsigned char* m_routeTable;
    unsigned char* m_relayNumTable;
    unsigned int* m_directStationBitmaps;
    unsigned char* m_relayEmulationTable;
    unsigned char* m_failureReasonTable;
    unsigned short m_rttMax;
    unsigned short m_relayNumMax;
    unsigned int m_unknown1c;
    unsigned int m_unknown20;
    unsigned int m_unknown24;
    unsigned char m_routeBitCount;
    unsigned char m_invalidRoute;
    unsigned int m_joiningStationBitmap;
    unsigned int m_unknown30;
    bool m_relayEmulationEnabled;
    unsigned char* m_originalRouteTable;
};

}
}
}

#endif

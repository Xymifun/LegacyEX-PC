#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATIONPROTOCOLMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATIONPROTOCOLMANAGER_HPP

#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class RttProtocol;
class StationProtocol;

class StationProtocolManager
{
public:
    StationProtocolManager();
    ~StationProtocolManager();
    static Result CreateInstance();
    static void DestroyInstance();
    Result Initialize(unsigned short stationNum, int initialPulseInterval);
    void Finalize();
    StationProtocol* GetStationProtocol();
    RttProtocol* GetRttProtocol();
    void Trace(unsigned long long value) const;

private:
    unsigned short m_stationProtocolId;
    unsigned short m_rttProtocolId;
};

extern StationProtocolManager* g_piaStationProtocolManager;

}
}
}

#endif

#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CONNECTIVITYMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CONNECTIVITYMANAGER_HPP

#include <stdint.h>

#include "nn/nex/PseudoSingleton.hpp"

namespace nn
{
namespace nex
{

class CallContext;
class ConnectivityTester;
class NATTransportAdapter;
class StationProbeList;

class ConnectivityManager : public PseudoSingleton
{
public:
    enum
    {
        ClassID = 11
    };

    ConnectivityManager();
    virtual ~ConnectivityManager();

    static ConnectivityManager* GetInstance();
    static ConnectivityManager* GetInstanceWithoutNetworkCheck();

    bool RegisterConnectivityTester(ConnectivityTester* connectivityTester);
    bool UnregisterConnectivityTester();
    bool RegisterTransportAdapter(NATTransportAdapter* transportAdapter);
    bool UnregisterTransportAdapter();

    bool StartNATSession();
    bool StartNATSession(CallContext* callContext);
    bool StartNATSessionInternal(CallContext* callContext);
    void OnStartNATSession(CallContext* callContext);
    bool StopNATSession();
    bool IsNATSessionStarted();
    bool CheckUnupdateNATPort();

    bool ProbeStations(CallContext* callContext, StationProbeList* stationProbeList, unsigned int timeout,
                       bool useRelay);
    bool InitiateProbes(StationProbeList& stationProbeList);

    void SetProbeLifetime(int probeLifetime);
    void SetProbeKeepAlive(int probeKeepAlive);

    static void SetRelayPublicAddress();
    static void SetSuspendBossDaemonOnNATSession(bool suspend);
    static bool GetSuspendBossDaemonOnNATSession();

    friend class JobResolveRelayInfo;

protected:
    bool m_natSessionStarted;
    bool m_natTraversalEnabled;
    unsigned short m_portNumber;
    unsigned short m_reservedPortNumber;
    ConnectivityTester* m_connectivityTester;
    uint32_t m_callContextId;
    uint32_t m_probeLifetime;
    int32_t m_bossDaemonSuspendCount;
    bool m_packetBufferManagerInitialized;

    static bool s_suspendBossDaemonOnNATSession;
    static bool s_enableRelayOnNATSession;
};

}
}

#endif

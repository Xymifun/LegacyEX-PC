#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHMESHLAYERCONTROLLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHMESHLAYERCONTROLLER_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn
{
namespace pia
{
namespace session
{
class IMatchmakeSession;
}
}
}

namespace nn
{
namespace pia
{
namespace lan
{

class LanMatchMeshLayerController : public session::MeshLayerController
{
public:
    LanMatchMeshLayerController();
    virtual ~LanMatchMeshLayerController();
    virtual Result Startup(const session::Session::StartupSetting& setting);
    virtual void Cleanup();
    virtual Result MeshStartup(session::IMatchmakeSession* matchmakeSession, bool isHost);
    virtual void MeshCleanup();
    virtual Result CheckConnectedStatus();
    virtual void Trace(unsigned long long value) const;
    virtual unsigned int GetMatchmakeSessionTotalStations() const;
    virtual unsigned short GetMatchmakeSessionTotalParticipants() const;
    virtual bool IsParticipated(unsigned int sessionId, unsigned long long principalId) const;

    void SetConnectionStatusDisconnected();

private:
    session::IMatchmakeSession* m_matchmakeSession;
    session::Session::ConnectionStatus m_connectionStatus;
};

}
}
}

#endif

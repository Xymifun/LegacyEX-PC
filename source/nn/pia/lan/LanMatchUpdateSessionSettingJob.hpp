#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHUPDATESESSIONSETTINGJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHUPDATESESSIONSETTINGJOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/session/UpdateSessionSettingJob.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanMatchmakeSession;

class LanMatchUpdateSessionSettingJob : public session::UpdateSessionSettingJob
{
public:
    LanMatchUpdateSessionSettingJob();
    virtual ~LanMatchUpdateSessionSettingJob();
    virtual Result StartupImpl(unsigned int sessionId, session::IMatchmakeSession* matchmakeSession,
                               const session::UpdateSessionSetting* setting, bool isJointSession);
    ExecuteResult UpdateSessionSetting();
    ExecuteResult WaitUpdateSessionSetting();
    virtual void Cleanup();
    virtual void Trace(unsigned long long value) const;

private:
    LanMatchmakeSession* m_matchmakeSession;
    bool m_isJointSession;
};

}
}
}

#endif

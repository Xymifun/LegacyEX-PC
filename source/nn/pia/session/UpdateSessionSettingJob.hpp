#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_UPDATE_SESSION_SETTING_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_UPDATE_SESSION_SETTING_JOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class IMatchmakeSession;
class UpdateSessionSetting;

class UpdateSessionSettingJob : public common::StepSequenceJob
{
public:
    UpdateSessionSettingJob();
    virtual ~UpdateSessionSettingJob();
    virtual void Trace(unsigned long long value) const;

    Result Startup(common::CallContext* callerContext, unsigned int sessionId, IMatchmakeSession* matchmakeSession,
                   const UpdateSessionSetting* setting, bool isJointSession);
    ExecuteResult CompleteProcess();
    virtual void Cleanup();
    virtual Result StartupImpl(unsigned int sessionId, IMatchmakeSession* matchmakeSession,
                               const UpdateSessionSetting* setting, bool isJointSession) = 0;
    virtual void SetCurrentSessionOpenStatus(Session::SessionOpenStatus status);
    virtual void SetBufferSessionOpenStatus(Session::SessionOpenStatus status);

    common::CallContext* m_callerContext;
    common::CallContext m_callContext;
    unsigned int m_sessionId;
};


}
}
}

#endif

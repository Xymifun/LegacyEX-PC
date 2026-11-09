#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHUPDATESESSIONSETTINGJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHUPDATESESSIONSETTINGJOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/session/UpdateSessionSettingJob.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexMatchmakeSession;

class NexMatchUpdateSessionSettingJob : public session::UpdateSessionSettingJob
{
public:
    NexMatchUpdateSessionSettingJob();
    virtual ~NexMatchUpdateSessionSettingJob();
    virtual Result StartupImpl(unsigned int sessionId, session::IMatchmakeSession* matchmakeSession,
                               const session::UpdateSessionSetting* setting, bool isJointSession);
    ExecuteResult UpdateSessionSetting();
    ExecuteResult WaitUpdateSessionSetting();
    ExecuteResult ModifyAttribute();
    ExecuteResult WaitModifyAttribute();
    ExecuteResult UpdateAttributes();
    ExecuteResult WaitUpdateAttributes();
    ExecuteResult UpdateApplicationData();
    ExecuteResult WaitUpdateApplicationData();
    virtual void Cleanup();
    virtual void Trace(unsigned long long value) const;

private:
    enum
    {
        cAttributeNum = 6,
        cApplicationDataSizeMax = 512
    };

    NexMatchmakeSession* m_matchmakeSession;
    unsigned int m_attributes[cAttributeNum];
    bool m_isAttributeUpdated[cAttributeNum];
    unsigned char m_applicationData[cApplicationDataSizeMax];
    unsigned int m_applicationDataSize;
    bool m_isJointSession;
};

}
}
}

#endif

#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHCREATESESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHCREATESESSIONJOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/session/CreateSessionJob.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanMatchmakeSession;

class LanMatchCreateSessionJob : public session::CreateSessionJob
{
public:
    LanMatchCreateSessionJob();
    virtual ~LanMatchCreateSessionJob();
    ExecuteResult CreateMatchmakeSession();
    ExecuteResult WaitCreateMatchmake();
    ExecuteResult OpenParticipate();
    ExecuteResult WaitOpenParticipate();
    ExecuteResult UnregisterGathering();
    ExecuteResult WaitUnregisterGathering();
    virtual void Cleanup();
    virtual void Trace(unsigned long long value) const;

protected:
    virtual Result StartupImpl(const session::CreateSessionSetting* setting);
    virtual void SetNextStepAtWaitCreateMesh();
    virtual ExecuteResult SetNextStepAtFailedCreateMesh(Result result);

private:
    Result m_result;
    LanMatchmakeSession* m_matchmakeSession;
    bool m_isOpenSession;
};

}
}
}

#endif

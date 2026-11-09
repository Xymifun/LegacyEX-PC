#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHCREATESESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHCREATESESSIONJOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/session/CreateSessionJob.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexMatchmakeSession;

class NexMatchCreateSessionJob : public session::CreateSessionJob
{
public:
    NexMatchCreateSessionJob();
    virtual ~NexMatchCreateSessionJob();
    virtual void Cleanup();
    virtual void Trace(unsigned long long value) const;
    ExecuteResult LeaveJoinedMatchmakeSession();
    ExecuteResult WaitLeaveJoinedMatchmakeSession();
    ExecuteResult CreateMatchmakeSession();
    ExecuteResult WaitCreateMatchmake();
    ExecuteResult WaitNotification();
    ExecuteResult StartNatSession();
    ExecuteResult WaitStartNatSession();
    ExecuteResult UnregisterGathering();
    ExecuteResult WaitUnregisterGathering();

protected:
    virtual Result StartupImpl(const session::CreateSessionSetting* setting);
    virtual void SetNextStepAtWaitCreateMesh();
    virtual ExecuteResult SetNextStepAtFailedCreateMesh(Result result);

private:
    enum
    {
        cJoinedSessionIdMax = 4
    };

    NexMatchmakeSession* m_matchmakeSession;
    unsigned int m_joinedSessionIds[cJoinedSessionIdMax];
    unsigned int m_joinedSessionIdNum;
    common::Time m_notificationDeadline;
    bool m_isOpenSession;
    common::Time m_createMatchmakeStartTime;
};

}
}
}

#endif

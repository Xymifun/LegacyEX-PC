#include "nn/pia/lan/LanMatchLeaveSessionJob.hpp"

#include "nn/pia/session/Session.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

LanMatchLeaveSessionJob::LanMatchLeaveSessionJob() : session::LeaveSessionJob()
{
}

LanMatchLeaveSessionJob::~LanMatchLeaveSessionJob()
{
}

bool LanMatchLeaveSessionJob::IsNeedSkipLeaveCurrentMatchmakeSession()
{
    return session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] == 0;
}

void LanMatchLeaveSessionJob::Trace(unsigned long long) const
{
}

}
}
}

#include "nn/pia/session/CommunityManagementJob.hpp"
#include "nn/pia/Transport.hpp"

namespace nn
{
namespace pia
{
namespace session
{

CommunityManagementJob::CommunityManagementJob() : common::StepSequenceJob(), m_callerContext(0), m_callContext()
{
}

CommunityManagementJob::~CommunityManagementJob()
{
}

Result CommunityManagementJob::BrowseCommunityStartup(common::CallContext* callerContext,
                                                      const CommunitySearchCriteria* criteria)
{
    if (callerContext == 0 || criteria == 0)
        return static_cast<Result>(transport::cResultInvalidArgument);
    const Result result = BrowseCommunityStartupImpl(criteria);
    if (result != 0)
        return result;
    m_callerContext = callerContext;
    callerContext->InitiateCall();
    Reset(true);
    return 0;
}

Result CommunityManagementJob::CreateCommunityStartup(common::CallContext* callerContext,
                                                      const CreateCommunitySetting* setting)
{
    if (callerContext == 0 || setting == 0)
        return static_cast<Result>(transport::cResultInvalidArgument);
    const Result result = CreateCommunityStartupImpl(setting);
    if (result != 0)
        return result;
    m_callerContext = callerContext;
    callerContext->InitiateCall();
    Reset(true);
    return 0;
}

Result CommunityManagementJob::JoinCommunityStartup(common::CallContext* callerContext,
                                                    const JoinCommunitySetting* setting)
{
    if (callerContext == 0)
        return static_cast<Result>(transport::cResultInvalidArgument);
    const Result result = JoinCommunityStartupImpl(setting);
    if (result != 0)
        return result;
    m_callerContext = callerContext;
    callerContext->InitiateCall();
    Reset(true);
    return 0;
}

Result CommunityManagementJob::UpdateCommunityStartup(common::CallContext* callerContext,
                                                      const UpdateCommunitySetting* setting)
{
    if (callerContext == 0)
        return static_cast<Result>(transport::cResultInvalidArgument);
    const Result result = UpdateCommunityStartupImpl(setting);
    if (result != 0)
        return result;
    m_callerContext = callerContext;
    callerContext->InitiateCall();
    Reset(true);
    return 0;
}

Result CommunityManagementJob::DestroyCommunityStartup(common::CallContext* callerContext, unsigned int communityId)
{
    if (callerContext == 0)
        return static_cast<Result>(transport::cResultInvalidArgument);
    const Result result = DestroyCommunityStartupImpl(communityId);
    if (result != 0)
        return result;
    m_callerContext = callerContext;
    callerContext->InitiateCall();
    Reset(true);
    return 0;
}

Result CommunityManagementJob::LeaveCommunityStartup(common::CallContext* callerContext, unsigned int communityId)
{
    if (callerContext == 0)
        return static_cast<Result>(transport::cResultInvalidArgument);
    const Result result = LeaveCommunityStartupImpl(communityId);
    if (result != 0)
        return result;
    m_callerContext = callerContext;
    callerContext->InitiateCall();
    Reset(true);
    return 0;
}

void CommunityManagementJob::Cleanup()
{
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
    }
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();
    m_callContext.Reset();
}

void CommunityManagementJob::Trace(unsigned long long) const
{
}

}
}
}

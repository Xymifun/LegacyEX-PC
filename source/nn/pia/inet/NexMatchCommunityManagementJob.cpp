#include "nn/pia/inet/NexMatchCommunityManagementJob.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/inet/NexCommunitySearchCriteriaCommunityId.hpp"
#include "nn/pia/inet/NexCommunitySearchCriteriaOfficial.hpp"
#include "nn/pia/inet/NexCommunitySearchCriteriaOwner.hpp"
#include "nn/pia/inet/NexCommunitySearchCriteriaParticipant.hpp"
#include "nn/pia/inet/NexCreateCommunitySetting.hpp"
#include "nn/pia/inet/NexJoinCommunitySetting.hpp"
#include "nn/pia/inet/NexMatchmakeSession.hpp"
#include "nn/pia/inet/NexUpdateCommunitySetting.hpp"
#include "nn/pia/session/CommunitySearchCriteria.hpp"
#include "nn/pia/session/ICommunityInfoList.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{
typedef common::Job::ExecuteResult ExecuteResult;

enum CommunityKeyType
{
    cCommunityKeyTypeCommunityId = 1,
    cCommunityKeyTypeParticipant = 2,
    cCommunityKeyTypeOwner = 3,
    cCommunityKeyTypeOfficial = 4
};

inline void SelectStep(common::StepSequenceJob::Step& slot, ExecuteResult (NexMatchCommunityManagementJob::*function)(),
                       const char* name)
{
    const common::StepSequenceJob::Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

NexMatchCommunityManagementJob::NexMatchCommunityManagementJob()
    : session::CommunityManagementJob(),
      m_communityId(0),
      m_principalId(0),
      m_resultRangeOffset(0),
      m_resultRangeSize(0),
      m_isAvailableCommunityOnly(true)
{
}

NexMatchCommunityManagementJob::~NexMatchCommunityManagementJob()
{
}

void NexMatchCommunityManagementJob::Cleanup()
{
    session::CommunityManagementJob::Cleanup();
    m_communityId = 0;
}

void NexMatchCommunityManagementJob::Trace(unsigned long long) const
{
}

Result NexMatchCommunityManagementJob::BrowseCommunityStartupImpl(const session::CommunitySearchCriteria* criteria)
{
    m_matchmakeSession = static_cast<NexMatchmakeSession*>(
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]);
    if (m_matchmakeSession == 0)
        return static_cast<Result>(transport::cResultInvalidState);
    switch (criteria->GetKeyType())
    {
    case cCommunityKeyTypeCommunityId:
    {
        const NexCommunitySearchCriteriaCommunityId* communityIdCriteria =
            static_cast<const NexCommunitySearchCriteriaCommunityId*>(criteria);
        if (communityIdCriteria->GetCommunityIdListSize() > session::g_piaSession->m_communityInfoList->GetMaxSize())
            return static_cast<Result>(transport::cResultInvalidArgument);
        m_matchmakeSession->ReflectCommunityIdList(communityIdCriteria->GetCommunityIdList(),
                                                   communityIdCriteria->GetCommunityIdListSize());
        SelectStep(m_currentStep, &NexMatchCommunityManagementJob::FindCommunityByCommunityId,
                   "NexMatchCommunityManagementJob::FindCommunityByCommunityId");
        break;
    }
    case cCommunityKeyTypeParticipant:
        m_principalId =
            static_cast<const NexCommunitySearchCriteriaParticipant*>(criteria)->GetParticipantPrincipalId();
        m_resultRangeOffset = criteria->GetResultRangeOffset();
        m_resultRangeSize = criteria->GetResultRangeSize();
        SelectStep(m_currentStep, &NexMatchCommunityManagementJob::FindCommunityByParticipant,
                   "NexMatchCommunityManagementJob::FindCommunityByParticipant");
        break;
    case cCommunityKeyTypeOwner:
        m_principalId = static_cast<const NexCommunitySearchCriteriaOwner*>(criteria)->GetOwnerPrincipalId();
        m_resultRangeOffset = criteria->GetResultRangeOffset();
        m_resultRangeSize = criteria->GetResultRangeSize();
        SelectStep(m_currentStep, &NexMatchCommunityManagementJob::FindCommunityByOwner,
                   "NexMatchCommunityManagementJob::FindCommunityByOwner");
        break;
    case cCommunityKeyTypeOfficial:
        m_isAvailableCommunityOnly =
            static_cast<const NexCommunitySearchCriteriaOfficial*>(criteria)->IsAvailableCommunityOnly();
        m_resultRangeOffset = criteria->GetResultRangeOffset();
        m_resultRangeSize = criteria->GetResultRangeSize();
        SelectStep(m_currentStep, &NexMatchCommunityManagementJob::FindOfficialCommunity,
                   "NexMatchCommunityManagementJob::FindOfficialCommunity");
        break;
    default:
        return static_cast<Result>(transport::cResultInvalidArgument);
    }
    return 0;
}

Result NexMatchCommunityManagementJob::CreateCommunityStartupImpl(const session::CreateCommunitySetting* setting)
{
    m_matchmakeSession = static_cast<NexMatchmakeSession*>(
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]);
    if (m_matchmakeSession == 0)
        return static_cast<Result>(transport::cResultInvalidState);
    const Result result = m_matchmakeSession->ReflectCreateCommunitySetting(
        static_cast<const NexCreateCommunitySetting*>(setting));
    if (result != 0)
        return result;
    SelectStep(m_currentStep, &NexMatchCommunityManagementJob::CreateCommunity,
               "NexMatchCommunityManagementJob::CreateCommunity");
    return 0;
}

Result NexMatchCommunityManagementJob::JoinCommunityStartupImpl(const session::JoinCommunitySetting* setting)
{
    m_matchmakeSession = static_cast<NexMatchmakeSession*>(
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]);
    if (m_matchmakeSession == 0)
        return static_cast<Result>(transport::cResultInvalidState);
    const Result result = m_matchmakeSession->ReflectJoinCommunitySetting(
        static_cast<const NexJoinCommunitySetting*>(setting));
    if (result != 0)
        return result;
    SelectStep(m_currentStep, &NexMatchCommunityManagementJob::JoinCommunity,
               "NexMatchCommunityManagementJob::JoinCommunity");
    return 0;
}

Result NexMatchCommunityManagementJob::UpdateCommunityStartupImpl(const session::UpdateCommunitySetting* setting)
{
    m_matchmakeSession = static_cast<NexMatchmakeSession*>(
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]);
    if (m_matchmakeSession == 0)
        return static_cast<Result>(transport::cResultInvalidState);
    const Result result = m_matchmakeSession->ReflectUpdateCommunitySetting(
        static_cast<const NexUpdateCommunitySetting*>(setting));
    if (result != 0)
        return result;
    SelectStep(m_currentStep, &NexMatchCommunityManagementJob::UpdateCommunity,
               "NexMatchCommunityManagementJob::UpdateCommunity");
    return 0;
}

Result NexMatchCommunityManagementJob::DestroyCommunityStartupImpl(unsigned int communityId)
{
    m_matchmakeSession = static_cast<NexMatchmakeSession*>(
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]);
    if (m_matchmakeSession == 0)
        return static_cast<Result>(transport::cResultInvalidState);
    m_communityId = communityId;
    SelectStep(m_currentStep, &NexMatchCommunityManagementJob::DestroyCommunity,
               "NexMatchCommunityManagementJob::DestroyCommunity");
    return 0;
}

Result NexMatchCommunityManagementJob::LeaveCommunityStartupImpl(unsigned int communityId)
{
    m_matchmakeSession = static_cast<NexMatchmakeSession*>(
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]);
    if (m_matchmakeSession == 0)
        return static_cast<Result>(transport::cResultInvalidState);
    m_communityId = communityId;
    SelectStep(m_currentStep, &NexMatchCommunityManagementJob::LeaveCommunity,
               "NexMatchCommunityManagementJob::LeaveCommunity");
    return 0;
}

NexMatchCommunityManagementJob::ExecuteResult NexMatchCommunityManagementJob::FindCommunityByCommunityId()
{
    const Result result = m_matchmakeSession->FindCommunityByGatheringId(&m_callContext);
    if (result != 0)
    {
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &NexMatchCommunityManagementJob::WaitFindCommunityByCommunityId,
               "NexMatchCommunityManagementJob::WaitFindCommunityByCommunityId");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCommunityManagementJob::ExecuteResult NexMatchCommunityManagementJob::WaitFindCommunityByCommunityId()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedNetwork);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown6);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown7);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown8);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (m_matchmakeSession->WaitFindCommunityByGatheringId())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            m_callerContext->SignalSuccess(0);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCommunityManagementJob::ExecuteResult NexMatchCommunityManagementJob::FindCommunityByParticipant()
{
    const Result result = m_matchmakeSession->FindCommunityByParticipant(&m_callContext, m_principalId,
                                                                         m_resultRangeOffset, m_resultRangeSize);
    if (result != 0)
    {
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &NexMatchCommunityManagementJob::WaitFindCommunityByParticipant,
               "NexMatchCommunityManagementJob::WaitFindCommunityByParticipant");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCommunityManagementJob::ExecuteResult NexMatchCommunityManagementJob::WaitFindCommunityByParticipant()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedNetwork);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown6);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown7);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown8);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (m_matchmakeSession->WaitFindCommunityByParticipant())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            m_callerContext->SignalSuccess(0);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCommunityManagementJob::ExecuteResult NexMatchCommunityManagementJob::FindOfficialCommunity()
{
    const Result result = m_matchmakeSession->FindOfficialCommunity(&m_callContext, m_isAvailableCommunityOnly,
                                                                    m_resultRangeOffset, m_resultRangeSize);
    if (result != 0)
    {
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &NexMatchCommunityManagementJob::WaitFindOfficialCommunity,
               "NexMatchCommunityManagementJob::WaitFindOfficialCommunity");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCommunityManagementJob::ExecuteResult NexMatchCommunityManagementJob::WaitFindOfficialCommunity()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedNetwork);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown6);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown7);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown8);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (m_matchmakeSession->WaitFindOfficialCommunity())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            m_callerContext->SignalSuccess(0);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCommunityManagementJob::ExecuteResult NexMatchCommunityManagementJob::FindCommunityByOwner()
{
    const Result result = m_matchmakeSession->FindCommunityByOwner(&m_callContext, m_principalId, m_resultRangeOffset,
                                                                   m_resultRangeSize);
    if (result != 0)
    {
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &NexMatchCommunityManagementJob::WaitFindCommunityByOwner,
               "NexMatchCommunityManagementJob::WaitFindCommunityByOwner");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCommunityManagementJob::ExecuteResult NexMatchCommunityManagementJob::WaitFindCommunityByOwner()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedNetwork);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown6);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown7);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown8);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (m_matchmakeSession->WaitFindCommunityByOwner())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            m_callerContext->SignalSuccess(0);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCommunityManagementJob::ExecuteResult NexMatchCommunityManagementJob::CreateCommunity()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedNetwork);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown6);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown7);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown8);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    const Result result = m_matchmakeSession->CreateCommunity(&m_callContext);
    if (result != 0)
    {
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &NexMatchCommunityManagementJob::WaitCreateCommunity,
               "NexMatchCommunityManagementJob::WaitCreateCommunity");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCommunityManagementJob::ExecuteResult NexMatchCommunityManagementJob::WaitCreateCommunity()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedNetwork);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown6);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown7);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown8);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (m_matchmakeSession->WaitCreateCommunity())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            m_callerContext->SignalSuccess(0);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCommunityManagementJob::ExecuteResult NexMatchCommunityManagementJob::JoinCommunity()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedNetwork);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown6);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown7);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown8);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    const Result result = m_matchmakeSession->JoinCommunity(&m_callContext);
    if (result != 0)
    {
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &NexMatchCommunityManagementJob::WaitJoinCommunity,
               "NexMatchCommunityManagementJob::WaitJoinCommunity");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCommunityManagementJob::ExecuteResult NexMatchCommunityManagementJob::WaitJoinCommunity()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedNetwork);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown6);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown7);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown8);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (m_matchmakeSession->WaitJoinCommunity())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            m_callerContext->SignalSuccess(0);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCommunityManagementJob::ExecuteResult NexMatchCommunityManagementJob::DestroyCommunity()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedNetwork);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown6);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown7);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown8);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    const Result result = m_matchmakeSession->UnregisterMatchmakeSession(&m_callContext, m_communityId);
    if (result != 0)
    {
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &NexMatchCommunityManagementJob::WaitDestroyCommunity,
               "NexMatchCommunityManagementJob::WaitDestroyCommunity");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCommunityManagementJob::ExecuteResult NexMatchCommunityManagementJob::WaitDestroyCommunity()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedNetwork);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown6);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown7);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown8);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (m_matchmakeSession->WaitUnregisterMatchmakeSession())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            m_callerContext->SignalSuccess(0);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCommunityManagementJob::ExecuteResult NexMatchCommunityManagementJob::LeaveCommunity()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedNetwork);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown6);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown7);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown8);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    const Result result = m_matchmakeSession->LeaveMatchmakeSession(&m_callContext, m_communityId);
    if (result != 0)
    {
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &NexMatchCommunityManagementJob::WaitLeaveCommunity,
               "NexMatchCommunityManagementJob::WaitLeaveCommunity");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCommunityManagementJob::ExecuteResult NexMatchCommunityManagementJob::WaitLeaveCommunity()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedNetwork);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown6);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown7);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown8);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (m_matchmakeSession->WaitLeaveMatchmakeSession())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            m_callerContext->SignalSuccess(0);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCommunityManagementJob::ExecuteResult NexMatchCommunityManagementJob::UpdateCommunity()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedNetwork);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown6);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown7);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown8);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    const Result result = m_matchmakeSession->UpdateCommunity(&m_callContext);
    if (result != 0)
    {
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &NexMatchCommunityManagementJob::WaitUpdateCommunity,
               "NexMatchCommunityManagementJob::WaitUpdateCommunity");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCommunityManagementJob::ExecuteResult NexMatchCommunityManagementJob::WaitUpdateCommunity()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedNetwork);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown6);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown7);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusUnknown8);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (m_matchmakeSession->WaitUpdateCommunity())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            m_callerContext->SignalSuccess(0);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

}
}
}

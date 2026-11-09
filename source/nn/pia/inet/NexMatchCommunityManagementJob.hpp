#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHCOMMUNITYMANAGEMENTJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHCOMMUNITYMANAGEMENTJOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/session/CommunityManagementJob.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexMatchmakeSession;

class NexMatchCommunityManagementJob : public session::CommunityManagementJob
{
public:
    NexMatchCommunityManagementJob();
    virtual ~NexMatchCommunityManagementJob();
    virtual void Cleanup();
    virtual void Trace(unsigned long long value) const;
    ExecuteResult FindCommunityByCommunityId();
    ExecuteResult WaitFindCommunityByCommunityId();
    ExecuteResult FindCommunityByParticipant();
    ExecuteResult WaitFindCommunityByParticipant();
    ExecuteResult FindOfficialCommunity();
    ExecuteResult WaitFindOfficialCommunity();
    ExecuteResult FindCommunityByOwner();
    ExecuteResult WaitFindCommunityByOwner();
    ExecuteResult CreateCommunity();
    ExecuteResult WaitCreateCommunity();
    ExecuteResult JoinCommunity();
    ExecuteResult WaitJoinCommunity();
    ExecuteResult DestroyCommunity();
    ExecuteResult WaitDestroyCommunity();
    ExecuteResult LeaveCommunity();
    ExecuteResult WaitLeaveCommunity();
    ExecuteResult UpdateCommunity();
    ExecuteResult WaitUpdateCommunity();

protected:
    virtual Result BrowseCommunityStartupImpl(const session::CommunitySearchCriteria* criteria);
    virtual Result CreateCommunityStartupImpl(const session::CreateCommunitySetting* setting);
    virtual Result JoinCommunityStartupImpl(const session::JoinCommunitySetting* setting);
    virtual Result UpdateCommunityStartupImpl(const session::UpdateCommunitySetting* setting);
    virtual Result DestroyCommunityStartupImpl(unsigned int communityId);
    virtual Result LeaveCommunityStartupImpl(unsigned int communityId);

private:
    NexMatchmakeSession* m_matchmakeSession;
    unsigned int m_communityId;
    unsigned long long m_principalId;
    unsigned int m_resultRangeOffset;
    unsigned int m_resultRangeSize;
    bool m_isAvailableCommunityOnly;
};

}
}
}

#endif

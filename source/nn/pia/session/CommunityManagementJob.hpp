#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_COMMUNITY_MANAGEMENT_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_COMMUNITY_MANAGEMENT_JOB_HPP

#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/CallContext.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class CommunitySearchCriteria;
class CreateCommunitySetting;
class JoinCommunitySetting;
class UpdateCommunitySetting;

class CommunityManagementJob : public common::StepSequenceJob
{
public:
    CommunityManagementJob();
    virtual ~CommunityManagementJob();

    Result BrowseCommunityStartup(common::CallContext* callerContext, const CommunitySearchCriteria* criteria);
    Result CreateCommunityStartup(common::CallContext* callerContext, const CreateCommunitySetting* setting);
    Result JoinCommunityStartup(common::CallContext* callerContext, const JoinCommunitySetting* setting);
    Result UpdateCommunityStartup(common::CallContext* callerContext, const UpdateCommunitySetting* setting);
    Result DestroyCommunityStartup(common::CallContext* callerContext, unsigned int communityId);
    Result LeaveCommunityStartup(common::CallContext* callerContext, unsigned int communityId);
    virtual void Cleanup();
    virtual void Trace(unsigned long long value) const;

protected:
    virtual Result BrowseCommunityStartupImpl(const CommunitySearchCriteria* criteria) = 0;
    virtual Result CreateCommunityStartupImpl(const CreateCommunitySetting* setting) = 0;
    virtual Result JoinCommunityStartupImpl(const JoinCommunitySetting* setting) = 0;
    virtual Result UpdateCommunityStartupImpl(const UpdateCommunitySetting* setting) = 0;
    virtual Result DestroyCommunityStartupImpl(unsigned int communityId) = 0;
    virtual Result LeaveCommunityStartupImpl(unsigned int communityId) = 0;

    common::CallContext* m_callerContext;
    common::CallContext m_callContext;
};

}
}
}

#endif

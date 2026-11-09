#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_IMATCHMAKE_SESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_IMATCHMAKE_SESSION_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/session/ISessionInfo.hpp"
#include "nn/pia/session/ISessionInfoList.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class CallContext;
}
}
}
namespace nn
{
namespace pia
{
namespace transport
{
class StationConnectionInfo;
}
}
}

namespace nn
{
namespace pia
{
namespace session
{

class ICommunityInfoList;

class IMatchmakeSession
{
public:
    virtual ~IMatchmakeSession() {}
    virtual void Cleanup() = 0;
    virtual Result RandomMatchmakeSession(common::CallContext* callContext) = 0;
    virtual bool WaitRandomMatchmakeSession(unsigned int* sessionId, bool* isNewSession) = 0;
    virtual Result BrowseMatchmakeSession(common::CallContext* callContext) = 0;
    virtual bool WaitBrowseMatchmakeSession() = 0;
    virtual ISessionInfoList* GetBrowseMatchmakeSessionResult() = 0;
    virtual Result CreateMatchmakeSession(common::CallContext* callContext) = 0;
    virtual bool WaitCreateMatchmakeSession(unsigned int* sessionId) = 0;
    virtual Result JoinMatchmakeSession(common::CallContext* callContext, unsigned int sessionId) = 0;
    virtual bool WaitJoinMatchmakeSession() = 0;
    virtual Result LeaveMatchmakeSession(common::CallContext* callContext, unsigned int sessionId) = 0;
    virtual bool WaitLeaveMatchmakeSession() = 0;
    virtual Result UnregisterMatchmakeSession(common::CallContext* callContext, unsigned int sessionId) = 0;
    virtual bool WaitUnregisterMatchmakeSession() = 0;
    virtual Result OpenParticipate(common::CallContext* callContext, unsigned int sessionId) = 0;
    virtual bool WaitOpenParticipate() = 0;
    virtual Result CloseParticipate(common::CallContext* callContext, unsigned int sessionId) = 0;
    virtual bool WaitCloseParticipate() = 0;
    virtual Result GetStationConnectionInfo(common::CallContext* callContext, unsigned int sessionId) = 0;
    virtual bool WaitGetStationConnectionInfo(transport::StationConnectionInfo* stationConnectionInfo) = 0;
    virtual Result RequestSessionInfo(common::CallContext* callContext, unsigned int sessionId) = 0;
    virtual bool WaitRequestSessionInfo() = 0;
    virtual unsigned long long ConvertGatheringIdToPersistentGatheringCode(unsigned int gatheringId) = 0;
    virtual unsigned int ConvertPersistentGatheringCodeToGatheringId(unsigned long long persistentGatheringCode) = 0;
    virtual ICommunityInfoList* GetBrowsedCommunityResult() = 0;
    virtual Result UpdateMatchmakeSessionHost(unsigned int sessionId) = 0;
    virtual bool IsMatchmakeOwner() const = 0;
    virtual void SetMatchmakeOwner(bool isOwner) = 0;
    virtual unsigned long long GetMatchmakeOwnerUniqueId() const = 0;
    virtual void SetMatchmakeOwnerUniqueId(unsigned long long uniqueId) = 0;
    virtual Result UpdateSelectionPriority(unsigned int sessionId, unsigned char selectionPriority) = 0;
    virtual unsigned int IsUpdatableSelectionPriority(unsigned char selectionPriority) = 0;
    virtual void UpdateStationInfoList() = 0;
    virtual const ISessionInfo* GetSessionInfo() const = 0;
    virtual ISessionInfo* GetSessionInfo() = 0;
};

}
}
}

#endif

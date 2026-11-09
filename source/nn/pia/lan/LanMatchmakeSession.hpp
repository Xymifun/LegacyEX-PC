#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHMAKESESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHMAKESESSION_HPP

#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/lan/LanSessionInfo.hpp"
#include "nn/pia/lan/LanSessionInfoPrivate.hpp"
#include "nn/pia/lan/LanSessionSearchCriteria.hpp"
#include "nn/pia/session/CommonMatchmakeSession.hpp"
#include "nn/pia/session/ISessionInfoList.hpp"
#include "nn/pia/session/SessionSearchCriteria.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class CreateSessionSetting;
class JoinSessionSetting;
class UpdateSessionSetting;

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
namespace lan
{

class LanMatchmakeSession : public session::CommonMatchmakeSession
{
public:
    LanMatchmakeSession();
    virtual ~LanMatchmakeSession();
    virtual void Cleanup();
    virtual Result RandomMatchmakeSession(common::CallContext* callContext);
    virtual bool WaitRandomMatchmakeSession(unsigned int* gatheringId, bool* isNewSession);
    virtual Result BrowseMatchmakeSession(common::CallContext* callContext);
    virtual bool WaitBrowseMatchmakeSession();
    virtual session::ISessionInfoList* GetBrowseMatchmakeSessionResult();
    virtual Result CreateMatchmakeSession(common::CallContext* callContext);
    virtual bool WaitCreateMatchmakeSession(unsigned int* gatheringId);
    virtual Result JoinMatchmakeSession(common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitJoinMatchmakeSession();
    virtual Result LeaveMatchmakeSession(common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitLeaveMatchmakeSession();
    virtual Result UnregisterMatchmakeSession(common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitUnregisterMatchmakeSession();
    virtual Result OpenParticipate(common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitOpenParticipate();
    virtual Result CloseParticipate(common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitCloseParticipate();
    virtual Result GetStationConnectionInfo(common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitGetStationConnectionInfo(transport::StationConnectionInfo* stationConnectionInfo);
    virtual Result RequestSessionInfo(common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitRequestSessionInfo();
    virtual Result UpdateMatchmakeSessionHost(unsigned int stationId);
    virtual void UpdateStationInfoList();
    virtual const session::ISessionInfo* GetSessionInfo() const;
    virtual session::ISessionInfo* GetSessionInfo();
    virtual Result UpdateSessionSetting(common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitUpdateSessionSetting();
    virtual Result InquiryOwnerPrincipalId(common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitInquiryOwnerPrincipalId();
    virtual Result CancelInquiryOwnerPrincipalId();

    void Clear();
    void SetSessionParam(const unsigned char* sessionParam, unsigned int sessionParamSize);
    unsigned int GetGatheringId() const;
    LanSessionInfo* GetSessionInfoPtr();
    LanSessionInfo* GetSessionInfoBufferPtr();
    session::ISessionInfoList* GetSessionInfoListPtr();
    LanSessionSearchCriteria* GetSessionSearchCriteriaPtr(unsigned char index);
    void ReflectSettingToMatchmakeSession(const session::CreateSessionSetting* setting);
    void ReflectSettingToMatchmakeSession(const session::JoinSessionSetting* setting);
    void ReflectSettingToMatchmakeSession(const session::UpdateSessionSetting* setting, bool open);
    unsigned int RegisterSearchCriteria(const session::SessionSearchCriteria* criteria, unsigned char count);
    void UnregisterSearchCriteria();
    void SetIgnoredSessionId(unsigned int sessionId);
    void SetPreferredSessionId(unsigned int sessionId);
    void CopySessionInfo();
    bool IsSessionParamRegistered() const;
    void Trace(unsigned long long value) const;

private:
    LanSessionInfoPrivate m_sessionInfo;
    LanSessionInfoPrivate m_sessionInfoBuffer;
    LanSessionSearchCriteria m_searchCriteria[2];
    unsigned char m_searchCriteriaCount;
    session::ISessionInfoList* m_sessionInfoList;
    unsigned int m_gatheringId;
    unsigned int m_ignoredSessionId;
    unsigned int m_preferredSessionId;
    common::CallContext* m_callContext;
    common::CallContext* m_inquiryCallContext;
    common::CallContext m_ownedCallContext;
    LanSessionInfo m_publishedSessionInfo;
    bool m_sessionParamRegistered;
};

}
}
}

#endif

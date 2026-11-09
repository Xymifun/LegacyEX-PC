#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXSESSIONINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXSESSIONINFO_HPP

#include <stdint.h>
#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/common/DateTime.hpp"
#include "nn/pia/inet/SessionType.hpp"
#include "nn/pia/session/ISessionInfo.hpp"

namespace nn
{
namespace pia
{
namespace inet
{
class NexSessionInfo : public session::ISessionInfo
{
public:
    NexSessionInfo();
    virtual ~NexSessionInfo();
    virtual uint32_t GetGameMode() const;
    virtual uint32_t GetSessionId() const;
    virtual uint16_t GetCurrentParticipantNum() const;
    virtual uint16_t GetMinParticipantNum() const;
    virtual uint16_t GetMaxParticipantNum() const;
    virtual bool IsOpened() const;
    virtual Result GetApplicationData(void* buffer, unsigned int bufferSize) const;
    virtual uint32_t GetApplicationDataSize() const;
    virtual void Clear();
    virtual void Trace(unsigned long long traceId) const;
    virtual uint32_t GetCommunityId() const;
    virtual Result GetAttribute(unsigned char index, unsigned int* value) const;
    virtual uint8_t IsRestrictedByUserPassword() const;
    virtual uint8_t IsRestrictedBySystemPassword() const;
    virtual uint32_t GetSessionType() const;
    virtual uint8_t IsAutoCloseSession() const;
    virtual unsigned long long GetHostPrincipalId() const;
    virtual unsigned long long GetTargetPrincipalId() const;
    virtual uint8_t GetSelectionPriority() const;
    virtual const common::DateTime& GetStartedTime() const;
    virtual void SetGameMode(uint32_t gameMode);
    virtual void SetSessionId(uint32_t sessionId);
    virtual void SetCurrentParticipantNum(uint16_t participantNum);
    virtual void SetMinParticipantNum(uint16_t participantNum);
    virtual void SetMaxParticipantNum(uint16_t participantNum);
    virtual void SetOpen(bool open);
    virtual void SetAttribute(uint8_t index, uint32_t value);
    virtual void SetApplicationData(const void* data, unsigned int size);
    virtual void SetUserPasswordEnabled(bool enabled);
    virtual void SetSystemPasswordEnabled(bool enabled);
    virtual void SetSessionType(SessionType sessionType, bool autoCloseSession);
    virtual void SetHostPrincipalId(unsigned long long principalId);
    virtual void SetTargetPrincipalId(unsigned long long principalId);
    virtual void SetSelectionPriority(uint8_t priority);
    virtual void SetStartedTime(const common::DateTime& time);
    virtual void SetCommunityId(uint32_t communityId);
    void Copy(const NexSessionInfo& other);
    NexSessionInfo(const NexSessionInfo& other);
    NexSessionInfo& operator=(const NexSessionInfo& other);

private:
    uint32_t game_mode_;
    uint32_t session_id_;
    uint16_t current_participant_num_;
    uint16_t min_participant_num_;
    uint16_t max_participant_num_;
    bool opened_;
    uint32_t attributes_[6];
    unsigned char application_data_[512];
    uint32_t application_data_size_;
    bool user_password_enabled_;
    bool system_password_enabled_;
    SessionType session_type_;
    bool auto_close_session_;
    uint64_t host_principal_id_;
    uint64_t target_principal_id_;
    uint8_t selection_priority_;
    common::DateTime started_time_;
    uint32_t community_id_;
};
}
}
}

#endif

#include "nn/pia/inet/NexSessionInfo.hpp"
#include <coreinit/memory.h>
#include <cstring>

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{
enum
{
    cResultInvalidArgument = 0x10c07,
    cResultBufferTooSmall = 0x10c04
};
}

NexSessionInfo::NexSessionInfo()
    : game_mode_(0),
      session_id_(0),
      current_participant_num_(0),
      min_participant_num_(0),
      max_participant_num_(0),
      opened_(false),
      user_password_enabled_(false),
      system_password_enabled_(false),
      session_type_(cSessionType_Normal),
      auto_close_session_(false),
      host_principal_id_(0),
      target_principal_id_(0),
      selection_priority_(0),
      started_time_(),
      community_id_(0)
{
    OSBlockSet(application_data_, 0, sizeof(application_data_));
    application_data_size_ = 0;
    started_time_.Clear();
    OSBlockSet(attributes_, 0, sizeof(attributes_));
}

NexSessionInfo::~NexSessionInfo()
{
}

uint32_t NexSessionInfo::GetGameMode() const
{
    return game_mode_;
}

uint32_t NexSessionInfo::GetSessionId() const
{
    return session_id_;
}

uint32_t NexSessionInfo::GetCommunityId() const
{
    return community_id_;
}

uint16_t NexSessionInfo::GetCurrentParticipantNum() const
{
    return current_participant_num_;
}

uint16_t NexSessionInfo::GetMinParticipantNum() const
{
    return min_participant_num_;
}

uint16_t NexSessionInfo::GetMaxParticipantNum() const
{
    return max_participant_num_;
}

bool NexSessionInfo::IsOpened() const
{
    return opened_;
}

Result NexSessionInfo::GetApplicationData(void* buffer, unsigned int bufferSize) const
{
    if (buffer == NULL)
    {
        Result result(cResultInvalidArgument);
        return result;
    }
    else if (bufferSize < application_data_size_)
    {
        Result result(cResultBufferTooSmall);
        return result;
    }
    else
    {
        OSBlockMove(buffer, application_data_, application_data_size_, false);
        return 0;
    }
}

uint32_t NexSessionInfo::GetApplicationDataSize() const
{
    return application_data_size_;
}

Result NexSessionInfo::GetAttribute(unsigned char index, unsigned int* value) const
{
    if (value == NULL)
    {
        return Result(cResultInvalidArgument);
    }
    else if (index >= 6)
    {
        Result result(cResultInvalidArgument);
        return result;
    }
    else
    {
        *value = attributes_[index];
        return Result(0);
    }
}

uint8_t NexSessionInfo::IsRestrictedByUserPassword() const
{
    return user_password_enabled_;
}

uint8_t NexSessionInfo::IsRestrictedBySystemPassword() const
{
    return system_password_enabled_;
}

uint32_t NexSessionInfo::GetSessionType() const
{
    return session_type_;
}

uint8_t NexSessionInfo::IsAutoCloseSession() const
{
    return auto_close_session_;
}

unsigned long long NexSessionInfo::GetHostPrincipalId() const
{
    return host_principal_id_;
}

unsigned long long NexSessionInfo::GetTargetPrincipalId() const
{
    return target_principal_id_;
}

uint8_t NexSessionInfo::GetSelectionPriority() const
{
    return selection_priority_;
}

const common::DateTime& NexSessionInfo::GetStartedTime() const
{
    return started_time_;
}

void NexSessionInfo::Clear()
{
    community_id_ = 0;
    game_mode_ = 0;
    session_id_ = 0;
    current_participant_num_ = 0;
    min_participant_num_ = 0;
    max_participant_num_ = 0;
    opened_ = false;
    started_time_.Clear();
    OSBlockSet(attributes_, 0, sizeof(attributes_));
    OSBlockSet(application_data_, 0, sizeof(application_data_));
    application_data_size_ = 0;
    user_password_enabled_ = false;
    system_password_enabled_ = false;
    session_type_ = cSessionType_Normal;
    auto_close_session_ = false;
    host_principal_id_ = 0;
    target_principal_id_ = 0;
    selection_priority_ = 0;
}

void NexSessionInfo::Trace(unsigned long long traceId) const
{
    (void)traceId;
}

void NexSessionInfo::SetGameMode(uint32_t gameMode)
{
    game_mode_ = gameMode;
}

void NexSessionInfo::SetSessionId(uint32_t sessionId)
{
    session_id_ = sessionId;
}

void NexSessionInfo::SetCurrentParticipantNum(uint16_t participantNum)
{
    current_participant_num_ = participantNum;
}

void NexSessionInfo::SetMinParticipantNum(uint16_t participantNum)
{
    min_participant_num_ = participantNum;
}

void NexSessionInfo::SetMaxParticipantNum(uint16_t participantNum)
{
    max_participant_num_ = participantNum;
}

void NexSessionInfo::SetOpen(bool open)
{
    opened_ = open;
}

void NexSessionInfo::SetAttribute(uint8_t index, uint32_t value)
{
    if (index < 6)
        attributes_[index] = value;
}

void NexSessionInfo::SetApplicationData(const void* data, unsigned int size)
{
    if (data != NULL && size <= sizeof(application_data_))
    {
        OSBlockSet(application_data_, 0, sizeof(application_data_));
        OSBlockMove(application_data_, data, size, false);
        application_data_size_ = size;
    }
}

void NexSessionInfo::SetUserPasswordEnabled(bool enabled)
{
    user_password_enabled_ = enabled;
}

void NexSessionInfo::SetSystemPasswordEnabled(bool enabled)
{
    system_password_enabled_ = enabled;
}

void NexSessionInfo::SetSessionType(SessionType sessionType, bool autoCloseSession)
{
    session_type_ = sessionType;
    auto_close_session_ = autoCloseSession;
}

void NexSessionInfo::SetHostPrincipalId(unsigned long long principalId)
{
    host_principal_id_ = principalId;
}

void NexSessionInfo::SetTargetPrincipalId(unsigned long long principalId)
{
    target_principal_id_ = principalId;
}

void NexSessionInfo::SetSelectionPriority(uint8_t priority)
{
    selection_priority_ = priority;
}

void NexSessionInfo::SetStartedTime(const common::DateTime& time)
{
    started_time_ = time;
}

void NexSessionInfo::SetCommunityId(uint32_t communityId)
{
    community_id_ = communityId;
}

}
}
}

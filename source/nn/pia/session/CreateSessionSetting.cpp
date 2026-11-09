#include "nn/pia/session/CreateSessionSetting.hpp"

namespace nn
{
namespace pia
{
namespace session
{

CreateSessionSetting::CreateSessionSetting()
{
    mMinParticipantNum = 0;
    mMinParticipantNumSet = 0;
    mMaxParticipantNum = 0;
    mMaxParticipantNumSet = 0;
}

CreateSessionSetting::~CreateSessionSetting()
{
}

uint16_t CreateSessionSetting::GetMinParticipantNum() const
{
    return mMinParticipantNum;
}

uint8_t CreateSessionSetting::IsSetMinParticipantNum() const
{
    return mMinParticipantNumSet;
}

void CreateSessionSetting::SetMaxParticipantNum(uint16_t value)
{
    mMaxParticipantNum = value;
    mMaxParticipantNumSet = 1;
}

uint16_t CreateSessionSetting::GetMaxParticipantNum() const
{
    return mMaxParticipantNum;
}

uint8_t CreateSessionSetting::IsSetMaxParticipantNum() const
{
    return mMaxParticipantNumSet;
}

void CreateSessionSetting::Reset()
{
    mMinParticipantNum = 0;
    mMinParticipantNumSet = 0;
    mMaxParticipantNum = 0;
    mMaxParticipantNumSet = 0;
}

void CreateSessionSetting::Trace(uint64_t) const
{
}

void CreateSessionSetting::Copy(const CreateSessionSetting& other)
{
    mMinParticipantNum = other.mMinParticipantNum;
    mMinParticipantNumSet = other.mMinParticipantNumSet;
    mMaxParticipantNum = other.mMaxParticipantNum;
    mMaxParticipantNumSet = other.mMaxParticipantNumSet;
}

}
}
}

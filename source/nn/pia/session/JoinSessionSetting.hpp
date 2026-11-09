#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOIN_SESSION_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOIN_SESSION_SETTING_HPP

#include "nn/pia/session/ISessionInfo.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class JoinSessionSetting
{
public:
    JoinSessionSetting();
    virtual ~JoinSessionSetting();

    virtual const ISessionInfo* GetSessionInfoPtr() const;
    virtual void SetSessionId(unsigned int sessionId);
    virtual unsigned int GetSessionId() const;
    virtual void SetMaxParticipantNum(unsigned short participantNum);
    virtual unsigned int GetMaxParticipantNum() const;
    virtual void SetSessionInfoPtr(const ISessionInfo* sessionInfo);
    virtual void Reset();
    void Trace(unsigned long long value) const;

    const ISessionInfo* m_sessionInfo;
    unsigned int m_sessionId;
    unsigned short m_maxParticipantNum;
};

}
}
}

#endif

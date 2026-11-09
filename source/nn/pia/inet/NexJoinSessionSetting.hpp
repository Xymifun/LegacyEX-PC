#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_JOIN_SESSION_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_JOIN_SESSION_SETTING_HPP

#include "nn/pia.h"
#include "nn/pia/String.hpp"
#include "nn/pia/session/JoinSessionSetting.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexJoinSessionSetting : public session::JoinSessionSetting
{
public:
    NexJoinSessionSetting();

    virtual void Reset();

    void SetBlockListOption(bool useMyBlockList, bool useOthersBlockList);
    void SetSessionUserPassword(const common::String& password);
    void SetSessionSystemPassword(const common::String& password);
    void SetExtraParticipantNum(unsigned short participantNum);
    void SetMatchmakeSessionOption(unsigned int option);

    const common::String& GetSessionUserPassword() const;
    const common::String& GetSessionSystemPassword() const;
    unsigned short GetExtraParticipantNum() const;
    unsigned int GetMatchmakeSessionOption() const;
    bool IsSetMatchmakeSessionOption() const;
    bool IsMyBlockListUsed() const;
    bool IsOthersBlockListUsed() const;

    common::FixedString<32> m_sessionUserPassword;
    common::FixedString<16> m_sessionSystemPassword;
    unsigned short m_extraParticipantNum;
    unsigned int m_matchmakeSessionOption;
    bool m_isSetMatchmakeSessionOption;
    bool m_isMyBlockListUsed;
    bool m_isOthersBlockListUsed;
};

}
}
}

#endif

#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOIN_RANDOM_SESSION_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOIN_RANDOM_SESSION_SETTING_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/session/CreateSessionSetting.hpp"
#include "nn/pia/session/SessionSearchCriteria.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class JoinRandomSessionSetting
{
public:
    virtual ~JoinRandomSessionSetting() {}
    virtual Result SetCreateSessionSetting(const CreateSessionSetting* setting) = 0;
    virtual Result SetSessionSearchCriteria(const SessionSearchCriteria* criteria, unsigned int count) = 0;
    virtual void Reset() = 0;
    virtual void Trace(unsigned long long value) const = 0;
    virtual CreateSessionSetting* GetCreateSessionSetting() = 0;
    virtual const CreateSessionSetting* GetCreateSessionSetting() const = 0;
    virtual SessionSearchCriteria* GetSessionSearchCriteria(unsigned char index) = 0;
    virtual const SessionSearchCriteria* GetSessionSearchCriteria(unsigned char index) const = 0;
    virtual unsigned int GetSessionSearchCriteriaSize() const = 0;
};

}
}
}

#endif

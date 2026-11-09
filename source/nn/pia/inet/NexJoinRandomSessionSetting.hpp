#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXJOINRANDOMSESSIONSETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXJOINRANDOMSESSIONSETTING_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/inet/NexCreateSessionSetting.hpp"
#include "nn/pia/inet/NexSessionSearchCriteria.hpp"
#include "nn/pia/session/JoinRandomSessionSetting.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexJoinRandomSessionSetting : public session::JoinRandomSessionSetting
{
public:
    NexJoinRandomSessionSetting();
    virtual ~NexJoinRandomSessionSetting();
    session::CreateSessionSetting* GetCreateSessionSetting();
    const session::CreateSessionSetting* GetCreateSessionSetting() const;
    session::SessionSearchCriteria* GetSessionSearchCriteria(unsigned char index);
    const session::SessionSearchCriteria* GetSessionSearchCriteria(unsigned char index) const;
    unsigned int GetSessionSearchCriteriaSize() const;
    void Reset();
    Result SetCreateSessionSetting(const session::CreateSessionSetting* setting);
    Result SetSessionSearchCriteria(const session::SessionSearchCriteria* criteria, unsigned int count);
    void Trace(unsigned long long p0) const;
    void SetBlockListOption(bool useMyBlockList, bool useOthersBlockList);
    void SetMatchmakeUniqueKeyword(bool enabled);
    bool IsMyBlockListUsed() const;
    bool IsOthersBlockListUsed() const;
    bool IsMatchmakeUniqueKeywordEnabled() const;

private:
    NexCreateSessionSetting m_createSessionSetting;
    NexSessionSearchCriteria m_sessionSearchCriteria[2];
    unsigned int m_sessionSearchCriteriaSize;
    bool m_myBlockListUsed;
    bool m_othersBlockListUsed;
    bool m_matchmakeUniqueKeyword;
};

}
}
}

#endif

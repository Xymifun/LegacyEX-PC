#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_SESSION_SEARCH_CRITERIA_SESSION_ID_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_SESSION_SEARCH_CRITERIA_SESSION_ID_HPP

#include "nn/pia/session/SessionSearchCriteria.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexSessionSearchCriteriaSessionId : public session::SessionSearchCriteria
{
public:
    NexSessionSearchCriteriaSessionId();
    NexSessionSearchCriteriaSessionId(unsigned int sessionId);
    virtual ~NexSessionSearchCriteriaSessionId();

    unsigned int GetSessionId() const;
    void Reset();
    nn::pia::Result SetResultRange(unsigned int offset, unsigned int size);
    void SetSessionId(unsigned int sessionId);
    void Copy(const NexSessionSearchCriteriaSessionId& other);
    NexSessionSearchCriteriaSessionId& operator=(const NexSessionSearchCriteriaSessionId& other);

private:
    unsigned int m_sessionId;
};

}
}
}

#endif

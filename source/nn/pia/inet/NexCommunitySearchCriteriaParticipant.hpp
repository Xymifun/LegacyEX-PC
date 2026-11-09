#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_COMMUNITY_SEARCH_CRITERIA_PARTICIPANT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_COMMUNITY_SEARCH_CRITERIA_PARTICIPANT_HPP

#include "nn/pia.h"
#include "nn/pia/session/CommunitySearchCriteria.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexCommunitySearchCriteriaParticipant : public session::CommunitySearchCriteria
{
public:
    NexCommunitySearchCriteriaParticipant();
    NexCommunitySearchCriteriaParticipant(unsigned long long participant, unsigned int offset, unsigned int size);
    NexCommunitySearchCriteriaParticipant(const NexCommunitySearchCriteriaParticipant& other);
    ~NexCommunitySearchCriteriaParticipant();
    NexCommunitySearchCriteriaParticipant& operator=(const NexCommunitySearchCriteriaParticipant& other);
    void Copy(const NexCommunitySearchCriteriaParticipant& other);
    unsigned long long GetParticipantPrincipalId() const;
    void SetParticipantPrincipalId(unsigned long long participant);
    Result SetResultRange(unsigned int offset, unsigned int size);
    void Reset();

private:
    unsigned long long m_participantPrincipalId;
};

}
}
}

#endif

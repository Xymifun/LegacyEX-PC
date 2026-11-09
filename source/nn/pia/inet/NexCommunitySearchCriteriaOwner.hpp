#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_COMMUNITY_SEARCH_CRITERIA_OWNER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_COMMUNITY_SEARCH_CRITERIA_OWNER_HPP

#include "nn/pia.h"
#include "nn/pia/session/CommunitySearchCriteria.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexCommunitySearchCriteriaOwner : public session::CommunitySearchCriteria
{
public:
    NexCommunitySearchCriteriaOwner();
    NexCommunitySearchCriteriaOwner(unsigned long long owner, unsigned int offset, unsigned int size);
    NexCommunitySearchCriteriaOwner(const NexCommunitySearchCriteriaOwner& other);
    ~NexCommunitySearchCriteriaOwner();
    NexCommunitySearchCriteriaOwner& operator=(const NexCommunitySearchCriteriaOwner& other);
    void Copy(const NexCommunitySearchCriteriaOwner& other);
    unsigned long long GetOwnerPrincipalId() const;
    void SetOwnerPrincipalId(unsigned long long owner);
    Result SetResultRange(unsigned int offset, unsigned int size);
    void Reset();

private:
    unsigned long long m_ownerPrincipalId;
};

}
}
}

#endif

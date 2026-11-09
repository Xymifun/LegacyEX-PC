#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_COMMUNITY_SEARCH_CRITERIA_OFFICIAL_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_COMMUNITY_SEARCH_CRITERIA_OFFICIAL_HPP

#include "nn/pia.h"
#include "nn/pia/session/CommunitySearchCriteria.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexCommunitySearchCriteriaOfficial : public session::CommunitySearchCriteria
{
public:
    NexCommunitySearchCriteriaOfficial();
    NexCommunitySearchCriteriaOfficial(bool availableOnly, unsigned int offset, unsigned int size);
    NexCommunitySearchCriteriaOfficial(const NexCommunitySearchCriteriaOfficial& other);
    ~NexCommunitySearchCriteriaOfficial();
    NexCommunitySearchCriteriaOfficial& operator=(const NexCommunitySearchCriteriaOfficial& other);
    void Copy(const NexCommunitySearchCriteriaOfficial& other);
    Result SetResultRange(unsigned int offset, unsigned int size);
    bool IsAvailableCommunityOnly() const;
    void SetAvailableCommunityOnly(bool availableOnly);
    void Reset();

private:
    bool m_availableCommunityOnly;
};

}
}
}

#endif

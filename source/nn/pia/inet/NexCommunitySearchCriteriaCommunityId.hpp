#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_COMMUNITY_SEARCH_CRITERIA_COMMUNITY_ID_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_COMMUNITY_SEARCH_CRITERIA_COMMUNITY_ID_HPP

#include "nn/pia.h"
#include "nn/pia/session/CommunitySearchCriteria.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexCommunitySearchCriteriaCommunityId : public session::CommunitySearchCriteria
{
public:
    NexCommunitySearchCriteriaCommunityId();
    NexCommunitySearchCriteriaCommunityId(unsigned int* communityIds, unsigned int count);
    NexCommunitySearchCriteriaCommunityId(const NexCommunitySearchCriteriaCommunityId& other);
    ~NexCommunitySearchCriteriaCommunityId();
    NexCommunitySearchCriteriaCommunityId& operator=(const NexCommunitySearchCriteriaCommunityId& other);
    void Copy(const NexCommunitySearchCriteriaCommunityId& other);
    void SetCommunityIdList(unsigned int* communityIds, unsigned int count);
    Result SetResultRange(unsigned int offset, unsigned int size);
    const unsigned int* GetCommunityIdList() const;
    unsigned int GetCommunityIdListSize() const;
    void Reset();

private:
    unsigned int m_communityIds[100];
    unsigned int m_communityIdCount;
};

}
}
}

#endif

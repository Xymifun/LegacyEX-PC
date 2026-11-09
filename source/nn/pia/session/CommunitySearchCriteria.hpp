#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_COMMUNITY_SEARCH_CRITERIA_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_COMMUNITY_SEARCH_CRITERIA_HPP

#include "nn/pia/Reconstruction.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class CommunitySearchCriteria
{
public:
    enum SearchCriteriaKey
    {
        cKeyTypeDefault = 0,
        cKeyTypeCommunityId = 1,
        cKeyTypeParticipant = 2,
        cKeyTypeOwner = 3,
        cKeyTypeOfficial = 4
    };

    CommunitySearchCriteria();
    CommunitySearchCriteria(SearchCriteriaKey keyType);
    ~CommunitySearchCriteria();
    void Copy(const CommunitySearchCriteria& other);
    Result SetResultRange(unsigned int offset, unsigned int size);
    SearchCriteriaKey GetKeyType() const;
    unsigned int GetResultRangeOffset() const;
    unsigned int GetResultRangeSize() const;
    void Reset();

protected:
    SearchCriteriaKey m_keyType;
    unsigned int m_resultRangeOffset;
    unsigned int m_resultRangeSize;
    unsigned int m_reserved;
};

}
}
}

#endif

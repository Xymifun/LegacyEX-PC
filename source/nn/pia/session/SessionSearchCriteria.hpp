#ifndef NN_PIA_SESSION_SESSIONSEARCHCRITERIA_HPP
#define NN_PIA_SESSION_SESSIONSEARCHCRITERIA_HPP

#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class SessionSearchCriteria
{
public:
    enum SearchCriteriaKey
    {
        cKeyTypeDefault = 0,
        cKeyTypeOwner = 1,
        cKeyTypeParticipant = 2,
        cKeyTypeSessionId = 3
    };

    SessionSearchCriteria();
    virtual ~SessionSearchCriteria();
    virtual nn::pia::Result SetResultRange(unsigned int offset, unsigned int size);
    virtual void Reset();
    unsigned int GetResultRangeOffset() const;
    unsigned int GetResultRangeSize() const;
    SearchCriteriaKey GetKeyType() const;
    void SetKeyType(SearchCriteriaKey keyType);
    void Copy(const SessionSearchCriteria& other);
    virtual void Trace(unsigned long long value) const;

    unsigned int m_resultRangeOffset;
    unsigned int m_resultRangeSize;
    SearchCriteriaKey m_keyType;
};

}
}
}

#endif

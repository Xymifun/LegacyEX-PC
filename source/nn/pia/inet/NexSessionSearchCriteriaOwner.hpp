#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXSESSIONSEARCHCRITERIAOWNER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXSESSIONSEARCHCRITERIAOWNER_HPP

#include <stdint.h>
#include "nn/pia/session/SessionSearchCriteria.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexSessionSearchCriteriaOwner : public nn::pia::session::SessionSearchCriteria
{
public:
    NexSessionSearchCriteriaOwner();
    ~NexSessionSearchCriteriaOwner();
    nn::pia::Result SetResultRange(uint32_t offset, uint32_t size);
    uint64_t GetOwnerPrincipalId() const;
    void Reset();
    void SetOwnerPrincipalId(unsigned long long principalId);
    void Copy(const NexSessionSearchCriteriaOwner& other);
    NexSessionSearchCriteriaOwner(const NexSessionSearchCriteriaOwner& other);
    NexSessionSearchCriteriaOwner& operator=(const NexSessionSearchCriteriaOwner& other);

private:
    uint64_t owner_principal_id_;
};
}
}
}

#endif

#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXSESSIONSEARCHCRITERIAPARTICIPANT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXSESSIONSEARCHCRITERIAPARTICIPANT_HPP

#include <stdint.h>
#include "nn/pia/session/SessionSearchCriteria.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexSessionSearchCriteriaParticipant : public nn::pia::session::SessionSearchCriteria
{
public:
    NexSessionSearchCriteriaParticipant();
    ~NexSessionSearchCriteriaParticipant();
    const unsigned long long* GetParticipantPrincipalId() const;
    uint32_t GetParticipantPrincipalIdSize() const;
    void Reset();
    nn::pia::Result SetResultRange(uint32_t offset, uint32_t size);
    void SetParticipantPrincipalId(const unsigned long long* principalIds, unsigned int count);
    void SetBlockListOption(bool useMyBlockList, bool useOthersBlockList);
    void SetApplicationDataEnabled(bool enabled);
    bool IsNetworkServiceAccountIdUsed() const;
    bool IsMyBlockListUsed() const;
    bool IsOthersBlockListUsed() const;
    void Copy(const NexSessionSearchCriteriaParticipant& other);
    NexSessionSearchCriteriaParticipant(const NexSessionSearchCriteriaParticipant& other);
    NexSessionSearchCriteriaParticipant& operator=(const NexSessionSearchCriteriaParticipant& other);

private:
    unsigned long long principal_ids_[100];
    uint32_t principal_id_size_;
    bool network_service_account_id_used_;
    bool my_block_list_used_;
    bool application_data_enabled_;
    bool others_block_list_used_;
};
}
}
}

#endif

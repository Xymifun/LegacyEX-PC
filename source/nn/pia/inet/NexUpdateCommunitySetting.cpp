#include "nn/pia/inet/NexUpdateCommunitySetting.hpp"
#include <coreinit/memory.h>
#include "nn/nex/MemAllocator.hpp"
#include <cstring>
#include <vector>

namespace nn
{
namespace pia
{
namespace inet
{

NexUpdateCommunitySetting::~NexUpdateCommunitySetting()
{
}

void NexUpdateCommunitySetting::Reset()
{
    communityId = 0;
    minParticipantNum = static_cast<unsigned short>(-1);
    maxParticipantNum = static_cast<unsigned short>(-1);
    OSBlockSet(attributes, 0, sizeof(attributes));
    OSBlockSet(applicationData, 0, sizeof(applicationData));
    applicationDataSize = 0;
    communityUserPassword.Clear();
    participationStartDate.Clear();
    participationEndDate.Clear();
    description.Clear();
}

void NexUpdateCommunitySetting::Trace(unsigned long long) const
{
}
unsigned int NexUpdateCommunitySetting::GetCommunityId() const
{
    return communityId;
}
unsigned short NexUpdateCommunitySetting::GetMinParticipantNum() const
{
    return minParticipantNum;
}
unsigned short NexUpdateCommunitySetting::GetMaxParticipantNum() const
{
    return maxParticipantNum;
}
unsigned int NexUpdateCommunitySetting::GetAttribute(unsigned int index) const
{
    return index < 6 ? attributes[index] : 0;
}
const common::String& NexUpdateCommunitySetting::GetDescription() const
{
    return description;
}
const common::String& NexUpdateCommunitySetting::GetCommunityUserPassword() const
{
    return communityUserPassword;
}
bool NexUpdateCommunitySetting::IsSetCommunityUserPassword() const
{
    return communityUserPasswordSet;
}
const common::DateTime& NexUpdateCommunitySetting::GetParticipaitonStartDate() const
{
    return participationStartDate;
}
const common::DateTime& NexUpdateCommunitySetting::GetParticipaitonEndDate() const
{
    return participationEndDate;
}
void NexUpdateCommunitySetting::CopyApplicationDataToVector(void* vector) const
{
    typedef std::vector<unsigned char, nn::nex::MemAllocator<unsigned char> > ByteVector;
    ByteVector* bytes = static_cast<ByteVector*>(vector);
    bytes->clear();
    bytes->reserve(applicationDataSize);
    for (unsigned int index = 0; index < applicationDataSize; ++index)
        bytes->push_back(applicationData[index]);
}
bool NexUpdateCommunitySetting::IsValid() const
{
    return true;
}

}
}
}

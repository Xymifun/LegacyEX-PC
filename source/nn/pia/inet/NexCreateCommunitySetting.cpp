#include "nn/pia/inet/NexCreateCommunitySetting.hpp"
#include "nn/nex/MemAllocator.hpp"

#include <coreinit/memory.h>
#include <vector>

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{
enum
{
    cFlagAttributeFirst = 3
};
}

NexCreateCommunitySetting::NexCreateCommunitySetting()
{
    Reset();
}

const common::DateTime* NexCreateCommunitySetting::GetParticipaitonStartDate() const
{
    return &m_participationStartDate;
}

const common::DateTime* NexCreateCommunitySetting::GetParticipaitonEndDate() const
{
    return &m_participationEndDate;
}

const common::String* NexCreateCommunitySetting::GetDescription() const
{
    return &m_description;
}

unsigned int NexCreateCommunitySetting::GetAttribute(unsigned int index) const
{
    if (index >= cAttributeNum)
        return 0;
    return m_attributes[index];
}

bool NexCreateCommunitySetting::IsSetAttribute(unsigned int index) const
{
    if (index >= cAttributeNum)
        return false;
    return (m_stateFlags >> (index + cFlagAttributeFirst) & 1) != 0;
}

void NexCreateCommunitySetting::CopyApplicationDataToVector(void* vector) const
{
    typedef std::vector<unsigned char, nn::nex::MemAllocator<unsigned char> > ByteVector;
    ByteVector* bytes = static_cast<ByteVector*>(vector);
    bytes->clear();
    bytes->reserve(m_applicationDataSize);
    for (unsigned int index = 0; index < m_applicationDataSize; ++index)
        bytes->push_back(m_applicationData[index]);
}

const common::String* NexCreateCommunitySetting::GetCommunityUserPassword() const
{
    return &m_communityUserPassword;
}

void NexCreateCommunitySetting::Reset()
{
    session::CreateCommunitySetting::Reset();
    m_applicationDataSize = 0;
    OSBlockSet(m_applicationData, 0, sizeof(m_applicationData));
    OSBlockSet(m_attributes, 0, sizeof(m_attributes));
    m_participationStartDate.Clear();
    m_participationEndDate.Clear();
    m_description.Clear();
    m_communityUserPassword.Clear();
    m_stateFlags = 0;
}

}
}
}

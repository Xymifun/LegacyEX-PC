#include "nn/pia/inet/NexCommunityInfoList.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

NexCommunityInfoList::~NexCommunityInfoList()
{
    for (unsigned int index = 0; index < m_maxSize; ++index)
        transport::DestroyObject(m_entries[index]);
    common::DeleteArray(m_entries);
}

session::ICommunityInfo* const* NexCommunityInfoList::Begin() const
{
    return m_entries;
}
session::ICommunityInfo** NexCommunityInfoList::Begin()
{
    return m_entries;
}
session::ICommunityInfo* const* NexCommunityInfoList::End() const
{
    return m_entries + m_size;
}
session::ICommunityInfo** NexCommunityInfoList::End()
{
    return m_entries + m_size;
}
unsigned int NexCommunityInfoList::GetSize() const
{
    return m_size;
}
unsigned int NexCommunityInfoList::GetMaxSize() const
{
    return m_maxSize;
}

void NexCommunityInfoList::Clear()
{
    m_size = 0;
    for (unsigned int index = 0; index < m_maxSize; ++index)
        m_entries[index]->Clear();
}

}
}
}

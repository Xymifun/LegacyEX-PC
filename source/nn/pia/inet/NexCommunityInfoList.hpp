#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_COMMUNITY_INFO_LIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_COMMUNITY_INFO_LIST_HPP

#include "nn/pia/NetworkFactory.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/inet/NexCommunityInfo.hpp"
#include "nn/pia/session/ICommunityInfoList.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexCommunityInfoList : public session::ICommunityInfoList
{
public:
    explicit NexCommunityInfoList(unsigned int maxSize)
        : m_maxSize(maxSize),
          m_entries(common::NewArray<session::ICommunityInfo*>(maxSize))
    {
        for (unsigned int index = 0; index < m_maxSize; ++index)
        {
            NexCommunityInfo* communityInfo = transport::CreateObject<NexCommunityInfo>();
            communityInfo->Clear();
            m_entries[index] = communityInfo;
        }
        Clear();
    }

    virtual ~NexCommunityInfoList();
    virtual session::ICommunityInfo* const* Begin() const;
    virtual session::ICommunityInfo** Begin();
    virtual session::ICommunityInfo* const* End() const;
    virtual session::ICommunityInfo** End();
    virtual unsigned int GetSize() const;
    virtual unsigned int GetMaxSize() const;
    virtual void Clear();

    NexCommunityInfo* AddCommunityInfo()
    {
        NexCommunityInfo* communityInfo = NULL;
        if (m_maxSize != 0 && m_size < m_maxSize)
        {
            communityInfo = static_cast<NexCommunityInfo*>(m_entries[m_size]);
            communityInfo->Clear();
            ++m_size;
        }
        return communityInfo;
    }

private:
    unsigned int m_size;
    unsigned int m_maxSize;
    session::ICommunityInfo** m_entries;
};

}
}
}

#endif

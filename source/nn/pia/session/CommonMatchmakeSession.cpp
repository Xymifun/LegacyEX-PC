#include "nn/pia/session/CommonMatchmakeSession.hpp"

namespace nn
{
namespace pia
{
namespace session
{

CommonMatchmakeSession::CommonMatchmakeSession() : m_matchmakeOwnerUniqueId(0), m_matchmakeOwner(false)
{
}

void CommonMatchmakeSession::Cleanup()
{
}

unsigned long long CommonMatchmakeSession::GetMatchmakeOwnerUniqueId() const
{
    return m_matchmakeOwnerUniqueId;
}

void CommonMatchmakeSession::SetMatchmakeOwnerUniqueId(unsigned long long id)
{
    m_matchmakeOwnerUniqueId = id;
}

bool CommonMatchmakeSession::IsMatchmakeOwner() const
{
    return m_matchmakeOwner;
}

void CommonMatchmakeSession::SetMatchmakeOwner(bool owner)
{
    m_matchmakeOwner = owner;
}

nn::pia::Result CommonMatchmakeSession::UpdateSelectionPriority(unsigned int, unsigned char)
{
    return nn::pia::Result(0xC0B);
}

unsigned int CommonMatchmakeSession::IsUpdatableSelectionPriority(unsigned char)
{
    return 0;
}

unsigned long long CommonMatchmakeSession::ConvertGatheringIdToPersistentGatheringCode(unsigned int)
{
    return 0;
}

unsigned int CommonMatchmakeSession::ConvertPersistentGatheringCodeToGatheringId(unsigned long long)
{
    return 0;
}

ICommunityInfoList* CommonMatchmakeSession::GetBrowsedCommunityResult()
{
    return 0;
}

void CommonMatchmakeSession::UpdateStationInfoList()
{
}

}
}
}

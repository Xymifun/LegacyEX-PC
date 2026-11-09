#ifndef NN_PIA_SESSION_COMMONMATCHMAKESESSION_HPP
#define NN_PIA_SESSION_COMMONMATCHMAKESESSION_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class CommonMatchmakeSession : public IMatchmakeSession
{
public:
    CommonMatchmakeSession();
    virtual ~CommonMatchmakeSession() {}
    virtual void Cleanup();
    virtual unsigned long long ConvertGatheringIdToPersistentGatheringCode(unsigned int gatheringId);
    virtual unsigned int ConvertPersistentGatheringCodeToGatheringId(unsigned long long code);
    virtual ICommunityInfoList* GetBrowsedCommunityResult();
    virtual bool IsMatchmakeOwner() const;
    virtual void SetMatchmakeOwner(bool owner);
    virtual unsigned long long GetMatchmakeOwnerUniqueId() const;
    virtual void SetMatchmakeOwnerUniqueId(unsigned long long id);
    virtual nn::pia::Result UpdateSelectionPriority(unsigned int sessionId, unsigned char selectionPriority);
    virtual unsigned int IsUpdatableSelectionPriority(unsigned char selectionPriority);
    virtual void UpdateStationInfoList();
    void Trace(unsigned long long value) const;

protected:
    unsigned long long m_matchmakeOwnerUniqueId;
    bool m_matchmakeOwner;
};

}
}
}

#endif

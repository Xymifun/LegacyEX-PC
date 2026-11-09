#include "nn/nex/ProviderSupport.hpp"
#include "nn/nex/Reconstruction.hpp"

#include <nn/middleware.h>

#include "nn/nex/AnyGatheringAdapter.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/DynamicGathering.hpp"
#include "nn/nex/GameSession.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/Scheduler.hpp"

#include "nn/nex/Message.hpp"
#include "nn/nex/ProtocolCallContext.hpp"

namespace nn
{
namespace nex
{

MatchMakingClient::MatchMakingClient()
    : ServiceClient(0x0c),
      m_matchMakingProtocolClient(0),
      m_matchMakingProtocolExtClient(0)
{
    SetModuleId("[SDK+Nintendo:NEX_MM_3_10_0]");
    m_matchMakingProtocolClient.SetProtocolID(0x15);
    m_matchMakingProtocolExtClient.SetProtocolID(0x32);
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (AnyGatheringAdapter::RegisterUse() == 1)
    {
        Gathering* pGathering = new Gathering;
        if (!RegisterPrototype(pGathering))
            delete pGathering;
        DynamicGathering* pDynamicGathering = new DynamicGathering;
        if (!RegisterPrototype(pDynamicGathering))
            delete pDynamicGathering;
        GameSession* pGameSession = new GameSession;
        if (!RegisterPrototype(pGameSession))
            delete pGameSession;
    }
    MatchMakingProtocolClient* pProtocolClient = new MatchMakingProtocolClient();
    GetProtocolRegistry()->RegisterProtocol(0x15, pProtocolClient);
    MatchMakingProtocolExtClient* pExtProtocolClient = new MatchMakingProtocolExtClient();
    GetProtocolRegistry()->RegisterProtocol(0x32, pExtProtocolClient);
}

MatchMakingClient::~MatchMakingClient()
{
    AnyGatheringAdapter::UnregisterUse();
}

void MatchMakingClient::UpdateProtocolsDefaultCredentials(Credentials* pCredentials)
{
    m_matchMakingProtocolClient.SetDefaultCredentials(pCredentials);
    m_matchMakingProtocolExtClient.SetDefaultCredentials(pCredentials);
}

Gathering* MatchMakingClient::FindPrototype(const String& rType) const
{
    return AnyGatheringAdapter::s_instance.FindPrototype(rType);
}

bool MatchMakingClient::RegisterGathering(ProtocolCallContext* pCallContext,
                                          const AnyObjectHolder<Gathering, String>& rGathering)
{
    if (FindPrototype(rGathering->GetGatheringType()) == 0)
    {
        if (!rGathering->IsA(String(L"DynamicGathering")))
        {
            return false;
        }
    }
    return m_matchMakingProtocolClient.CallRegisterGathering(pCallContext, &rGathering->m_uiValue1, rGathering);
}

bool MatchMakingClient::UnregisterGathering(ProtocolCallContext* pCallContext, unsigned int uiGatheringID)
{
    return m_matchMakingProtocolClient.CallUnregisterGathering(pCallContext, NULL, uiGatheringID);
}

bool MatchMakingClient::FindByID(ProtocolCallContext* pCallContext, unsigned int uiGatheringID,
                                 AnyObjectHolder<Gathering, String>* pGathering)
{
    return m_matchMakingProtocolClient.CallFindBySingleID(pCallContext, uiGatheringID, NULL, pGathering);
}

bool MatchMakingClient::MigrateGatheringOwnership(ProtocolCallContext* pCallContext, unsigned int uiGatheringID,
                                                  const qList<unsigned int>& rCandidateIDs, bool bParticipantsOnly)
{
    if (uiGatheringID == 0)
    {
        return false;
    }
    return m_matchMakingProtocolClient.CallMigrateGatheringOwnership(pCallContext, uiGatheringID, rCandidateIDs,
                                                                     bParticipantsOnly);
}

bool MatchMakingClient::MigrateGatheringOwnership(ProtocolCallContext* pCallContext, unsigned int uiGatheringID,
                                                  const qList<unsigned int>& rCandidateIDs)
{
    return MigrateGatheringOwnership(pCallContext, uiGatheringID, rCandidateIDs, true);
}

bool MatchMakingClient::GetSessionURLs(ProtocolCallContext* pCallContext, unsigned int uiGatheringID,
                                       qList<StationURL>* pURLs)
{
    return m_matchMakingProtocolClient.CallGetSessionURLs(pCallContext, uiGatheringID, pURLs);
}

bool MatchMakingClient::EndParticipation(ProtocolCallContext* pCallContext, unsigned int uiGatheringID, bool* pSuccess,
                                         const String& strMessage)
{
    if (uiGatheringID == 0)
    {
        return false;
    }
    return m_matchMakingProtocolExtClient.CallEndParticipation(pCallContext, pSuccess, uiGatheringID, strMessage);
}

bool MatchMakingClient::EndParticipation(ProtocolCallContext* pCallContext, unsigned int uiGatheringID,
                                         const String& strMessage)
{
    if (uiGatheringID == 0)
    {
        return false;
    }
    return m_matchMakingProtocolExtClient.CallEndParticipation(pCallContext, NULL, uiGatheringID, strMessage);
}

bool MatchMakingClient::UpdateSessionHost(ProtocolCallContext* pCallContext, unsigned int uiGatheringID,
                                          bool bIsMigrateOwner)
{
    return m_matchMakingProtocolClient.CallUpdateSessionHost(pCallContext, uiGatheringID, bIsMigrateOwner);
}

bool MatchMakingClient::RegisterPrototype(Gathering* pPrototype)
{
    return AnyGatheringAdapter::s_instance.RegisterPrototype(pPrototype);
}

}
}

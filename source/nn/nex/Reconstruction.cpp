#include "nn/nex/Reconstruction.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/ByteOrder.hpp"
#include "nn/nex/HMACChecksum.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/MD5Checksum.hpp"
#include "nn/nex/MatchmakeSession.hpp"
#include "nn/nex/PersistentGathering.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int APPEND_AT_END = 0xFFFFFFFFu;
const unsigned int PERSISTENT_GATHERING_CODE_CHECK_BITS = 14;
const unsigned int PERSISTENT_GATHERING_CODE_CHECK_SHIFT = 32 - PERSISTENT_GATHERING_CODE_CHECK_BITS;
const unsigned int GATHERING_ID_BITS = 32;
const unsigned int GATHERING_ID_MASK = 0xFFFFFFFFu;
const unsigned long long PERSISTENT_GATHERING_CODE_MAX =
    (static_cast<unsigned long long>(1u << PERSISTENT_GATHERING_CODE_CHECK_BITS) << GATHERING_ID_BITS) - 1;

enum
{
    RESULT_INVALID_ARGUMENT = static_cast<int>(0x8001000Au),
    RESULT_INVALID_SEQUENCE = static_cast<int>(0x80010011u)
};

const unsigned int ATTRIBUTE_INDEX_OFFSET = 1;
const unsigned int MATCHMAKE_SESSION_ATTRIBUTE_NUM = 6;
const unsigned char PROGRESS_SCORE_MIN = 0;
const unsigned char PROGRESS_SCORE_MAX = 100;
const long long PROGRESS_SCORE_UPDATE_INTERVAL = 30000;

KeyedChecksumAlgorithm* s_pPersistentGatheringCodeChecksum = NULL;
bool s_bPersistentGatheringCodeKeySet = false;

}

MatchmakeExtensionClient::MatchmakeExtensionClient()
    : MatchMakingClient(),
      m_protocolRegistered(false),
      m_matchmakeExtensionProtocolClient(0),
      m_uiLastProgressScoreTime(0)
{
    m_matchmakeExtensionProtocolClient.SetProtocolID(0x6d);
}

MatchmakeExtensionClient::~MatchmakeExtensionClient()
{
    if (m_protocolRegistered)
    {
        UnregisterProtocol(&m_matchmakeExtensionProtocolClient);
    }
}

bool MatchmakeExtensionClient::Bind(Credentials* pCredentials)
{
    const bool bBound = ServiceClient::Bind(pCredentials);
    if (bBound && !m_protocolRegistered)
    {
        RegisterProtocol(&m_matchmakeExtensionProtocolClient);
        MatchmakeSession* pMatchmakeSession = new MatchmakeSession();
        if (!RegisterPrototype(pMatchmakeSession))
        {
            delete pMatchmakeSession;
        }
        PersistentGathering* pPersistentGathering = new PersistentGathering();
        if (!RegisterPrototype(pPersistentGathering))
        {
            delete pPersistentGathering;
        }
        m_protocolRegistered = true;
    }
    return bBound;
}

bool MatchmakeExtensionClient::Unbind()
{
    const bool bUnbound = ServiceClient::Unbind();
    if (bUnbound && m_protocolRegistered)
    {
        UnregisterProtocol(&m_matchmakeExtensionProtocolClient);
    }
    m_protocolRegistered = false;
    return bUnbound;
}

bool MatchmakeExtensionClient::AutoMatchmake(ProtocolCallContext* pCallContext, const AutoMatchmakeParam& rParam,
                                             MatchmakeSession* pJoinedSession)
{
    return m_matchmakeExtensionProtocolClient.CallAutoMatchmakeWithParam_Postpone(pCallContext, rParam, pJoinedSession);
}

bool MatchmakeExtensionClient::BrowseMatchmakeSession(ProtocolCallContext* pCallContext,
                                                      const MatchmakeSessionSearchCriteria& rSearchCriteria,
                                                      const ResultRange& rRange,
                                                      qList<AnyObjectHolder<Gathering, String> >* pGatherings)
{
    return m_matchmakeExtensionProtocolClient.CallBrowseMatchmakeSession(pCallContext, rSearchCriteria, rRange,
                                                                         pGatherings);
}

bool MatchmakeExtensionClient::CreateMatchmakeSession(ProtocolCallContext* pCallContext,
                                                      const CreateMatchmakeSessionParam& rParam,
                                                      MatchmakeSession* pCreatedSession)
{
    return m_matchmakeExtensionProtocolClient.CallCreateMatchmakeSessionWithParam(pCallContext, rParam,
                                                                                  pCreatedSession);
}

bool MatchmakeExtensionClient::JoinMatchmakeSession(ProtocolCallContext* pCallContext,
                                                    const JoinMatchmakeSessionParam& rParam,
                                                    MatchmakeSession* pJoinedSession)
{
    return m_matchmakeExtensionProtocolClient.CallJoinMatchmakeSessionWithParam(pCallContext, rParam, pJoinedSession);
}

bool MatchmakeExtensionClient::CloseParticipation(ProtocolCallContext* pCallContext, unsigned int uiGatheringID)
{
    return m_matchmakeExtensionProtocolClient.CallCloseParticipation(pCallContext, uiGatheringID);
}

bool MatchmakeExtensionClient::OpenParticipation(ProtocolCallContext* pCallContext, unsigned int uiGatheringID)
{
    return m_matchmakeExtensionProtocolClient.CallOpenParticipation(pCallContext, uiGatheringID);
}

bool MatchmakeExtensionClient::ModifyMatchmakeSessionAttribute(ProtocolCallContext* pCallContext,
                                                               unsigned int uiGatheringID,
                                                               unsigned int uiAttributeIndex, unsigned int uiNewValue)
{
    return m_matchmakeExtensionProtocolClient.CallModifyCurrentGameAttribute(
        pCallContext, uiGatheringID, uiAttributeIndex + ATTRIBUTE_INDEX_OFFSET, uiNewValue);
}

bool MatchmakeExtensionClient::UpdateMatchmakeSessionAttribute(ProtocolCallContext* pCallContext,
                                                               unsigned int uiGatheringID, unsigned int* pAttributes)
{
    qVector<unsigned int> attributes;
    for (unsigned int i = 0; i < MATCHMAKE_SESSION_ATTRIBUTE_NUM; ++i)
    {
        attributes.push_back(pAttributes[i]);
    }
    return m_matchmakeExtensionProtocolClient.CallUpdateMatchmakeSessionAttribute(pCallContext, uiGatheringID,
                                                                                  attributes);
}

bool MatchmakeExtensionClient::UpdateProgressScore(ProtocolCallContext* pCallContext, unsigned int uiGatheringID,
                                                   unsigned char uiProgressScore)
{
    const Time currentTime(Time::GetTime());
    if (uiProgressScore > PROGRESS_SCORE_MAX)
    {
        pCallContext->InitiateSpecialCall();
        pCallContext->SignalFailure(qResult(RESULT_INVALID_ARGUMENT));
        return false;
    }
    if (uiProgressScore != PROGRESS_SCORE_MIN && uiProgressScore != PROGRESS_SCORE_MAX
        && m_uiLastProgressScoreTime != 0)
    {
        const long long iElapsedTime = currentTime - m_uiLastProgressScoreTime;
        if (iElapsedTime < PROGRESS_SCORE_UPDATE_INTERVAL)
        {
            pCallContext->InitiateSpecialCall();
            pCallContext->SignalFailure(qResult(RESULT_INVALID_SEQUENCE));
            return false;
        }
    }
    m_uiLastProgressScoreTime = currentTime;
    return m_matchmakeExtensionProtocolClient.CallUpdateProgressScore(pCallContext, uiGatheringID, uiProgressScore);
}

bool MatchmakeExtensionClient::UpdateProgressScore(unsigned int uiGatheringID, unsigned char uiProgressScore)
{
    ProtocolCallContext* pCallContext = new ProtocolCallContext();
    pCallContext->SetFlag(CALL_CONTEXT_FLAG_SELF_RELEASE);
    return UpdateProgressScore(pCallContext, uiGatheringID, uiProgressScore);
}

bool MatchmakeExtensionClient::UpdateApplicationBuffer(ProtocolCallContext* pCallContext, unsigned int uiGatheringID,
                                                       qVector<unsigned char> applicationBuffer)
{
    return m_matchmakeExtensionProtocolClient.CallUpdateApplicationBuffer(pCallContext, uiGatheringID,
                                                                          applicationBuffer);
}

bool MatchmakeExtensionClient::UpdateMatchmakeSession(ProtocolCallContext* pCallContext,
                                                      const UpdateMatchmakeSessionParam& rParam)
{
    return m_matchmakeExtensionProtocolClient.CallUpdateMatchmakeSessionPart(pCallContext, rParam);
}

bool MatchmakeExtensionClient::UpdateNotificationData(ProtocolCallContext* pCallContext, unsigned int uiType,
                                                      unsigned int uiParam1, unsigned int uiParam2, String strText)
{
    return m_matchmakeExtensionProtocolClient.CallUpdateNotificationData(pCallContext, uiType, uiParam1, uiParam2,
                                                                         strText);
}

bool MatchmakeExtensionClient::GetFriendNotificationData(ProtocolCallContext* pCallContext,
                                                         qList<NotificationEvent>* pEvents,
                                                         const qList<unsigned int>& rTypes)
{
    return m_matchmakeExtensionProtocolClient.CallGetlstFriendNotificationData(pCallContext, pEvents, rTypes);
}

bool MatchmakeExtensionClient::GetPlayingSession(ProtocolCallContext* pCallContext,
                                                 const qList<unsigned int>& rPrincipalIDs,
                                                 qList<PlayingSession>* pPlayingSessions)
{
    return m_matchmakeExtensionProtocolClient.CallGetPlayingSession(pCallContext, rPrincipalIDs, pPlayingSessions);
}

bool MatchmakeExtensionClient::RequestMatchmaking(ProtocolCallContext* pCallContext, const AutoMatchmakeParam& rParam,
                                                  unsigned long long* pRequestID)
{
    return m_matchmakeExtensionProtocolClient.CallRequestMatchmaking(pCallContext, rParam, pRequestID);
}

bool MatchMakingClient::FindByOwner(ProtocolCallContext* pCallContext, unsigned int uiOwnerID,
                                    const ResultRange& rRange, qList<AnyObjectHolder<Gathering, String> >* pGatherings)
{
    return m_matchMakingProtocolClient.CallFindByOwner(pCallContext, uiOwnerID, rRange, pGatherings);
}

bool MatchmakeExtensionClient::CreatePersistentGathering(ProtocolCallContext* pCallContext,
                                                         const PersistentGathering& rGathering,
                                                         unsigned int* pGatheringID, const String& strMessage)
{
    return m_matchmakeExtensionProtocolClient.CallCreateCommunity(pCallContext, rGathering, pGatheringID, strMessage);
}

bool MatchmakeExtensionClient::UpdatePersistentGathering(ProtocolCallContext* pCallContext,
                                                         const PersistentGathering& rGathering)
{
    return m_matchmakeExtensionProtocolClient.CallUpdateCommunity(pCallContext, rGathering);
}

bool MatchmakeExtensionClient::JoinPersistentGathering(ProtocolCallContext* pCallContext, unsigned int uiGatheringID,
                                                       const String& strMessage, const String& strPassword)
{
    return m_matchmakeExtensionProtocolClient.CallJoinCommunity(pCallContext, uiGatheringID, strMessage, strPassword);
}

bool MatchmakeExtensionClient::FindPersistentGatheringByGatheringId(ProtocolCallContext* pCallContext,
                                                                    const qList<unsigned int>& rGatheringIDs,
                                                                    qList<PersistentGathering>* pGatherings)
{
    return m_matchmakeExtensionProtocolClient.CallFindCommunityByGatheringId(pCallContext, rGatheringIDs, pGatherings);
}

bool MatchmakeExtensionClient::FindOfficialPersistentGathering(ProtocolCallContext* pCallContext, bool bAvailableOnly,
                                                               const ResultRange& rRange,
                                                               qList<PersistentGathering>* pGatherings)
{
    return m_matchmakeExtensionProtocolClient.CallFindOfficialCommunity(pCallContext, pGatherings, bAvailableOnly,
                                                                        rRange);
}

bool MatchmakeExtensionClient::FindPersistentGatheringByParticipant(ProtocolCallContext* pCallContext,
                                                                    unsigned int uiPrincipalID,
                                                                    const ResultRange& rRange,
                                                                    qList<PersistentGathering>* pGatherings)
{
    return m_matchmakeExtensionProtocolClient.CallFindCommunityByParticipant(pCallContext, pGatherings, uiPrincipalID,
                                                                             rRange);
}

bool MatchmakeExtensionClient::FindMatchmakeSessionByGatheringIdDetail(ProtocolCallContext* pCallContext,
                                                                       unsigned int uiGatheringID,
                                                                       MatchmakeSession* pMatchmakeSession)
{
    return m_matchmakeExtensionProtocolClient.CallFindMatchmakeSessionByGatheringIdDetail(pCallContext, uiGatheringID,
                                                                                          pMatchmakeSession);
}

bool MatchmakeExtensionClient::GenerateMatchmakeSessionSystemPassword(ProtocolCallContext* pCallContext,
                                                                      unsigned int uiGatheringID, String* pPassword)
{
    return m_matchmakeExtensionProtocolClient.CallGenerateMatchmakeSessionSystemPassword(pCallContext, uiGatheringID,
                                                                                         pPassword);
}

bool MatchmakeExtensionClient::ClearMatchmakeSessionSystemPassword(ProtocolCallContext* pCallContext,
                                                                   unsigned int uiGatheringID)
{
    return m_matchmakeExtensionProtocolClient.CallClearMatchmakeSessionSystemPassword(pCallContext, uiGatheringID);
}

unsigned long long MatchmakeExtensionClient::GatheringIdToPersistentGatheringCode(unsigned int uiGatheringID)
{
    if (uiGatheringID == 0 || !s_bPersistentGatheringCodeKeySet)
    {
        return 0;
    }
    unsigned int uiNetworkGatheringID = HostOrderToNexNetworkOrder(uiGatheringID);
    unsigned int uiChecksum = s_pPersistentGatheringCodeChecksum->ComputeChecksumForTransport(
        reinterpret_cast<const unsigned char*>(&uiNetworkGatheringID), sizeof(uiNetworkGatheringID));
    uiChecksum >>= PERSISTENT_GATHERING_CODE_CHECK_SHIFT;
    unsigned long long uiCode = uiChecksum;
    return (uiCode << GATHERING_ID_BITS) | uiGatheringID;
}

bool MatchmakeExtensionClient::IsValidPersistentGatheringCode(unsigned long long uiPersistentGatheringCode)
{
    return uiPersistentGatheringCode != 0 && uiPersistentGatheringCode <= PERSISTENT_GATHERING_CODE_MAX
           && GatheringIdToPersistentGatheringCode(
                  static_cast<unsigned int>(uiPersistentGatheringCode & GATHERING_ID_MASK))
                  == uiPersistentGatheringCode;
}

unsigned int MatchmakeExtensionClient::PersistentGatheringCodeToGatheringId(
    unsigned long long uiPersistentGatheringCode)
{
    unsigned int uiGatheringID = 0;
    if (IsValidPersistentGatheringCode(uiPersistentGatheringCode))
    {
        uiGatheringID = static_cast<unsigned int>(uiPersistentGatheringCode & GATHERING_ID_MASK);
    }
    return uiGatheringID;
}

void MatchmakeExtensionClient::SetPersistentGatheringCodeKey(const String& rKey)
{
    if (rKey.GetLength() == 0)
    {
        return;
    }
    if (s_pPersistentGatheringCodeChecksum == NULL)
    {
        s_pPersistentGatheringCodeChecksum = new HMACChecksum();
    }
    MD5Checksum md5Checksum;
    Buffer keyBuffer;
    Buffer digest(md5Checksum.GetChecksumLength(), 0);
    char* pKey;
    rKey.CreateCopy(&pKey);
    keyBuffer.AppendData(pKey, rKey.GetLength(), APPEND_AT_END);
    String::ReleaseCopy(pKey);
    md5Checksum.ComputeChecksum(keyBuffer, &digest);
    s_pPersistentGatheringCodeChecksum->SetKey(Key(digest.GetData(), digest.GetLength()));
    s_bPersistentGatheringCodeKeySet = true;
}

void MatchmakeExtensionClient::ClearPersistentGatheringCodeKey()
{
    if (s_pPersistentGatheringCodeChecksum != NULL)
    {
        delete s_pPersistentGatheringCodeChecksum;
        s_pPersistentGatheringCodeChecksum = NULL;
        s_bPersistentGatheringCodeKeySet = false;
    }
}

}
}

#include "nn/pia/lan/LanMatchmakeUpdateJob.hpp"

#include <coreinit/memory.h>

#include "nn/pia/common/Crypto.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/lan/LanMatchMeshLayerController.hpp"
#include "nn/pia/lan/LanMatchmakeSession.hpp"
#include "nn/pia/lan/LanProtocol.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/SessionInfoList.hpp"
#include "nn/pia/session/Session.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

namespace
{
enum
{
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cResultSocketNoBufferSpace = 0x4c0d,
    cMessageTypeRequest = 0,
    cMessageTypeReply = 1,
    cMessageHeaderSize = 5,
    cMessageTrailerVersion = 1,
    cTrailerFlagCrypto = 1,
    cTrailerSequenceOffset = 2,
    cTrailerKeyParamOffset = 10,
    cTrailerSignatureOffset = 0x1a,
    cTrailerTokenOffset = 0x2a,
    cTrailerTokenSize = 0x100,
    cTrailerHashSize = 0x10,
    cRequestTrailerSize = 0x12a,
    cReplyTrailerSize = 0x3a,
    cTrailerMinimumSize = 0x2a,
    cSessionParamSize = 0x20,
    cKeyParamSize = 0x10,
    cKeySize = 0x10,
    cIvSize = 0xc,
    cBrowseRequestCountMax = 2,
    cBrowseTimeoutMsec = 2000,
    cReplySendCount = 3,
    cWaitMsec = 10
};

const unsigned long long cTraceMessage = 0x0200000000000000ULL;
const unsigned long long cTraceSessionInfo = 0x0100000000000000ULL;

inline LanMatchmakeSession* GetCurrentMatchmakeSession()
{
    return static_cast<LanMatchmakeSession*>(
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]);
}
}

LanMatchmakeUpdateJob::LanMatchmakeUpdateJob(LanProtocol* protocol)
    : common::StepSequenceJob(),
      m_protocol(protocol),
      m_socket(),
      m_searchCriteria(),
      m_browseContext(),
      m_browseDeadline()
{
    OSBlockSet(m_browseKeyParam, 0, sizeof(m_browseKeyParam));
}

LanMatchmakeUpdateJob::~LanMatchmakeUpdateJob()
{
}

void LanMatchmakeUpdateJob::Trace(unsigned long long value) const
{
}

Result LanMatchmakeUpdateJob::Startup(unsigned short* port)
{
    Result result = m_socket.Open();
    if (result != 0)
        return ResultInvalidState();

    result = m_socket.Bind(common::InetAddress(0, cBrowsePort), true);
    if (result == 0)
    {
        if (port != 0)
            *port = cBrowsePort;
        m_browseContext.Reset();
        m_randomSeedPending = true;
        m_browseSequence = 0;
        m_browseRequested = false;
        Reset(true);
        const Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LanMatchmakeUpdateJob::Update),
                        "LanMatchmakeUpdateJob::Update");
        m_currentStep = step;
        return 0;
    }
    return cResultInvalidState;
}

void LanMatchmakeUpdateJob::Cleanup()
{
    if (m_browseContext.m_state == common::CallContext::cStateInProgress)
        m_browseContext.SignalCancel();
    if (m_socket.m_openCount > 0)
        m_socket.Close();
}

void LanMatchmakeUpdateJob::NotifyDisconnectToMeshLayerController()
{
    static_cast<LanMatchMeshLayerController*>(session::g_piaSession->m_meshLayerController)
        ->SetConnectionStatusDisconnected();
}

Result LanMatchmakeUpdateJob::SendBrowseReplyMessage(const common::InetAddress& address, unsigned char* keyParam,
                                                     unsigned int keyParamSize, unsigned char* token,
                                                     unsigned int tokenSize, bool isAuthenticated,
                                                     bool omitSessionParam)
{
    LanMatchmakeSession* matchmake = GetCurrentMatchmakeSession();
    unsigned char payload[cMessageBufferSize];
    unsigned int payloadSize;
    Result result = matchmake->GetSessionInfoPtr()->Serialize(payload, &payloadSize,
                                                              sizeof(payload) - cMessageHeaderSize - cReplyTrailerSize);
    if (result == 0)
    {
        unsigned char* message = m_sendBuffer;
        common::serializeU8(message, cMessageTypeReply);
        common::serializeU32(message + 1, payloadSize);
        OSBlockMove(message + cMessageHeaderSize, payload, payloadSize, FALSE);
        unsigned int sendSize;
        if (omitSessionParam)
        {
            sendSize = payloadSize + cMessageHeaderSize - cSessionParamSize;
        }
        else
        {
            unsigned char* trailer = message + cMessageHeaderSize + payloadSize;
            ++m_browseSequence;
            common::serializeU8(trailer, cMessageTrailerVersion);
            common::serializeU8(trailer + 1, m_protocol->IsCryptoEnabled());
            common::serializeU64(trailer + cTrailerSequenceOffset, m_browseSequence);
            const unsigned int sessionInfoSize = payloadSize - cSessionParamSize;
            if (m_protocol->IsCryptoEnabled() && m_protocol->IsCryptoSettingRegistered() && isAuthenticated)
            {
                unsigned char peerSessionParam[cSessionParamSize];
                if (keyParamSize > cKeyParamSize)
                    keyParamSize = cKeyParamSize;
                common::crypto::GenerateRandomParameter(peerSessionParam, cKeyParamSize);
                OSBlockMove(peerSessionParam + cKeyParamSize, keyParam, keyParamSize, FALSE);
                if (!matchmake->IsSessionParamRegistered())
                {
                    matchmake->SetSessionParam(peerSessionParam, cSessionParamSize);
                    m_protocol->RequestRegisterSessionKey();
                    OSBlockMove(message + cMessageHeaderSize + sessionInfoSize, peerSessionParam, cSessionParamSize,
                                FALSE);
                }
                OSBlockMove(trailer + cTrailerKeyParamOffset, peerSessionParam, cKeyParamSize, FALSE);
                unsigned char iv[cIvSize];
                common::serializeU32(iv, m_protocol->m_broadcastAddress);
                common::serializeU64(iv + 4, m_browseSequence);
                unsigned char key[cKeySize];
                result = m_protocol->CalculateCryptionKeyForBrowseReply(peerSessionParam, cSessionParamSize, key,
                                                                        cKeySize);
                unsigned char hash[cTrailerHashSize];
                result = m_protocol->CalculateTokenHash(token, tokenSize, hash, cTrailerHashSize);
                common::Crypto::Setting setting;
                setting.m_mode = common::Crypto::cModeAesGcm;
                setting.m_iv = iv;
                setting.m_ivLength = cIvSize;
                setting.m_key = key;
                setting.m_keyLength = cKeySize;
                common::Crypto::Encrypt(trailer + cTrailerTokenOffset, hash, cTrailerHashSize,
                                        trailer + cTrailerSignatureOffset, cTrailerHashSize, setting);
            }
            else
            {
                pead::Random random;
                for (unsigned int index = 0; index < cSessionParamSize; ++index)
                    message[cMessageHeaderSize + sessionInfoSize + index] = static_cast<unsigned char>(random.getU32());
                for (unsigned int index = 0; index < cKeyParamSize; ++index)
                    trailer[cTrailerKeyParamOffset + index] = static_cast<unsigned char>(random.getU32());
                for (unsigned int index = 0; index < cTrailerHashSize; ++index)
                    trailer[cTrailerSignatureOffset + index] = static_cast<unsigned char>(random.getU32());
                for (unsigned int index = 0; index < cTrailerHashSize; ++index)
                    trailer[cTrailerTokenOffset + index] = static_cast<unsigned char>(random.getU32());
            }
            sendSize = payloadSize + cMessageHeaderSize + cReplyTrailerSize;
        }
        common::InetAddress destination(address);
        for (unsigned int attempt = 0; attempt < cReplySendCount; ++attempt)
        {
            int sentSize;
            result = m_socket.SendTo(message, sendSize, destination, &sentSize);
            if (result != 0 && result != cResultSocketNoBufferSpace)
                NotifyDisconnectToMeshLayerController();
        }
        destination.Trace(cTraceMessage);
        matchmake->GetSessionInfoPtr()->Trace(cTraceMessage);
    }
    return result;
}

Result LanMatchmakeUpdateJob::SendBrowseMessage(unsigned char index)
{
    if (index >= cBrowseRequestCountMax)
        return cResultInvalidArgument;
    if (m_protocol == 0)
        return cResultInvalidState;
    if (!m_protocol->IsCryptoSettingRegistered())
        return ResultInvalidState();
    if (!m_protocol->IsCryptoEnabled())
        return cResultInvalidState;

    LanMatchmakeSession* matchmake = GetCurrentMatchmakeSession();
    unsigned char payload[cMessageBufferSize];
    unsigned int payloadSize;
    Result result = matchmake->GetSessionSearchCriteriaPtr(index)->Serialize(
        payload, &payloadSize, sizeof(payload) - cMessageHeaderSize - cRequestTrailerSize);
    if (result != 0)
        return result;

    unsigned char* message = m_sendBuffer;
    common::serializeU8(message, cMessageTypeRequest);
    common::serializeU32(message + 1, payloadSize);
    OSBlockMove(message + cMessageHeaderSize, payload, payloadSize, FALSE);
    unsigned char* trailer = message + cMessageHeaderSize + payloadSize;
    ++m_browseSequence;
    common::serializeU8(trailer, cMessageTrailerVersion);
    common::serializeU8(trailer + 1, m_protocol->IsCryptoEnabled());
    common::serializeU64(trailer + cTrailerSequenceOffset, m_browseSequence);
    OSBlockMove(trailer + cTrailerKeyParamOffset, m_browseKeyParam, cKeyParamSize, FALSE);
    if (m_protocol->IsCryptoEnabled())
    {
        unsigned char iv[cIvSize];
        common::serializeU32(iv, m_protocol->m_broadcastAddress);
        common::serializeU64(iv + 4, m_browseSequence);
        unsigned char key[cKeySize];
        result = m_protocol->CalculateCryptionKeyForBrowse(m_browseKeyParam, cKeyParamSize, key, cKeySize);
        common::Crypto::Setting setting;
        setting.m_mode = common::Crypto::cModeAesGcm;
        setting.m_iv = iv;
        setting.m_ivLength = cIvSize;
        setting.m_key = key;
        setting.m_keyLength = cKeySize;
        common::Crypto::Encrypt(trailer + cTrailerTokenOffset, m_browseToken, cTrailerTokenSize,
                                trailer + cTrailerSignatureOffset, cTrailerHashSize, setting);
    }
    else
    {
        pead::Random random;
        for (unsigned int offset = 0; offset < cTrailerHashSize; ++offset)
            trailer[cTrailerSignatureOffset + offset] = static_cast<unsigned char>(random.getU32());
        for (unsigned int offset = 0; offset < cTrailerTokenSize; ++offset)
            trailer[cTrailerTokenOffset + offset] = static_cast<unsigned char>(random.getU32());
    }
    const unsigned int sendSize = payloadSize + cMessageHeaderSize + cRequestTrailerSize;
    int sentSize;
    result = m_socket.SendTo(message, sendSize, common::InetAddress(m_protocol->m_broadcastAddress, cBrowsePort),
                             &sentSize);
    if (result != 0 && result != cResultSocketNoBufferSpace)
        NotifyDisconnectToMeshLayerController();
    matchmake->GetSessionSearchCriteriaPtr(index)->Trace(cTraceMessage);
    return result;
}

LanMatchmakeUpdateJob::ExecuteResult LanMatchmakeUpdateJob::Update()
{
    common::InetAddress sender;
    if (m_randomSeedPending)
    {
        const unsigned long long first = common::crypto::GenerateRandomSeed();
        const unsigned long long second = common::crypto::GenerateRandomSeed();
        if (m_protocol != 0)
            m_protocol->SetRandomSeed(first, second);
        m_randomSeedPending = false;
    }

    LanMatchmakeSession* matchmake = GetCurrentMatchmakeSession();
    if (matchmake == 0)
        return MakeExecuteResult(cExecuteKindWait, cWaitMsec);

    const bool isLeavingMesh = session::g_piaMesh->IsProcessingLeaveMesh();
    bool isCryptoActive = false;
    if (m_protocol->IsCryptoEnabled() && m_protocol->IsCryptoSettingRegistered())
        isCryptoActive = true;
    bool isAuthenticated = true;
    bool omitSessionParam = false;
    unsigned char ancillary[4];
    int receivedSize;
    unsigned char peerParam[cSessionParamSize];
    unsigned char token[cTrailerTokenSize + 4];
    const Result receiveResult = m_socket.RecvFrom(m_receiveBuffer, cMessageBufferSize, &sender, ancillary,
                                                   &receivedSize);
    if (receiveResult == 0)
    {
        if (sender == common::InetAddress(m_protocol->m_address, m_protocol->m_filterPort))
            return MakeExecuteResult(cExecuteKindWait, cWaitMsec);

        const unsigned char type = m_receiveBuffer[0];
        unsigned int payloadSize = common::deserializeU32(m_receiveBuffer + 1);
        bool isValid = true;
        if (payloadSize + cMessageHeaderSize >= receivedSize)
        {
            omitSessionParam = true;
        }
        else if ((type == cMessageTypeRequest || type == cMessageTypeReply)
                 && payloadSize + cMessageHeaderSize + cTrailerMinimumSize <= receivedSize)
        {
            const unsigned char* trailer = m_receiveBuffer + (payloadSize + cMessageHeaderSize);
            common::deserializeU8(trailer);
            bool isEncrypted = false;
            if (common::deserializeU8(trailer + 1) & cTrailerFlagCrypto)
                isEncrypted = true;
            if (isCryptoActive != isEncrypted)
            {
                isValid = false;
            }
            else if (isCryptoActive && isEncrypted)
            {
                unsigned char iv[cIvSize];
                common::serializeU32(iv, m_protocol->m_broadcastAddress);
                OSBlockMove(iv + 4, trailer + cTrailerSequenceOffset, 8, FALSE);
                if (type == cMessageTypeRequest)
                {
                    if (payloadSize + cMessageHeaderSize + cRequestTrailerSize > receivedSize)
                    {
                        isValid = false;
                    }
                    else
                    {
                        OSBlockMove(peerParam, trailer + cTrailerKeyParamOffset, cKeyParamSize, FALSE);
                        unsigned char key[cKeySize];
                        m_protocol->CalculateCryptionKeyForBrowse(peerParam, cKeyParamSize, key, cKeySize);
                        common::Crypto::Setting setting;
                        setting.m_mode = common::Crypto::cModeAesGcm;
                        setting.m_iv = iv;
                        setting.m_ivLength = cIvSize;
                        setting.m_key = key;
                        setting.m_keyLength = cKeySize;
                        if (common::Crypto::Decrypt(token, trailer + cTrailerTokenOffset, cTrailerTokenSize,
                                                    const_cast<unsigned char*>(trailer) + cTrailerSignatureOffset,
                                                    cTrailerHashSize, setting)
                            != 0)
                            isAuthenticated = false;
                    }
                }
                else if (type == cMessageTypeReply)
                {
                    if (payloadSize + cMessageHeaderSize + cReplyTrailerSize > receivedSize)
                    {
                        isValid = false;
                    }
                    else
                    {
                        OSBlockMove(peerParam, trailer + cTrailerKeyParamOffset, cKeyParamSize, FALSE);
                        OSBlockMove(peerParam + cKeyParamSize, m_browseKeyParam, cKeyParamSize, FALSE);
                        unsigned char key[cKeySize];
                        m_protocol->CalculateCryptionKeyForBrowseReply(peerParam, cSessionParamSize, key, cKeySize);
                        common::Crypto::Setting setting;
                        setting.m_mode = common::Crypto::cModeAesGcm;
                        setting.m_iv = iv;
                        setting.m_ivLength = cIvSize;
                        setting.m_key = key;
                        setting.m_keyLength = cKeySize;
                        if (common::Crypto::Decrypt(token, trailer + cTrailerTokenOffset, cTrailerHashSize,
                                                    const_cast<unsigned char*>(trailer) + cTrailerSignatureOffset,
                                                    cTrailerHashSize, setting)
                            != 0)
                        {
                            isAuthenticated = false;
                        }
                        else
                        {
                            unsigned char expectedHash[cTrailerHashSize];
                            m_protocol->CalculateTokenHash(m_browseToken, cBrowseTokenSize, expectedHash,
                                                           cTrailerHashSize);
                            for (unsigned int index = 0; index < cTrailerHashSize; ++index)
                            {
                                if (token[index] != expectedHash[index])
                                {
                                    isAuthenticated = false;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            isValid = false;
        }

        if (isValid)
        {
            switch (type)
            {
            case cMessageTypeRequest:
                if (!isLeavingMesh && matchmake->IsMatchmakeOwner())
                {
                    m_searchCriteria.Deserialize(m_receiveBuffer + cMessageHeaderSize, payloadSize);
                    m_searchCriteria.Trace(cTraceMessage);
                    matchmake->GetSessionInfoPtr()->Trace(cTraceMessage);
                    if (m_searchCriteria.IsHit(matchmake->GetSessionInfoPtr()))
                        SendBrowseReplyMessage(sender, peerParam, cKeyParamSize, token, cTrailerTokenSize,
                                               isAuthenticated, omitSessionParam);
                }
                break;
            case cMessageTypeReply:
                if (!isLeavingMesh && isAuthenticated)
                {
                    if (omitSessionParam)
                    {
                        OSBlockSet(m_sendBuffer + cMessageHeaderSize + payloadSize, 0, cSessionParamSize);
                        payloadSize += cSessionParamSize;
                    }
                    matchmake->GetSessionInfoBufferPtr()->Deserialize(m_receiveBuffer + cMessageHeaderSize,
                                                                      payloadSize);
                    session::ISessionInfoList* list = matchmake->GetSessionInfoListPtr();
                    if (list != 0)
                    {
                        session::ISessionInfo* entry = 0;
                        for (session::ISessionInfo** it = list->Begin(); it != list->End(); ++it)
                        {
                            if ((*it)->GetSessionId() == matchmake->GetSessionInfoBufferPtr()->GetSessionId())
                            {
                                entry = *it;
                                break;
                            }
                        }
                        if (entry == 0)
                        {
                            session::SessionInfoList<LanSessionInfoPrivate>* infoList =
                                static_cast<session::SessionInfoList<LanSessionInfoPrivate>*>(
                                    matchmake->GetSessionInfoListPtr());
                            if (infoList->m_maxSize != 0 && infoList->m_size < infoList->m_maxSize)
                            {
                                entry = infoList->m_items[infoList->m_size];
                                entry->Clear();
                                ++infoList->m_size;
                            }
                        }
                        if (entry != 0)
                        {
                            static_cast<LanSessionInfo*>(entry)->Deserialize(m_receiveBuffer + cMessageHeaderSize,
                                                                             payloadSize);
                            entry->Trace(cTraceSessionInfo);
                        }
                    }
                }
                break;
            }
        }
    }

    if (m_browseRequested)
    {
        if (isCryptoActive)
        {
            common::crypto::GenerateRandomParameter(m_browseKeyParam, sizeof(m_browseKeyParam));
            common::crypto::GenerateRandomParameter(m_browseToken, sizeof(m_browseToken));
        }
        for (unsigned char index = 0; index < m_browseRequestCount; ++index)
            SendBrowseMessage(index);
        m_browseRequested = false;
        const common::TimeSpan& ticksPerMsec = common::TimeSpan::GetTicksPerMSec();
        common::Time deadline;
        deadline.SetNow();
        deadline.m_tick += ticksPerMsec.m_tick * cBrowseTimeoutMsec;
        m_browseDeadline = deadline;
        return MakeExecuteResult(cExecuteKindWait, cWaitMsec);
    }

    if (m_browseContext.m_state == common::CallContext::cStateInProgress)
    {
        common::Time now;
        now.SetNow();
        if (m_browseDeadline < now)
            m_browseContext.SignalSuccess(0);
    }
    return MakeExecuteResult(cExecuteKindWait, cWaitMsec);
}

bool LanMatchmakeUpdateJob::RequestBrowseMessage(unsigned char count)
{
    if (count >= 1 && count <= cBrowseRequestCountMax
        && m_browseContext.m_state != common::CallContext::cStateInProgress)
    {
        m_browseRequestCount = count;
        m_browseRequested = true;
        m_browseContext.Reset();
        m_browseContext.InitiateCall();
        return true;
    }
    return false;
}

bool LanMatchmakeUpdateJob::IsBrowseMessageCompleted() const
{
    return m_browseContext.m_state == common::CallContext::cStateSuccess
           || m_browseContext.m_state == common::CallContext::cStateFailure
           || m_browseContext.m_state == common::CallContext::cStateCancelled;
}

}
}
}

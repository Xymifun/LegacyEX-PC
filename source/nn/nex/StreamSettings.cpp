#include "nn/nex/StreamSettings.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/KeyedChecksumAlgorithm.hpp"
#include "nn/nex/MD5.hpp"
#include "nn/nex/MD5Checksum.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

PseudoGlobalVariable<StreamSettings> g_streamSettings[StreamSettings::StreamTypeCount];

StreamSettings::StreamSettings() : m_bundling(), m_encryptionKey()
{
    m_iStreamType = -1;
    m_ucKeyByteSum = 0;
    m_iKeySum = 0;
    m_pCompressionAlgorithm = 0;
    m_pEncryptionAlgorithm = 0;
    m_uiMaxRetransmission = 20;
    m_uiKeepAliveTimeout = 1000;
    m_bEarlyActivityTimestamps = true;
    m_uiMaxSilenceTime = 10000;
    m_uiWindowSize = 32;
    m_uiMaxBufferedPackets = 128;
    m_fRetransmitTimeoutMultiplier = 1.25f;
    m_fExtraRetransmitTimeoutMultiplier = 1.0f;
    m_uiExtraRetransmitTimeoutTrigger = 50;
    m_uiInitialRTT = 750;
    m_uiSynInitialRTT = 250;
    m_uiRTTRetransmit = 2;
    m_uiMaxRetransmitTimeout = 2000;
    m_bFaultDetectionActive = true;
    m_bUnknownB6 = false;
    m_bChangeSettingsAllowed = true;
    m_uiMaxSubStreamID = 0;
    m_iSecureMode = 2;
    m_pChecksumAlgorithm = 0;
    m_bIsEncryptionRequired = false;
}

StreamSettings::~StreamSettings()
{
}

StreamSettings& StreamSettings::operator=(const StreamSettings& rOther)
{
    m_iStreamType = rOther.m_iStreamType;
    m_ucKeyByteSum = rOther.m_ucKeyByteSum;
    m_iKeySum = rOther.m_iKeySum;
    m_pCompressionAlgorithm = rOther.m_pCompressionAlgorithm;
    m_pEncryptionAlgorithm = rOther.m_pEncryptionAlgorithm;
    m_bundling = rOther.m_bundling;
    m_uiMaxRetransmission = rOther.m_uiMaxRetransmission;
    m_uiKeepAliveTimeout = rOther.m_uiKeepAliveTimeout;
    m_bEarlyActivityTimestamps = rOther.m_bEarlyActivityTimestamps;
    m_uiMaxSilenceTime = rOther.m_uiMaxSilenceTime;
    m_uiWindowSize = rOther.m_uiWindowSize;
    m_uiMaxBufferedPackets = rOther.m_uiMaxBufferedPackets;
    m_fRetransmitTimeoutMultiplier = rOther.m_fRetransmitTimeoutMultiplier;
    m_fExtraRetransmitTimeoutMultiplier = rOther.m_fExtraRetransmitTimeoutMultiplier;
    m_uiExtraRetransmitTimeoutTrigger = rOther.m_uiExtraRetransmitTimeoutTrigger;
    m_uiInitialRTT = rOther.m_uiInitialRTT;
    m_uiSynInitialRTT = rOther.m_uiSynInitialRTT;
    m_uiRTTRetransmit = rOther.m_uiRTTRetransmit;
    m_uiMaxRetransmitTimeout = rOther.m_uiMaxRetransmitTimeout;
    m_bUnknownB4 = rOther.m_bUnknownB4;
    m_bFaultDetectionActive = rOther.m_bFaultDetectionActive;
    m_bUnknownB6 = rOther.m_bUnknownB6;
    m_bChangeSettingsAllowed = rOther.m_bChangeSettingsAllowed;
    m_uiMaxSubStreamID = rOther.m_uiMaxSubStreamID;
    m_iSecureMode = rOther.m_iSecureMode;
    m_pChecksumAlgorithm = rOther.m_pChecksumAlgorithm;
    m_bIsEncryptionRequired = rOther.m_bIsEncryptionRequired;
    m_encryptionKey = rOther.m_encryptionKey;
    return *this;
}

unsigned int StreamSettings::GetWindowSize() const
{
    return m_uiWindowSize;
}

unsigned int StreamSettings::GetMaxRetransmission() const
{
    return m_uiMaxRetransmission;
}

unsigned int StreamSettings::GetKeepAliveTimeout() const
{
    return m_uiKeepAliveTimeout;
}

qResult StreamSettings::SetKeepAliveTimeout(unsigned int uiKeepAliveTimeout)
{
    m_uiKeepAliveTimeout = uiKeepAliveTimeout;
    return qResult(RESULT_SUCCESS);
}

unsigned int StreamSettings::GetMaxSilenceTime() const
{
    return m_uiMaxSilenceTime;
}

qResult StreamSettings::SetMaxSilenceTime(unsigned int uiMaxSilenceTime)
{
    m_uiMaxSilenceTime = uiMaxSilenceTime;
    return qResult(RESULT_SUCCESS);
}

float StreamSettings::GetRetransmitTimeoutMultiplier() const
{
    return m_fRetransmitTimeoutMultiplier;
}

float StreamSettings::GetExtraRetransmitTimeoutMultiplier() const
{
    return m_fExtraRetransmitTimeoutMultiplier;
}

unsigned int StreamSettings::GetExtraRetransmitTimeoutTrigger() const
{
    return m_uiExtraRetransmitTimeoutTrigger;
}

unsigned int StreamSettings::GetInitialRTT() const
{
    return m_uiInitialRTT;
}

unsigned int StreamSettings::GetSynInitialRTT() const
{
    return m_uiSynInitialRTT;
}

unsigned int StreamSettings::GetRTTRetransmit() const
{
    return m_uiRTTRetransmit;
}

bool StreamSettings::IgnoreChangeSettings()
{
    if (m_bChangeSettingsAllowed)
        return false;
    return true;
}

void StreamSettings::SetKey(const String& rKey)
{
    if (IgnoreChangeSettings())
        return;

    char* pCopy;
    rKey.CreateCopy(&pCopy);
    unsigned char ucByteSum = 0;
    int iSum = 0;
    for (const char* pChar = pCopy; *pChar != 0; ++pChar)
    {
        ucByteSum += *pChar;
        iSum += *pChar;
    }

    String::ReleaseCopy(pCopy);
    m_ucKeyByteSum = ucByteSum;
    m_iKeySum = iSum;
    if (m_pChecksumAlgorithm != 0)
    {
        MD5Checksum md5Checksum;
        Buffer keyBuffer(Buffer::s_uiDefaultBufferSize, 0);
        Buffer digest(MD5::DigestSize, 0);
        char* pKeyCopy;
        rKey.CreateCopy(&pKeyCopy);
        keyBuffer.AppendData(pKeyCopy, rKey.GetLength(), 0xFFFFFFFFu);
        String::ReleaseCopy(pKeyCopy);
        md5Checksum.ComputeChecksum(keyBuffer, &digest);
        KeyedChecksumAlgorithm* pChecksumAlgorithm = m_pChecksumAlgorithm;
        pChecksumAlgorithm->SetKey(Key(digest.GetData(), digest.GetLength()));
    }
}

void StreamSettings::SetIsEncryptionRequired(bool bIsEncryptionRequired)
{
    if (!IgnoreChangeSettings())
        m_bIsEncryptionRequired = bIsEncryptionRequired;
}

bool StreamSettings::FaultDetectionIsActive()
{
    return m_bFaultDetectionActive;
}

}
}

#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STREAMSETTINGS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STREAMSETTINGS_HPP

#include "nn/nex/LiteBuffer.hpp"
#include "nn/nex/PseudoGlobalVariable.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/StreamBundling.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class CompressionAlgorithm;
class EncryptionAlgorithm;
class KeyedChecksumAlgorithm;
class String;

class StreamSettings : public RootObject
{
public:
    enum
    {
        StreamTypeCount = 16,
        KEY_BUFFER_SIZE = 32
    };

    StreamSettings();
    virtual ~StreamSettings();

    StreamSettings& operator=(const StreamSettings& rOther);

    unsigned int GetWindowSize() const;
    unsigned int GetMaxRetransmission() const;
    unsigned int GetKeepAliveTimeout() const;
    qResult SetKeepAliveTimeout(unsigned int uiKeepAliveTimeout);
    unsigned int GetMaxSilenceTime() const;
    qResult SetMaxSilenceTime(unsigned int uiMaxSilenceTime);
    float GetRetransmitTimeoutMultiplier() const;
    float GetExtraRetransmitTimeoutMultiplier() const;
    unsigned int GetExtraRetransmitTimeoutTrigger() const;
    unsigned int GetInitialRTT() const;
    unsigned int GetSynInitialRTT() const;
    unsigned int GetRTTRetransmit() const;
    bool IgnoreChangeSettings();
    void SetKey(const String& rKey);
    void SetIsEncryptionRequired(bool bIsEncryptionRequired);
    bool FaultDetectionIsActive();

    StreamBundling& GetBundling() { return m_bundling; }

    unsigned char GetKeyByteSum() const { return m_ucKeyByteSum; }

    int GetKeySum() const { return m_iKeySum; }

    CompressionAlgorithm* GetCompressionAlgorithm() const { return m_pCompressionAlgorithm; }

    void SetCompressionAlgorithm(CompressionAlgorithm* pCompressionAlgorithm)
    {
        m_pCompressionAlgorithm = pCompressionAlgorithm;
    }

    EncryptionAlgorithm* GetEncryptionAlgorithm() const { return m_pEncryptionAlgorithm; }

    void SetEncryptionAlgorithm(EncryptionAlgorithm* pEncryptionAlgorithm)
    {
        m_pEncryptionAlgorithm = pEncryptionAlgorithm;
    }

    KeyedChecksumAlgorithm* GetChecksumAlgorithm() const { return m_pChecksumAlgorithm; }

    void SetChecksumAlgorithm(KeyedChecksumAlgorithm* pChecksumAlgorithm) { m_pChecksumAlgorithm = pChecksumAlgorithm; }

    void SetStreamType(int iStreamType) { m_iStreamType = iStreamType; }

    bool UsesEarlyActivityTimestamps() const { return m_bEarlyActivityTimestamps; }

    unsigned int GetMaxBufferedPackets() const { return m_uiMaxBufferedPackets; }

    unsigned int GetMaxRetransmitTimeout() const { return m_uiMaxRetransmitTimeout; }

    unsigned char GetMaxSubStreamID() const { return m_uiMaxSubStreamID; }

    int GetSecureMode() const { return m_iSecureMode; }

    const LiteBuffer<KEY_BUFFER_SIZE>& GetEncryptionKey() const { return m_encryptionKey; }

    bool IsEncryptionRequired() const { return m_bIsEncryptionRequired; }

    qResult SetWindowSize(unsigned int uiWindowSize)
    {
        m_uiWindowSize = uiWindowSize;
        return qResult(RESULT_SUCCESS);
    }

    qResult SetMaxRetransmission(unsigned int uiMaxRetransmission)
    {
        m_uiMaxRetransmission = uiMaxRetransmission;
        return qResult(RESULT_SUCCESS);
    }

    qResult SetRetransmitTimeoutMultiplier(float fRetransmitTimeoutMultiplier)
    {
        m_fRetransmitTimeoutMultiplier = fRetransmitTimeoutMultiplier;
        return qResult(RESULT_SUCCESS);
    }

    qResult SetExtraRetransmitTimeoutMultiplier(float fExtraRetransmitTimeoutMultiplier)
    {
        m_fExtraRetransmitTimeoutMultiplier = fExtraRetransmitTimeoutMultiplier;
        return qResult(RESULT_SUCCESS);
    }

    qResult SetExtraRetransmitTimeoutTrigger(unsigned int uiExtraRetransmitTimeoutTrigger)
    {
        m_uiExtraRetransmitTimeoutTrigger = uiExtraRetransmitTimeoutTrigger;
        return qResult(RESULT_SUCCESS);
    }

    qResult SetInitialRTT(unsigned int uiInitialRTT)
    {
        m_uiInitialRTT = uiInitialRTT;
        return qResult(RESULT_SUCCESS);
    }

    qResult SetRTTRetransmit(unsigned int uiRTTRetransmit)
    {
        m_uiRTTRetransmit = uiRTTRetransmit;
        return qResult(RESULT_SUCCESS);
    }

    qResult SetSynInitialRTT(unsigned int uiSynInitialRTT)
    {
        m_uiSynInitialRTT = uiSynInitialRTT;
        return qResult(RESULT_SUCCESS);
    }

    void SetActiveFaultDetection(bool bActive) { m_bFaultDetectionActive = bActive; }

private:
    enum
    {
        RESULT_SUCCESS = 0x10001
    };

    int m_iStreamType;
    unsigned char m_ucKeyByteSum;
    int m_iKeySum;
    CompressionAlgorithm* m_pCompressionAlgorithm;
    EncryptionAlgorithm* m_pEncryptionAlgorithm;
    StreamBundling m_bundling;
    unsigned int m_uiMaxRetransmission;
    unsigned int m_uiKeepAliveTimeout;
    bool m_bEarlyActivityTimestamps;
    unsigned int m_uiMaxSilenceTime;
    unsigned int m_uiWindowSize;
    unsigned int m_uiMaxBufferedPackets;
    float m_fRetransmitTimeoutMultiplier;
    float m_fExtraRetransmitTimeoutMultiplier;
    unsigned int m_uiExtraRetransmitTimeoutTrigger;
    unsigned int m_uiInitialRTT;
    unsigned int m_uiSynInitialRTT;
    unsigned int m_uiRTTRetransmit;
    unsigned int m_uiMaxRetransmitTimeout;
    bool m_bUnknownB4;
    bool m_bFaultDetectionActive;
    bool m_bUnknownB6;
    bool m_bChangeSettingsAllowed;
    unsigned char m_uiMaxSubStreamID;
    int m_iSecureMode;
    KeyedChecksumAlgorithm* m_pChecksumAlgorithm;
    bool m_bIsEncryptionRequired;
    LiteBuffer<KEY_BUFFER_SIZE> m_encryptionKey;
};

extern PseudoGlobalVariable<StreamSettings> g_streamSettings[StreamSettings::StreamTypeCount];

}
}

#endif

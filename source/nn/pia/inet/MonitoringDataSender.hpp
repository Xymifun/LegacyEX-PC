#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_MONITORING_DATA_SENDER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_MONITORING_DATA_SENDER_HPP

#include "nn/pia/common/MonitoringDataSender.hpp"
#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/inet/MonitoringDataSendJob.hpp"
#include "nn/pia/inet/MonitoringServerAddressResolveJob.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class MonitoringDataProtocol;

class MonitoringDataSender : public common::MonitoringDataSender
{
public:
    enum
    {
        cReportBufferSize = 2960,
        cCompressBufferSize = 3024,
        cCompressWorkBufferSize = 9924,
        cBufferAlignment = 32
    };

    enum
    {
        cDataTypeNum = 3
    };

    enum
    {
        cIvSize = 12,
        cIdentifierSize = 8,
        cCryptoKeySize = 16,
        cCryptoKeyNum = 16,
        cSignatureSize = 16,
        cCryptoBlockSize = 16,
        cMaxReportSize = 1232
    };

    enum
    {
        cCompressionLevel = 9,
        cCompressionWindowBits = 9,
        cCompressionMemLevel = 2
    };

    enum MakeReportDataResult
    {
        cMakeReportDataSuccess = 0,
        cMakeReportDataTooLarge = 1,
        cMakeReportDataFailure = 2
    };

    MonitoringDataSender();
    virtual ~MonitoringDataSender();
    virtual Result Startup();
    virtual void Cleanup();
    virtual void SetSendSessionBeginReportRequested(bool requested);
    virtual unsigned char IsSendSessionBeginReportRequested() const;
    void GetIv(unsigned char* iv, unsigned char* identifier) const;
    virtual void Trace(unsigned long long value) const;
    virtual unsigned int IsEnableMonitoring() const;
    bool IsResolutionInProgress() const;
    const common::InetAddress& GetServerAddress() const;
    DataType GetDataType() const;
    const unsigned char* GetCryptoKey(unsigned char index) const;
    MakeReportDataResult MakeReportData(DataType dataType, bool resetStateContent);
    void SendReport();
    virtual void RequestSendReportImpl(DataType dataType);
    virtual unsigned int IsPreparedForCleanup() const;

private:
    unsigned char* m_reportBuffer;
    unsigned char* m_compressBuffer;
    unsigned char* m_compressWorkBuffer;
    unsigned int m_reportSize;
    DataType m_dataType;
    MonitoringDataSendJob m_sendJob;
    MonitoringServerAddressResolveJob m_serverAddressResolveJob;
    bool m_sendSessionBeginReportRequested;
    common::InetAddress m_serverAddress;
    unsigned short m_protocolHandle;
    MonitoringDataProtocol* m_protocol;
};

}
}
}

#endif

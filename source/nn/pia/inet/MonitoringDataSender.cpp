#include "nn/pia/inet/MonitoringDataSender.hpp"
#include "nn/nex/NgsBridgeInterface.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Crypto.hpp"
#include "nn/pia/common/SessionBeginMonitoringData.hpp"
#include "nn/pia/common/SessionEndMonitoringData.hpp"
#include "nn/pia/common/ZlibCompressor.hpp"
#include "nn/pia/inet/MonitoringDataProtocol.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "platform/wiiu/pead.hpp"

#include <coreinit/memory.h>

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{

enum
{
    cHeaderContentSizeOffset = 4,
    cHeaderAttributeMask = 3,
    cStateContentAttributeMask = 4
};

const unsigned char s_ivTemplate[MonitoringDataSender::cIvSize] = {0, 0, 0, 0, 0, 0, 0, 0, 0x5b, 0xd0, 0x85, 0xfa};

const unsigned char s_cryptoKeys[MonitoringDataSender::cCryptoKeyNum][MonitoringDataSender::cCryptoKeySize] = {
#include "platform/wiiu/shaders/MonitoringCryptoKeys.inc"
};

common::SessionBeginMonitoringData s_sessionBeginMonitoringData;
common::SessionEndMonitoringData s_sessionEndMonitoringData;

}

MonitoringDataSender::MonitoringDataSender()
    : m_reportSize(0),
      m_dataType(static_cast<DataType>(cDataTypeNum)),
      m_sendSessionBeginReportRequested(false)
{
    m_reportBuffer = pead::NewArray<unsigned char>(cReportBufferSize, common::HeapManager::GetHeap(), cBufferAlignment);
    m_compressBuffer = pead::NewArray<unsigned char>(cCompressBufferSize, common::HeapManager::GetHeap(),
                                                     cBufferAlignment);
    m_compressWorkBuffer = pead::NewArray<unsigned char>(cCompressWorkBufferSize, common::HeapManager::GetHeap(),
                                                         cBufferAlignment);
    transport::Transport* transport = transport::g_piaTransport;
    m_protocolHandle = static_cast<unsigned short>(
        transport->m_protocolManager.CreateProtocol<MonitoringDataProtocol>(0));
    m_protocol = transport->m_protocolManager.GetProtocol<MonitoringDataProtocol>(m_protocolHandle);
    m_serverAddress.m_address = 0;
    m_serverAddress.m_port = 0;
}

MonitoringDataSender::~MonitoringDataSender()
{
    if (m_reportBuffer != 0)
    {
        pead::DeleteArray(m_reportBuffer);
        m_reportBuffer = 0;
    }
    if (m_compressBuffer != 0)
    {
        pead::DeleteArray(m_compressBuffer);
        m_compressBuffer = 0;
    }
    if (m_compressWorkBuffer != 0)
    {
        pead::DeleteArray(m_compressWorkBuffer);
        m_compressWorkBuffer = 0;
    }
    if (m_protocol != 0)
    {
        transport::g_piaTransport->m_protocolManager.DestroyProtocol(m_protocolHandle);
        m_protocolHandle = 0;
        m_protocol = 0;
    }
}

Result MonitoringDataSender::Startup()
{
    return Result(0);
}

void MonitoringDataSender::Cleanup()
{
    m_serverAddressResolveJob.Cleanup();
    m_sendJob.Cleanup();
    m_sendJob.Reset(false);
    OSBlockSet(m_reportBuffer, 0, cReportBufferSize);
    OSBlockSet(m_compressBuffer, 0, cCompressBufferSize);
    OSBlockSet(m_compressWorkBuffer, 0, cCompressWorkBufferSize);
    m_serverAddress.m_port = 0;
    m_dataType = static_cast<DataType>(cDataTypeNum);
    m_serverAddress.m_address = 0;
    m_reportSize = 0;
    m_sendSessionBeginReportRequested = false;
}

void MonitoringDataSender::SetSendSessionBeginReportRequested(bool requested)
{
    m_sendSessionBeginReportRequested = requested;
}

unsigned char MonitoringDataSender::IsSendSessionBeginReportRequested() const
{
    return m_sendSessionBeginReportRequested;
}

void MonitoringDataSender::GetIv(unsigned char* iv, unsigned char* identifier) const
{
    unsigned int index = 0;
    while (index < cIvSize)
    {
        if (index < cIdentifierSize)
            iv[index] = identifier[index];
        else
            iv[index] = s_ivTemplate[index];
        ++index;
    }
}

const unsigned char* MonitoringDataSender::GetCryptoKey(unsigned char index) const
{
    return s_cryptoKeys[index % cCryptoKeyNum];
}

MonitoringDataSender::MakeReportDataResult MonitoringDataSender::MakeReportData(DataType dataType,
                                                                                bool resetStateContent)
{
    unsigned int compressedSize = 0;
    Result result;
    unsigned int serializedSize = 0;
    OSBlockSet(m_reportBuffer, 0, cReportBufferSize);
    OSBlockSet(m_compressBuffer, 0, cCompressBufferSize);
    OSBlockSet(m_compressWorkBuffer, 0, cCompressWorkBufferSize);

    common::Crypto::Setting setting;
    setting.m_mode = common::Crypto::cModeAesGcm;
    setting.m_ivLength = cIvSize;
    setting.m_keyLength = cCryptoKeySize;
    unsigned char iv[cIvSize];

    Result serializeResult;
    if (dataType == cDataTypeSessionBegin)
    {
        s_sessionBeginMonitoringData.Initialize();
        s_sessionBeginMonitoringData.m_content = common::g_sessionBeginMonitoringContent;
        s_sessionBeginMonitoringData.m_attribute &= ~cHeaderAttributeMask;
        serializeResult = s_sessionBeginMonitoringData.Serialize(m_reportBuffer, &serializedSize, cReportBufferSize);
        GetIv(iv, s_sessionBeginMonitoringData.m_identifier);
        setting.m_iv = iv;
        setting.m_key = GetCryptoKey(s_sessionBeginMonitoringData.m_keyIndex);
    }
    else
    {
        if (dataType == cDataTypeSessionEnd)
            s_sessionEndMonitoringData.Initialize(cDataTypeSessionEnd);
        else if (dataType == cDataType2)
            s_sessionEndMonitoringData.Initialize(cDataType2);
        s_sessionEndMonitoringData.m_beginContent = common::g_sessionBeginMonitoringContent;
        if (resetStateContent)
        {
            s_sessionEndMonitoringData.m_stateContent.Initialize();
            s_sessionEndMonitoringData.m_stateContent.m_attribute &= ~cStateContentAttributeMask;
        }
        else
        {
            s_sessionEndMonitoringData.m_stateContent = common::g_sessionStateMonitoringContent;
        }
        s_sessionEndMonitoringData.m_attribute &= ~cHeaderAttributeMask;
        serializeResult = s_sessionEndMonitoringData.Serialize(m_reportBuffer, &serializedSize, cReportBufferSize);
        GetIv(iv, s_sessionEndMonitoringData.m_identifier);
        setting.m_iv = iv;
        setting.m_key = GetCryptoKey(s_sessionEndMonitoringData.m_keyIndex);
    }
    if (serializeResult != 0 || serializedSize == 0 || serializedSize <= sizeof(common::MonitoringDataHeader))
        return cMakeReportDataFailure;

    common::ZlibCompressor compressor;
    result = compressor.Initialize(m_compressWorkBuffer, cCompressWorkBufferSize);
    if (result != 0)
        return cMakeReportDataFailure;
    result = compressor.Startup(m_compressBuffer, cCompressBufferSize, cCompressionLevel, cCompressionWindowBits,
                                cCompressionMemLevel);
    if (result == 0)
    {
        if (compressor.Deflate(m_reportBuffer + sizeof(common::MonitoringDataHeader),
                               serializedSize - sizeof(common::MonitoringDataHeader))
            == 0)
        {
            unsigned int outputSize;
            compressedSize = compressor.FinishDeflate(&outputSize) == 0 ? static_cast<unsigned short>(outputSize) : 0;
        }
    }
    compressor.Cleanup();
    compressor.Finalize();
    if (compressedSize == 0)
        return cMakeReportDataFailure;

    unsigned int encryptedSize = (compressedSize & ~(cCryptoBlockSize - 1)) + cCryptoBlockSize;
    unsigned int reportSize = sizeof(common::MonitoringDataHeader) + encryptedSize + cSignatureSize;
    if (reportSize > cMaxReportSize)
        return cMakeReportDataTooLarge;

    common::serializeU16(m_reportBuffer + cHeaderContentSizeOffset, static_cast<unsigned short>(compressedSize));
    const unsigned char padding = ~m_compressBuffer[compressedSize - 1];
    while (compressedSize < encryptedSize)
        m_compressBuffer[compressedSize++] = padding;

    unsigned char signature[cSignatureSize];
    OSBlockSet(m_reportBuffer + sizeof(common::MonitoringDataHeader), 0,
               cReportBufferSize - sizeof(common::MonitoringDataHeader));
    result = common::Crypto::Encrypt(m_reportBuffer + sizeof(common::MonitoringDataHeader), m_compressBuffer,
                                     encryptedSize, signature, cSignatureSize, setting);
    if (result != 0)
        return cMakeReportDataFailure;
    OSBlockMove(m_reportBuffer + sizeof(common::MonitoringDataHeader) + encryptedSize, signature, cSignatureSize,
                false);
    m_reportSize = reportSize;
    return cMakeReportDataSuccess;
}

void MonitoringDataSender::SendReport()
{
    NexFacade* facade = NexFacade::GetInstance();
    if (facade == 0)
        return;
    nex::NgsBridgeInterface* ngsBridge = facade->m_ngsBridge;
    if (ngsBridge == 0)
        return;
    ngsBridge->SendReport(m_dataType, m_reportBuffer, m_reportSize);
}

void MonitoringDataSender::RequestSendReportImpl(DataType dataType)
{
    MakeReportDataResult result = MakeReportData(dataType, false);
    if (result == cMakeReportDataTooLarge)
        result = MakeReportData(dataType, true);
    if (result == cMakeReportDataSuccess)
    {
        m_dataType = dataType;
        if (dataType == cDataTypeSessionBegin)
            m_sendSessionBeginReportRequested = true;
        SendReport();
    }
}

unsigned int MonitoringDataSender::IsPreparedForCleanup() const
{
    if (m_serverAddressResolveJob.IsRunning() && m_serverAddressResolveJob.IsBackground())
        return false;
    return true;
}

void MonitoringDataSender::Trace(unsigned long long) const
{
}

unsigned int MonitoringDataSender::IsEnableMonitoring() const
{
    return 1;
}

}
}
}

#include "nn/nex/ByteStream.hpp"

#include "nn/nex/StringStream.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

void SetUnalignedNexNetworkOrder64(uint64_t uiValue, unsigned char* pDestination)
{
    pDestination[0] = static_cast<unsigned char>(uiValue & 0x00000000000000FFULL);
    pDestination[1] = static_cast<unsigned char>((uiValue & 0x000000000000FF00ULL) >> 8);
    pDestination[2] = static_cast<unsigned char>((uiValue & 0x0000000000FF0000ULL) >> 16);
    pDestination[3] = static_cast<unsigned char>((uiValue & 0x00000000FF000000ULL) >> 24);
    pDestination[4] = static_cast<unsigned char>((uiValue & 0x000000FF00000000ULL) >> 32);
    pDestination[5] = static_cast<unsigned char>((uiValue & 0x0000FF0000000000ULL) >> 40);
    pDestination[6] = static_cast<unsigned char>((uiValue & 0x00FF000000000000ULL) >> 48);
    pDestination[7] = static_cast<unsigned char>((uiValue & 0xFF00000000000000ULL) >> 56);
}

ByteStream::ByteStream()
{
    m_pBuffer = new Buffer();
    Init();
}

ByteStream::ByteStream(unsigned int uiSize)
{
    m_pBuffer = new Buffer(uiSize, 0);
    Init();
}

ByteStream::ByteStream(Buffer* pBuffer)
{
    m_pBuffer = pBuffer;
    pBuffer->AddRef();
    Init();
}

ByteStream::~ByteStream()
{
    m_pBuffer->ReleaseRef();
    m_pBuffer = 0;
}

void ByteStream::Init()
{
    m_bError = false;
    m_uiPosition = 0;
}

void ByteStream::Clear()
{
    m_pBuffer->Clear();
    Init();
}

bool ByteStream::SetLength(unsigned int uiLength)
{
    if (uiLength > m_pBuffer->GetRealSize() - m_pBuffer->GetHeadShiftSize())
    {
        return false;
    }

    m_pBuffer->SetLength(uiLength);
    return true;
}

bool ByteStream::ValidateBufferLimit(unsigned int uiSize)
{
    if (GetLength() - GetPosition() >= uiSize)
    {
        return true;
    }

    bool bValid = false;
    SetError();
    return bValid;
}

bool ByteStream::AppendRaw(const unsigned char* pData, unsigned int uiSize)
{
    return m_pBuffer->AppendData(pData, uiSize, APPEND_AT_END);
}

bool ByteStream::Append(const unsigned char* pData, unsigned int uiSize, unsigned int uiCount)
{
    for (unsigned int uiIndex = 0; uiIndex < uiCount; ++uiIndex)
    {
        for (int iByte = static_cast<int>(uiSize) - 1; iByte >= 0; --iByte)
        {
            AppendRaw(pData + uiIndex * uiSize + iByte, 1);
        }
    }

    return true;
}

bool ByteStream::Append(const Time* pTime)
{
    unsigned char aBytes[8];

    SetUnalignedNexNetworkOrder64(*pTime, aBytes);
    m_pBuffer->AppendData(aBytes, sizeof(aBytes), APPEND_AT_END);
    return true;
}

bool ByteStream::ExtractRaw(unsigned char* pData, unsigned int uiSize)
{
    bool bComplete = true;
    unsigned int uiExtractSize;

    if (!ValidateBufferLimit(uiSize))
    {
        uiExtractSize = m_pBuffer->GetLength() - m_uiPosition;
        bComplete = false;
    }
    else
    {
        uiExtractSize = uiSize;
    }

    if (uiExtractSize != 0)
    {
        m_pBuffer->CopyContent(pData, uiExtractSize, m_uiPosition);
        m_uiPosition += uiExtractSize;
    }

    return bComplete;
}

bool ByteStream::Extract(unsigned char* pData, unsigned int uiSize, unsigned int uiCount)
{
    if (uiSize == 1)
    {
        return ExtractRaw(pData, uiCount);
    }

    bool bComplete = true;

    for (unsigned int uiIndex = 0; uiIndex < uiCount && bComplete; ++uiIndex)
    {
        for (int iByte = static_cast<int>(uiSize) - 1; iByte >= 0 && bComplete; --iByte)
        {
            bComplete = ExtractRaw(pData + uiIndex * uiSize + iByte, 1);
        }
    }

    return bComplete;
}

void ByteStream::Extract(Time* pTime)
{
    uint64_t uiValue;

    if (ExtractPrimitive(&uiValue))
    {
        *pTime = uiValue;
    }
}

ByteStream& ByteStream::operator<<(const Buffer& rBuffer)
{
    AppendRaw(rBuffer.GetData(), rBuffer.GetLength());
    return *this;
}

ByteStream& ByteStream::operator<<(const bool& rValue)
{
    const unsigned char uiValue = static_cast<unsigned char>(rValue);

    Append(&uiValue, 1, 1);
    return *this;
}

ByteStream& ByteStream::operator>>(bool& rValue)
{
    signed char cValue;

    ExtractPrimitive(&cValue);
    rValue = cValue != 0;
    return *this;
}

void ByteStream::TraceBytes(uint64_t uiFilter, unsigned char* pData, unsigned int uiSize)
{
    unsigned int uiIndex = 0;
    StringStream stream;

    zerohex(stream);

    for (; uiIndex < uiSize; ++uiIndex)
    {
        if (uiIndex != 0 && ((uiIndex + 1) & 7) == 0)
        {
            stream.StreamNumber(pData[uiIndex]);
            stream.Clear();
        }
        else
        {
            stream.StreamNumber(pData[uiIndex]) << L" ";
        }
    }
}

void ByteStream::Trace(uint64_t uiFilter, bool bDumpContents)
{
    if (m_pBuffer == 0 || !bDumpContents)
    {
        return;
    }

    TraceBytes(uiFilter, m_pBuffer->GetData(), GetBuffer()->GetLength());
}

}
}

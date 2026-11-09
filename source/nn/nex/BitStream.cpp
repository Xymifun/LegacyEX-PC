#include "nn/nex/BitStream.hpp"

#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

BitStream::BitStream()
{
    m_pBuffer = new Buffer();
    Init();
}

BitStream::BitStream(Buffer* pBuffer)
{
    m_pBuffer = pBuffer;
    m_pBuffer->AddRef();
    Init();
    AdjustLength();
}

BitStream::~BitStream()
{
    m_pBuffer->ReleaseRef();
    m_pBuffer = 0;
}

void BitStream::Init()
{
    m_bError = false;
    m_uiBytePosition = 0;
    m_uiBitPosition = 7;
    m_uiBitLength = 0;
}

void BitStream::AdjustLength()
{
    m_uiBitLength = m_pBuffer->GetLength() * 8;
}

BitStream& BitStream::SetPosition(unsigned int uiPosition)
{
    m_uiBytePosition = uiPosition / 8;
    m_uiBitPosition = ~uiPosition & 7;
    return *this;
}

bool BitStream::CanExtract(unsigned int uiBits) const
{
    return m_uiBytePosition * 8 - m_uiBitPosition + uiBits + 7 <= m_uiBitLength;
}

void BitStream::PrepareAppend(unsigned int uiBits)
{
    BitStream& rStream = SetPosition(m_uiBitLength);
    Buffer* pBuffer = rStream.m_pBuffer;
    unsigned int uiRequired = rStream.m_uiBytePosition + ((uiBits - rStream.m_uiBitPosition + 7) >> 3) + 1;
    unsigned int uiAvailable = pBuffer->GetRealSize() - pBuffer->GetHeadShiftSize();

    if (uiAvailable < uiRequired)
    {
        unsigned int uiGrown = uiAvailable + (uiAvailable >> 1);

        if (uiGrown <= uiRequired)
        {
            uiGrown = uiRequired;
        }

        pBuffer->ResizeByRealSize(uiGrown + pBuffer->GetHeadShiftSize());
    }
}

void BitStream::AppendByteImpl(unsigned char uiByte)
{
    if (m_uiBitPosition != 7)
    {
        unsigned int uiShift = m_uiBitPosition + 1;
        unsigned char uiRightShift = 8 - uiShift;
        unsigned char uiHigh = uiByte >> uiRightShift;
        unsigned char uiLow = uiByte << uiShift;

        (*m_pBuffer)[m_uiBytePosition] = (*m_pBuffer)[m_uiBytePosition] | uiHigh;
        (*m_pBuffer)[m_uiBytePosition + 1] = uiLow;
    }
    else
    {
        (*m_pBuffer)[m_uiBytePosition] = uiByte;
    }

    ++m_uiBytePosition;
    m_uiBitLength += 8;
}

void BitStream::AppendBitImpl(bool bBit)
{
    if (m_uiBitPosition == 7)
    {
        (*m_pBuffer)[m_uiBytePosition] = bBit ? 0x80 : 0;
    }
    else if (bBit)
    {
        (*m_pBuffer)[m_uiBytePosition] = (*m_pBuffer)[m_uiBytePosition]
                                         | (1 << static_cast<unsigned char>(m_uiBitPosition));
    }

    unsigned int uiBitPosition = m_uiBitPosition;

    if (uiBitPosition == 0)
    {
        uiBitPosition = 8;
        ++m_uiBytePosition;
    }

    m_uiBitPosition = uiBitPosition - 1;
    ++m_uiBitLength;
}

void BitStream::AppendRaw(const unsigned char* pData, unsigned int uiBits)
{
    PrepareAppend(uiBits);

    unsigned int uiByte = 0;
    unsigned char uiRemaining = uiBits & 7;

    if (m_uiBitPosition != 7)
    {
        for (unsigned int uiDone = 8; uiDone <= uiBits; uiDone += 8)
        {
            AppendByteImpl(pData[uiByte]);
            ++uiByte;
        }
    }
    else
    {
        uiByte = uiBits / 8;
        m_pBuffer->AppendData(pData, uiByte, m_uiBytePosition);
        m_uiBytePosition += uiByte;
        m_uiBitLength += uiBits & ~7u;
    }

    if (uiRemaining != 0)
    {
        unsigned char uiLast = pData[uiByte];
        unsigned char uiMask = 0x80;

        do
        {
            AppendBitImpl((uiLast & uiMask) != 0);
            --uiRemaining;
            uiMask >>= 1;
        } while (uiRemaining != 0);
    }
}

void BitStream::Append(const unsigned char* pData, unsigned int uiSize, unsigned int uiCount)
{
    for (unsigned int uiIndex = 0; uiIndex < uiCount; ++uiIndex)
    {
        for (int iByte = uiSize; --iByte >= 0;)
        {
            AppendRaw(pData + uiIndex * uiSize + iByte, 8);
        }
    }
}

void BitStream::ExtractByteImpl(unsigned char& rByte)
{
    if (!CanExtract(8))
    {
        m_bError = true;
        return;
    }

    unsigned char uiShift = m_uiBitPosition + 1;
    unsigned char uiHigh = (*m_pBuffer)[m_uiBytePosition];
    unsigned char uiLow = (*m_pBuffer)[m_uiBytePosition + 1];

    rByte = (uiHigh << (8 - uiShift)) | (uiLow >> uiShift);
    ++m_uiBytePosition;
}

void BitStream::ExtractBitImpl(bool& rBit)
{
    if (!CanExtract(1))
    {
        m_bError = true;
        return;
    }

    unsigned int uiMask = (1 << m_uiBitPosition) & 0xFF;

    rBit = (uiMask & (*m_pBuffer)[m_uiBytePosition]) == uiMask;

    unsigned int uiBitPosition = m_uiBitPosition;

    if (uiBitPosition == 0)
    {
        uiBitPosition = 8;
        ++m_uiBytePosition;
    }

    m_uiBitPosition = uiBitPosition - 1;
}

void BitStream::ExtractRaw(unsigned char* pData, unsigned int uiBits)
{
    if (!CanExtract(uiBits))
    {
        m_bError = true;
        return;
    }

    unsigned int uiByte = 0;

    if (m_uiBitPosition != 7)
    {
        for (unsigned int uiDone = 8; uiDone <= uiBits; uiDone += 8)
        {
            ExtractByteImpl(pData[uiByte]);
            ++uiByte;
        }
    }
    else
    {
        uiByte = uiBits / 8;
        m_pBuffer->CopyContent(pData, uiByte, m_uiBytePosition);
        m_uiBytePosition += uiByte;
    }

    unsigned char uiRemaining = uiBits & 7;

    if (uiRemaining != 0)
    {
        unsigned char uiMask = 0x80;
        unsigned int uiLast = 0;

        do
        {
            bool bBit = false;

            ExtractBitImpl(bBit);

            if (bBit)
            {
                uiLast |= uiMask;
            }

            --uiRemaining;
            uiMask >>= 1;
        } while (uiRemaining != 0);

        pData[uiByte] = static_cast<unsigned char>(uiLast);
    }
}

void BitStream::Extract(unsigned char* pData, unsigned int uiSize, unsigned int uiCount)
{
    for (unsigned int uiIndex = 0; uiIndex < uiCount; ++uiIndex)
    {
        for (int iByte = uiSize; --iByte >= 0;)
        {
            ExtractRaw(pData + uiIndex * uiSize + iByte, 8);
        }
    }
}

BitStream& BitStream::operator<<(const Buffer& rBuffer)
{
    unsigned int uiLength = rBuffer.GetLength();

    Append(reinterpret_cast<const unsigned char*>(&uiLength), sizeof(uiLength), 1);

    if (rBuffer.GetLength() != 0)
    {
        AppendRaw(rBuffer.GetData(), rBuffer.GetLength() * 8);
    }

    return *this;
}

BitStream& BitStream::operator>>(Buffer& rBuffer)
{
    unsigned int uiLength;

    Extract(reinterpret_cast<unsigned char*>(&uiLength), sizeof(uiLength), 1);

    if (!CanExtract(uiLength * 8))
    {
        m_bError = true;
    }
    else if (!m_bError)
    {
        if (uiLength != 0)
        {
            rBuffer.ResizeByRealSize(uiLength + rBuffer.GetHeadShiftSize());
            ExtractRaw(rBuffer.GetData(), uiLength * 8);
        }

        rBuffer.SetLength(uiLength);
    }

    return *this;
}

BitStream& BitStream::operator>>(BitStream& rStream)
{
    *this >> *rStream.m_pBuffer;
    rStream.AdjustLength();
    return *this;
}

void BitStream::TraceBits(uint64_t uiFilter, unsigned char* pData, unsigned int uiBits)
{
    if (uiBits != 0)
    {
        String text(L"");

        text.Format(L"Stream of %d bits: ", uiBits);

        unsigned int uiByte = 0;
        unsigned int uiBit = 0;
        unsigned int uiMask = 0x80;

        while (uiBit < uiBits)
        {
            text += String((pData[uiByte] & uiMask) == uiMask ? L"1" : L"0");

            uiMask >>= 1;
            ++uiBit;

            if (uiMask == 0)
            {
                text += String(L" ");
                uiMask = 0x80;
                ++uiByte;
            }

            if (uiBit == 128)
            {
                text += String(L"...");
                uiBit = uiBits;
            }
        }
    }
}

}
}

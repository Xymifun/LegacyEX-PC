#include "nn/nex/Buffer.hpp"

#include <cstring>

#include "nn/nex/MemoryManager.hpp"
#include "nn/nex/PacketBufferManager.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/StringConversion.hpp"
#include "nn/nex/SystemError.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int APPEND_AT_END = 0xFFFFFFFFu;

}

unsigned int Buffer::s_uiDefaultBufferSize = 1364;

unsigned int Buffer::GetDefaultBufferSize()
{
    return s_uiDefaultBufferSize;
}

Buffer::Buffer(const Buffer& rBuffer)
{
    m_uiFlags = 0;
    m_uiRealSize = rBuffer.m_uiRealSize;
    m_pData = AllocateDataBuffer(m_uiRealSize);
    m_uiHeadShiftSize = 0;
    m_uiLength = rBuffer.m_uiLength;

    *this = rBuffer;

    m_uiFlags = static_cast<unsigned char>(rBuffer.m_uiFlags & ~BUFFER_FLAG_POOLED);
    m_uiTTL = rBuffer.m_uiTTL;
    m_uiFlags3 = rBuffer.m_uiFlags3;
}

Buffer::Buffer(const String& rString)
{
    m_uiHeadShiftSize = 0;

    const unsigned int uiLength = rString.GetLength();

    m_uiFlags = 0;

    if (uiLength == 0)
    {
        m_uiRealSize = 0;
        m_uiTTL = 0;
        m_uiFlags3 = 0;
        m_pData = 0;
        m_uiLength = 0;
        return;
    }

    m_uiLength = uiLength / 2;
    m_uiRealSize = uiLength / 2;
    m_pData = AllocateDataBuffer(m_uiRealSize);

    StringConversion::StringToHex(rString, m_pData, m_uiRealSize);

    m_uiFlags3 = 0;
    m_uiTTL = 0;
}

Buffer::~Buffer()
{
    FreeDataBuffer(m_pData, m_uiRealSize);
}

Buffer& Buffer::operator=(const Buffer& rBuffer)
{
    if (this != &rBuffer)
    {
        const unsigned int uiSize = rBuffer.m_uiLength + rBuffer.m_uiHeadShiftSize;

        m_uiLength = 0;
        m_uiHeadShiftSize = 0;

        if (m_uiRealSize < uiSize)
        {
            ResizeByRealSize(uiSize);
        }

        AppendData(rBuffer.m_pData, uiSize, APPEND_AT_END);
        SetHeadShiftSize(rBuffer.m_uiHeadShiftSize);

        m_uiFlags = static_cast<unsigned char>((rBuffer.m_uiFlags & ~BUFFER_FLAG_POOLED)
                                               | (m_uiFlags & BUFFER_FLAG_POOLED));
        m_uiTTL = rBuffer.m_uiTTL;
    }

    return *this;
}

Buffer& Buffer::operator+=(const Buffer& rBuffer)
{
    AppendData(rBuffer.GetData(), rBuffer.m_uiLength, APPEND_AT_END);
    return *this;
}

unsigned char& Buffer::operator[](unsigned int uiOffset)
{
    if (m_uiRealSize == 0)
    {
        ResizeByRealSize(5);
    }

    const unsigned int uiRequired = uiOffset + m_uiHeadShiftSize;

    if (m_uiRealSize <= uiRequired)
    {
        ResizeByRealSize((uiRequired * 15) / 10);
    }

    if (m_uiLength <= uiOffset)
    {
        m_uiLength = uiOffset + 1;
    }

    return *(m_pData + m_uiHeadShiftSize + uiOffset);
}

bool Buffer::operator==(const Buffer& rBuffer) const
{
    if (m_uiLength != rBuffer.m_uiLength)
    {
        return false;
    }

    return std::memcmp(rBuffer.GetData(), GetData(), m_uiLength) == 0;
}

void Buffer::Initialize(unsigned int uiRealSize, unsigned char uiFlags)
{
    m_uiFlags = uiFlags;
    m_uiHeadShiftSize = 0;
    m_uiRealSize = uiRealSize;
    m_uiLength = 0;
    m_pData = AllocateDataBuffer(uiRealSize);
    m_uiFlags3 = 0;
    m_uiTTL = 0;
}

bool Buffer::IsValid() const
{
    return m_pData != 0;
}

bool Buffer::AppendData(const void* pData, unsigned int uiSize, unsigned int uiOffset)
{
    if (uiSize != 0)
    {
        unsigned int uiLength = m_uiLength;

        if (uiOffset == APPEND_AT_END)
        {
            uiOffset = uiLength;
        }

        if (uiLength < uiOffset || uiLength < uiOffset + uiSize)
        {
            uiLength = uiOffset + uiSize;
        }

        ResizeByRealSize(GetAllocateSize(m_uiRealSize, uiLength + m_uiHeadShiftSize));
        m_uiLength = uiLength;
        std::memcpy(m_pData + m_uiHeadShiftSize + uiOffset, pData, uiSize);
    }

    return true;
}

bool Buffer::CopyContent(void* pDestination, unsigned int uiSize, unsigned int uiOffset) const
{
    if (m_uiLength < uiOffset + uiSize)
    {
        SystemError::SignalError(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/Buffer.cpp", 0xC6,
                                 0xE0000019, 0);
        return false;
    }

    std::memcpy(pDestination, GetData() + uiOffset, uiSize);
    return true;
}

void Buffer::ResizeByRealSize(unsigned int uiRealSize)
{
    if (uiRealSize != m_uiRealSize)
    {
        unsigned char* pData = AllocateDataBuffer(uiRealSize);

        if (pData == 0)
        {
            m_uiFlags = static_cast<unsigned char>(m_uiFlags & ~BUFFER_FLAG_POOLED);
            pData = AllocateDataBuffer(uiRealSize);
        }

        unsigned int uiCopySize = m_uiRealSize;

        if (uiRealSize < m_uiRealSize)
        {
            uiCopySize = uiRealSize;
        }

        std::memcpy(pData, m_pData, uiCopySize);
        FreeDataBuffer(m_pData, m_uiRealSize);
        m_pData = pData;
        m_uiRealSize = uiRealSize;
    }
}

bool Buffer::AttemptExpand(unsigned int uiSize)
{
    const unsigned int uiRealSize = GetAllocateSize(m_uiRealSize, m_uiLength + m_uiHeadShiftSize + uiSize);

    if (uiRealSize <= m_uiRealSize)
    {
        return true;
    }

    unsigned char* pData = AllocateDataBuffer(uiRealSize);

    if (pData == 0)
    {
        return false;
    }

    std::memcpy(pData, m_pData, m_uiRealSize);
    FreeDataBuffer(m_pData, m_uiRealSize);
    m_pData = pData;
    m_uiRealSize = uiRealSize;
    return true;
}

void Buffer::SetHeadShiftSize(unsigned int uiHeadShiftSize)
{
    const unsigned int uiPrevious = m_uiHeadShiftSize;

    m_uiHeadShiftSize = uiHeadShiftSize;
    m_uiLength += uiPrevious - uiHeadShiftSize;
}

void Buffer::Swap(Buffer& rBuffer)
{
    const unsigned char uiFlags = m_uiFlags;

    m_uiFlags = rBuffer.m_uiFlags;
    rBuffer.m_uiFlags = uiFlags;

    unsigned char* pData = m_pData;

    m_pData = rBuffer.m_pData;
    rBuffer.m_pData = pData;
}

unsigned char Buffer::ComputeCheckSum(unsigned int uiSize, unsigned char uiSeed)
{
    const unsigned char* pData = GetData();
    unsigned int uiSum = 0;
    unsigned int i;

    for (i = 0; i < uiSize / 4; ++i)
    {
        uiSum += static_cast<unsigned int>(pData[i * 4]) | (static_cast<unsigned int>(pData[i * 4 + 1]) << 8)
                 | (static_cast<unsigned int>(pData[i * 4 + 2]) << 16)
                 | (static_cast<unsigned int>(pData[i * 4 + 3]) << 24);
    }

    for (i = 0; i < (uiSize & 3); ++i)
    {
        uiSeed = static_cast<unsigned char>(uiSeed + pData[(uiSize & ~3u) + i]);
    }

    const unsigned char* pSum = reinterpret_cast<const unsigned char*>(&uiSum);

    for (i = 0; i < sizeof(uiSum); ++i)
    {
        uiSeed = static_cast<unsigned char>(uiSeed + pSum[i]);
    }

    return uiSeed;
}

unsigned char Buffer::GetCheckSum()
{
    if (m_uiLength == 0)
    {
        return 0;
    }

    unsigned char uiCheckSum = 0;

    CopyContent(&uiCheckSum, 1, m_uiLength - 1);
    return uiCheckSum;
}

void Buffer::AppendCheckSum(unsigned char uiSeed)
{
    const unsigned char uiCheckSum = ComputeCheckSum(m_uiLength, uiSeed);

    AppendData(&uiCheckSum, 1, APPEND_AT_END);
}

void Buffer::StripCheckSum()
{
    m_uiLength -= 1;
}

bool Buffer::IsCheckSumValid(unsigned char uiSeed)
{
    if (m_uiLength == 0)
    {
        return false;
    }

    return GetCheckSum() == ComputeCheckSum(m_uiLength - 1, uiSeed);
}

String Buffer::ToString() const
{
    String result;

    result.Reserve(static_cast<int>(m_uiLength) * 2 + 1);
    StringConversion::HexToString(GetData(), result, m_uiLength);
    return result;
}

void Buffer::Trace(uint64_t) const
{
    for (unsigned int uiIndex = 0; uiIndex < m_uiLength; ++uiIndex)
    {
    }
}

unsigned int Buffer::GetAllocateSize(unsigned int uiCurrentSize, unsigned int uiRequiredSize)
{
    if (uiRequiredSize <= uiCurrentSize)
    {
        return uiCurrentSize;
    }

    if (uiCurrentSize == 0)
    {
        uiCurrentSize = 1;
    }

    unsigned int uiUnitSize = 0;

    if (PacketBufferManager::GetInstance() != 0)
    {
        uiUnitSize = PacketBufferManager::GetInstance()->GetProperUnitSize(uiRequiredSize);
    }

    if (uiUnitSize < uiRequiredSize)
    {
        uiUnitSize = uiCurrentSize;

        while (uiUnitSize < uiRequiredSize)
        {
            uiUnitSize *= 2;
        }
    }

    return uiUnitSize;
}

unsigned char* Buffer::AllocateDataBuffer(unsigned int uiSize)
{
    if (PacketBufferManager::GetInstance() == 0 || uiSize > PacketBufferManager::GetInstance()->GetMaxUnitSize())
    {
        m_uiFlags = static_cast<unsigned char>(m_uiFlags & ~BUFFER_FLAG_POOLED);
    }

    if ((m_uiFlags & BUFFER_FLAG_POOLED) == 0)
    {
        return static_cast<unsigned char*>(MemoryManager::Allocate(uiSize));
    }

    return static_cast<unsigned char*>(PacketBufferManager::GetInstance()->Allocate(uiSize));
}

void Buffer::FreeDataBuffer(unsigned char* pData, unsigned int)
{
    MemoryManager::Free(pData);
}

}
}

#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_BUFFER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_BUFFER_HPP

#include <stdint.h>

#include "nn/nex/RefCountedObject.hpp"

namespace nn
{
namespace nex
{

class String;

class Buffer : public RefCountedObject
{
public:
    enum
    {
        BUFFER_FLAG_POOLED = 0x04
    };

    Buffer(const Buffer& rBuffer);
    explicit Buffer(const String& rString);
    virtual ~Buffer();

    Buffer() { Initialize(GetDefaultBufferSize(), 0); }

    Buffer(unsigned int uiRealSize, unsigned char uiFlags) { Initialize(uiRealSize, uiFlags); }

    Buffer& operator=(const Buffer& rBuffer);
    Buffer operator+(const Buffer& rBuffer);
    Buffer& operator+=(const Buffer& rBuffer);
    unsigned char& operator[](unsigned int uiOffset);
    bool operator==(const Buffer& rBuffer) const;

    void Initialize(unsigned int uiRealSize, unsigned char uiFlags);
    bool IsValid() const;

    bool AppendData(const void* pData, unsigned int uiSize, unsigned int uiOffset);
    bool CopyContent(void* pDestination, unsigned int uiSize, unsigned int uiOffset) const;
    void ResizeByRealSize(unsigned int uiRealSize);
    bool AttemptExpand(unsigned int uiSize);
    void SetHeadShiftSize(unsigned int uiHeadShiftSize);
    void Swap(Buffer& rBuffer);

    unsigned char ComputeCheckSum(unsigned int uiSize, unsigned char uiSeed);
    unsigned char GetCheckSum();
    void AppendCheckSum(unsigned char uiSeed);
    void StripCheckSum();
    bool IsCheckSumValid(unsigned char uiSeed);

    String ToString() const;
    void Trace(uint64_t uiFilter) const;

    static unsigned int s_uiDefaultBufferSize;

    static unsigned int GetDefaultBufferSize();
    static void SetDefaultBufferSize(unsigned int uiSize);
    static unsigned int GetAllocateSize(unsigned int uiCurrentSize, unsigned int uiRequiredSize);

    unsigned char* GetData() const { return m_pData + m_uiHeadShiftSize; }

    bool IsPooled() const { return (m_uiFlags & BUFFER_FLAG_POOLED) != 0; }

    unsigned char GetTTL() const { return m_uiTTL; }

    void SetTTL(unsigned char uiTTL) { m_uiTTL = uiTTL; }

    unsigned int GetLength() const { return m_uiLength; }

    void SetLength(unsigned int uiLength) { m_uiLength = uiLength; }

    unsigned int GetRealSize() const { return m_uiRealSize; }

    unsigned int GetHeadShiftSize() const { return m_uiHeadShiftSize; }

    void Clear()
    {
        m_uiLength = 0;
        m_uiHeadShiftSize = 0;
    }

private:
    unsigned char* AllocateDataBuffer(unsigned int uiSize);
    void FreeDataBuffer(unsigned char* pData, unsigned int uiSize);

    unsigned char* m_pData;
    unsigned int m_uiLength;
    unsigned int m_uiRealSize;
    unsigned int m_uiHeadShiftSize;
    unsigned char m_uiFlags;
    unsigned char m_uiTTL;
    unsigned char m_uiFlags3;
};

}
}

#endif

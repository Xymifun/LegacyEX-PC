#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STRINGSTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STRINGSTREAM_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class String;

class StringStream : public RootObject
{
public:
    enum
    {
        INLINE_BUFFER_LENGTH = 256
    };

    StringStream();
    virtual ~StringStream();

    int GetLength() const;
    void Clear();

    StringStream& operator<<(const wchar_t* pString);
    StringStream& operator<<(const void* pPointer);
    StringStream& operator<<(bool bValue);
    StringStream& operator<<(unsigned long long uiValue);
    StringStream& operator<<(long long iValue);
    StringStream& operator<<(float fValue);
    StringStream& operator<<(double dValue);
    StringStream& operator<<(const StringStream& rStream);

    StringStream& StreamNumber(int iValue);
    StringStream& StreamNumber(unsigned int uiValue);
    StringStream& StreamNumber(unsigned char uiValue);

    void BytesDump(const unsigned char* pData, unsigned int uiSize);

    const wchar_t* GetBuffer() const { return m_pBuffer; }

private:
    friend void zerohex(StringStream& rStream);
    StringStream& hex(StringStream& rStream);
    friend StringStream& hex(StringStream& rStream);
    friend StringStream& noshowbase(StringStream& rStream);

    void FreeBuffer(wchar_t* pBuffer);
    void FreeBuffer();
    void ResizeBuffer(unsigned int uiCapacity);
    void TestFreeRoom(unsigned int uiCount);
    void AddBaseIfRequired();

    wchar_t* m_pBuffer;
    unsigned int m_uiCapacity;
    wchar_t* m_pCursor;
    wchar_t m_aInlineBuffer[INLINE_BUFFER_LENGTH];
    bool m_bHex;
    bool m_bAddBase;
    bool m_bTextualBool;
    bool m_bZeroPadded;
};

void zerohex(StringStream& rStream);

StringStream& operator<<(StringStream& rStream, const String& rString);

}
}

#endif

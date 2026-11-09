#include "nn/crypto/ShaBlock512BitContext.hpp"

namespace nn
{
namespace crypto
{

namespace
{

unsigned char s_padding[64] = {0x80};

}

void ShaBlock512BitContext::Update(const void* pData, unsigned int uiSize)
{
    const unsigned char* pSource = static_cast<const unsigned char*>(pData);
    while (uiSize != 0)
    {
        unsigned int uiCopySize = BlockSize - m_blockSize;
        if (uiCopySize > uiSize)
        {
            uiCopySize = uiSize;
        }
        unsigned char* pDest = &m_block.bytes[m_blockSize];
        for (int i = uiCopySize; i > 0; --i)
        {
            *pDest++ = *pSource++;
        }
        m_blockSize += uiCopySize;
        uiSize -= uiCopySize;
        if (m_blockSize >= BlockSize)
        {
            ProcessBlock();
            m_blockSize = 0;
            ++m_blockCount;
            if (m_blockCount == 0)
            {
                ++m_blockCountHigh;
            }
        }
    }
}

void ShaBlock512BitContext::AddPadding()
{
    unsigned int length[2];
    length[0] = (m_blockCountHigh << 9) | (m_blockCount >> 23);
    length[1] = m_blockCount * 512 + m_blockSize * 8;
    unsigned int uiPaddingEnd = 120;
    if (m_blockSize < 56)
    {
        uiPaddingEnd = 56;
    }
    Update(s_padding, uiPaddingEnd - m_blockSize);
    Update(length, sizeof(length));
}

}
}

#include "nn/pia/common/ShaBlock512BitContext.hpp"

#include <coreinit/memory.h>

namespace nn
{
namespace pia
{
namespace common
{

namespace
{
unsigned char s_padding[64] = {0x80};
}

unsigned int ShaBlock512BitContext::GetBlockSize() const
{
    return sizeof(m_block);
}

void ShaBlock512BitContext::Update(const void* data, unsigned int size)
{
    const unsigned char* bytes = static_cast<const unsigned char*>(data);
    while (size != 0)
    {
        unsigned int copySize = sizeof(m_block) - m_blockSize;
        if (copySize > size)
            copySize = size;
        OSBlockMove(m_block + m_blockSize, bytes, copySize, FALSE);
        bytes += copySize;
        size -= copySize;
        m_blockSize += copySize;
        if (m_blockSize >= sizeof(m_block))
        {
            ProcessBlock();
            m_blockSize = 0;
            ++m_blockCount;
            if (m_blockCount == 0)
                ++m_blockCountHigh;
        }
    }
}

void ShaBlock512BitContext::AddPadding()
{
    unsigned int length[2];
    length[0] = (m_blockCountHigh << 9) | (m_blockCount >> 23);
    length[1] = m_blockCount * 512 + m_blockSize * 8;
    unsigned int paddingEnd = 120;
    if (m_blockSize < 56)
        paddingEnd = 56;
    Update(s_padding, paddingEnd - m_blockSize);
    Update(length, sizeof(length));
}

void ShaBlock512BitContext::ProcessBlock()
{
}

}
}
}

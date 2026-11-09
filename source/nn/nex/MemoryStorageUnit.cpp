#include "nn/nex/MemoryStorageUnit.hpp"

#include <algorithm>

namespace nn
{
namespace nex
{

unsigned int MemoryStorageUnit::GetSize() const
{
    return m_data.size();
}

void MemoryStorageUnit::Truncate()
{
    m_data.clear();
}

unsigned int MemoryStorageUnit::Read(unsigned int uiOffset, unsigned int uiSize, unsigned char* pBuffer) const
{
    if (m_data.size() < uiOffset)
    {
        return 0;
    }
    unsigned int uiAvailable = m_data.size() - uiOffset;
    unsigned int uiCount = uiSize < uiAvailable ? uiSize : uiAvailable;
    std::copy(m_data.begin() + uiOffset, m_data.begin() + uiOffset + uiCount, pBuffer);
    return uiCount;
}

unsigned int MemoryStorageUnit::Write(unsigned int uiOffset, unsigned int uiSize, unsigned char* pBuffer)
{
    if (m_data.size() < uiOffset + uiSize)
    {
        m_data.resize(uiOffset + uiSize, 0);
    }
    std::copy(pBuffer, pBuffer + uiSize, m_data.begin() + uiOffset);
    return uiSize;
}

}
}

#include "nn/nex/HTTPStringData.hpp"

namespace nn
{
namespace nex
{

HTTPStringData::~HTTPStringData()
{
}

unsigned int HTTPStringData::Read(void* pBuffer, unsigned int uiSize)
{
    unsigned int uiCount = m_data.GetLength() - m_position;
    if (uiCount > uiSize)
    {
        uiCount = uiSize;
    }
    const wchar_t* pData = m_data;
    for (unsigned int i = 0; i < uiCount; ++i)
    {
        static_cast<char*>(pBuffer)[i] = static_cast<char>(pData[m_position + i]);
    }
    m_position += uiCount;
    return uiCount;
}

unsigned int HTTPStringData::GetContentLength() const
{
    return m_data.GetLength();
}

}
}

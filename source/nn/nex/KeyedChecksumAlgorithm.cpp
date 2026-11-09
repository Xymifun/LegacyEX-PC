#include "nn/nex/KeyedChecksumAlgorithm.hpp"

namespace nn
{
namespace nex
{

KeyedChecksumAlgorithm::KeyedChecksumAlgorithm(unsigned int uiMinKeyLength, unsigned int uiMaxKeyLength)
    : m_uiMinKeyLength(uiMinKeyLength),
      m_uiMaxKeyLength(uiMaxKeyLength)
{
}

KeyedChecksumAlgorithm::~KeyedChecksumAlgorithm()
{
}

bool KeyedChecksumAlgorithm::IsReady() const
{
    bool bReady = false;
    unsigned int uiLength = m_key.GetLength();
    if (uiLength >= m_uiMinKeyLength && uiLength <= m_uiMaxKeyLength)
    {
        bReady = true;
    }
    return bReady;
}

bool KeyedChecksumAlgorithm::SetKey(const Key& rKey)
{
    unsigned int uiLength = rKey.GetLength();
    if (uiLength >= m_uiMinKeyLength && uiLength <= m_uiMaxKeyLength)
    {
        m_key = rKey;
        KeyHasChanged();
        return true;
    }

    return false;
}

}
}

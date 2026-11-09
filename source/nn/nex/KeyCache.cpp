#include "nn/nex/KeyCache.hpp"

#include <string.h>

namespace nn
{
namespace nex
{

bool KeyCache::RetrieveKey(unsigned int uiPrincipalID, const Buffer& rBuffer, Key* pKey)
{
    ScopedCS lock(&m_criticalSection);
    qMap<unsigned int, Buffer>::iterator itBuffer = m_buffers.find(uiPrincipalID);
    if (itBuffer != m_buffers.end() && itBuffer->second == rBuffer)
    {
        qMap<unsigned int, Key>::iterator itKey = m_keys.find(uiPrincipalID);
        *pKey = itKey->second;
        return true;
    }
    return false;
}

void KeyCache::AddKey(unsigned int uiPrincipalID, const Buffer& rBuffer, const Key& rKey)
{
    ScopedCS lock(&m_criticalSection);
    m_buffers[uiPrincipalID] = rBuffer;
    m_keys[uiPrincipalID] = rKey;
}

bool KeyCache::RetrieveKey(unsigned int uiPrincipalID, const char* pPassword, Key* pKey)
{
    Buffer buffer;
    buffer.AppendData(pPassword, strlen(pPassword), 0xFFFFFFFF);
    return RetrieveKey(uiPrincipalID, buffer, pKey);
}

void KeyCache::AddKey(unsigned int uiPrincipalID, const char* pPassword, const Key& rKey)
{
    Buffer buffer;
    buffer.AppendData(pPassword, strlen(pPassword), 0xFFFFFFFF);
    AddKey(uiPrincipalID, buffer, rKey);
}

}
}

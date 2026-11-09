#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_KEYCACHE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_KEYCACHE_HPP

#include "nn/nex/Buffer.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/qMap.hpp"

namespace nn
{
namespace nex
{

class KeyCache : public RootObject
{
public:
    bool RetrieveKey(unsigned int uiPrincipalID, const Buffer& rBuffer, Key* pKey);
    void AddKey(unsigned int uiPrincipalID, const Buffer& rBuffer, const Key& rKey);
    bool RetrieveKey(unsigned int uiPrincipalID, const char* pPassword, Key* pKey);
    void AddKey(unsigned int uiPrincipalID, const char* pPassword, const Key& rKey);

private:
    qMap<unsigned int, Buffer> m_buffers;
    qMap<unsigned int, Key> m_keys;
    CriticalSection m_criticalSection;
};

}
}

#endif

#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_KEYDERIVATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_KEYDERIVATION_HPP

#include "nn/nex/Key.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class CallContext;
class KeyCache;

class KeyDerivation : public RootObject
{
public:
    virtual ~KeyDerivation() {}

    virtual Key CreateKey(unsigned int uiPrincipalID, const char* pPassword) = 0;
    virtual bool CreateKey(CallContext* pCallContext, unsigned int uiPrincipalID, const char* pPassword, Key* pKey,
                           KeyCache* pKeyCache) = 0;
    virtual Key InitializeKey(const char* pPassword) = 0;
    virtual bool InitializeKey(CallContext* pCallContext, const char* pPassword, Key* pKey) = 0;
    virtual Key FinalizeKey(unsigned int uiPrincipalID, const Key& rKey) = 0;
    virtual bool FinalizeKey(CallContext* pCallContext, unsigned int uiPrincipalID, const Key& rKey,
                             Key* pFinalKey) = 0;
};

}
}

#endif

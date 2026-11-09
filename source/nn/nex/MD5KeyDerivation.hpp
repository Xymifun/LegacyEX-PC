#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MD5KEYDERIVATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MD5KEYDERIVATION_HPP

#include "nn/nex/KeyDerivation.hpp"

namespace nn
{
namespace nex
{

class MD5KeyDerivation : public KeyDerivation
{
public:
    MD5KeyDerivation();
    MD5KeyDerivation(unsigned int uiClientSideIterations, unsigned int uiPrincipalIdModulo);
    virtual ~MD5KeyDerivation();

    virtual Key CreateKey(unsigned int uiPrincipalID, const char* pPassword);
    virtual bool CreateKey(CallContext* pCallContext, unsigned int uiPrincipalID, const char* pPassword, Key* pKey,
                           KeyCache* pKeyCache);
    virtual Key InitializeKey(const char* pPassword);
    virtual bool InitializeKey(CallContext* pCallContext, const char* pPassword, Key* pKey);
    virtual Key FinalizeKey(unsigned int uiPrincipalID, const Key& rKey);
    virtual bool FinalizeKey(CallContext* pCallContext, unsigned int uiPrincipalID, const Key& rKey, Key* pFinalKey);

    void SetClientSideIterations(unsigned int uiClientSideIterations);
    void SetPrincipalIdModulo(unsigned int uiPrincipalIdModulo);

private:
    unsigned int m_uiClientSideIterations;
    unsigned int m_uiPrincipalIdModulo;
};

}
}

#endif

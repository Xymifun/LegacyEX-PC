#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ENCRYPTIONALGORITHM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ENCRYPTIONALGORITHM_HPP

#include "nn/nex/Buffer.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/PluginObject.hpp"

namespace nn
{
namespace nex
{

class EncryptionAlgorithm : public PluginObject
{
public:
    EncryptionAlgorithm(unsigned int uiMinKeyLength, unsigned int uiMaxKeyLength);
    virtual ~EncryptionAlgorithm();

    virtual int Encrypt(const Buffer& rSource, Buffer* pDestination) = 0;
    virtual int Encrypt(Buffer* pBuffer);
    virtual int Decrypt(const Buffer& rSource, Buffer* pDestination) = 0;
    virtual int Decrypt(Buffer* pBuffer);
    virtual bool GetErrorString(unsigned int uiError, wchar_t* pString, unsigned int uiLength);
    virtual void KeyHasChanged() {}

    bool SetKey(const Key& rKey);

    const Key& GetKey() const { return m_key; }

protected:
    Key m_key;
    unsigned int m_uiMinKeyLength;
    unsigned int m_uiMaxKeyLength;
};

}
}

#endif

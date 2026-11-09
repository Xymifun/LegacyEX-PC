#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RC4ENCRYPTION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RC4ENCRYPTION_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/EncryptionAlgorithm.hpp"

namespace nn
{
namespace nex
{

class RC4Encryption : public EncryptionAlgorithm
{
public:
    enum
    {
        StateSize = 256
    };

    RC4Encryption();
    virtual ~RC4Encryption();

    virtual int Encrypt(const Buffer& rSource, Buffer* pDestination);
    virtual int Encrypt(Buffer* pBuffer);
    virtual int Decrypt(const Buffer& rSource, Buffer* pDestination);
    virtual int Decrypt(Buffer* pBuffer);
    virtual void KeyHasChanged();

    static Key GetDefaultKey();

    void Encrypt(unsigned char* pData, unsigned int uiLength);
    void Decrypt(unsigned char* pData, unsigned int uiLength);
    void SetReinitEverytime(bool bReinit);

private:
    void PrepareEncryption();
    void ReinitStateArray();
    void EncryptDecrypt(unsigned char* pData, unsigned int uiLength);

    unsigned char m_initialState[StateSize];
    unsigned char m_state[StateSize];
    unsigned char m_uiIndexI;
    unsigned char m_uiIndexJ;
    bool m_bReinitEverytime;
    CriticalSection* m_pCriticalSection;
};

}
}

#endif

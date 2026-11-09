#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PACKETENCDEC_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PACKETENCDEC_HPP

#include "nn/nex/EncryptionAlgorithm.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/Stream.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Packet;

class PacketEncDec : public RootObject
{
public:
    PacketEncDec();
    ~PacketEncDec();

    void Decode(Packet* pPacket);
    void Encode(Packet* pPacket, Stream::Type streamType, bool bSwapPayload);

    void SetEncryptionKey(const Key& rKey);
    void SetUnreliableEncryptionKey(const Key& rKey);
    void SetReliableEncryptionKey(const Key& rKey, unsigned char uiEncryptSalt, unsigned char uiDecryptSalt);
    Key& GetEncryptionKey();

    void DecideMaxUserReliableSubStreams(unsigned short uiMaxSubStreams);

private:
    typedef qVector<EncryptionAlgorithm*> EncryptorVector;

    void GenerateKey(const Key& rKey, const unsigned char* pSaltA, const unsigned char* pSaltB, Key* pOutput);
    void ReinitUserReliableSubStreams();

    EncryptorVector m_encryptors;
    EncryptorVector m_decryptors;
    EncryptionAlgorithm* m_pUnreliableEncryptor;
    Key m_encryptionKey;
    Key m_unreliableKey;
    Key m_unreliableAlternateKey;
    bool m_bReliableKeySet;
    unsigned short m_uiMaxUserReliableSubStreams;
};

}
}

#endif

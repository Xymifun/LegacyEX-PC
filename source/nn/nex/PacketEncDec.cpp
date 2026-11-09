#include "nn/nex/PacketEncDec.hpp"

#include <string.h>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/MD5.hpp"
#include "nn/nex/Packet.hpp"
#include "nn/nex/RC4Encryption.hpp"
#include "nn/nex/StreamSettings.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned char s_unreliableSaltA[8] = {0x18, 0xD8, 0x23, 0x34, 0x37, 0xE4, 0xE3, 0xFE};
const unsigned char s_unreliableSaltB[8] = {0x23, 0x3E, 0x60, 0x01, 0x23, 0xCD, 0xAB, 0x80};
const unsigned char s_alternateSaltA[8] = {0x98, 0xA8, 0x93, 0x84, 0x87, 0x94, 0xE3, 0x6E};
const unsigned char s_alternateSaltB[8] = {0x45, 0xEE, 0xFC, 0xAB, 0xE0, 0x54, 0x02, 0x01};

}

PacketEncDec::PacketEncDec()
    : m_encryptors(1),
      m_decryptors(1),
      m_unreliableKey(32),
      m_unreliableAlternateKey(32),
      m_bReliableKeySet(false),
      m_uiMaxUserReliableSubStreams(1)
{
    RC4Encryption* pReliableEncryptor = new RC4Encryption();
    pReliableEncryptor->SetReinitEverytime(false);
    m_encryptors[0] = pReliableEncryptor;
    RC4Encryption* pReliableDecryptor = new RC4Encryption();
    pReliableDecryptor->SetReinitEverytime(false);
    m_decryptors[0] = pReliableDecryptor;
    RC4Encryption* pUnreliableEncryptor = new RC4Encryption();
    pUnreliableEncryptor->SetReinitEverytime(false);
    m_pUnreliableEncryptor = pUnreliableEncryptor;
}

PacketEncDec::~PacketEncDec()
{
    for (int i = 0; i < m_encryptors.size(); ++i)
    {
        delete m_encryptors[i];
        delete m_decryptors[i];
    }

    delete m_pUnreliableEncryptor;
}

void PacketEncDec::Decode(Packet* pPacket)
{
    Stream::Type streamType = pPacket->GetStreamType();
    if (g_streamSettings[streamType].Get().IsEncryptionRequired() && !pPacket->m_bSkipEncryption
        && pPacket->m_pPayload != 0)
    {
        if ((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_DATA
            || (pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_USER)
        {
            if ((pPacket->m_uiTypeAndFlags & PACKET_FLAG_RELIABLE) != 0)
            {
                m_decryptors[pPacket->m_uiSubStreamID]->Decrypt(pPacket->m_pPayload);
                return;
            }

            Key packetKey(32);
            if ((pPacket->m_uiTypeAndFlags & PACKET_FLAG_ALTERNATE_KEY_A) != 0
                || (pPacket->m_uiTypeAndFlags & PACKET_FLAG_ALTERNATE_KEY_B) != 0)
                memcpy(packetKey.GetContentPtr(), m_unreliableAlternateKey.GetContentPtr(), 32);
            else
                memcpy(packetKey.GetContentPtr(), m_unreliableKey.GetContentPtr(), 32);
            unsigned short uiSequenceID = pPacket->m_sequenceID.GetValue();
            unsigned char uiSequenceLow = static_cast<unsigned char>(uiSequenceID);
            unsigned char uiSequenceHigh = static_cast<unsigned char>(uiSequenceID >> 8);
            packetKey.GetContentPtr()[0] += uiSequenceLow;
            packetKey.GetContentPtr()[1] += uiSequenceHigh;
            packetKey.GetContentPtr()[packetKey.GetLength() - 1] += pPacket->m_uiSessionID;
            m_pUnreliableEncryptor->SetKey(packetKey);
            m_pUnreliableEncryptor->Decrypt(pPacket->m_pPayload);
        }
    }
}

void PacketEncDec::Encode(Packet* pPacket, Stream::Type streamType, bool bSwapPayload)
{
    if (g_streamSettings[streamType].Get().IsEncryptionRequired() && !pPacket->m_bSkipEncryption
        && pPacket->m_pPayload != 0
        && ((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_DATA
            || (pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_USER))
    {
        Buffer* pPayload = pPacket->m_pPayload;
        if (bSwapPayload)
        {
            pPayload = new Buffer(*pPayload);
            pPayload->Swap(*pPacket->m_pPayload);
        }

        if ((pPacket->m_uiTypeAndFlags & PACKET_FLAG_RELIABLE) != 0)
        {
            m_encryptors[pPacket->m_uiSubStreamID]->Encrypt(pPayload);
        }
        else
        {
            Key packetKey(32);
            if ((pPacket->m_uiTypeAndFlags & PACKET_FLAG_ALTERNATE_KEY_A) != 0
                || (pPacket->m_uiTypeAndFlags & PACKET_FLAG_ALTERNATE_KEY_B) != 0)
                memcpy(packetKey.GetContentPtr(), m_unreliableAlternateKey.GetContentPtr(), 32);
            else
                memcpy(packetKey.GetContentPtr(), m_unreliableKey.GetContentPtr(), 32);
            unsigned short uiSequenceID = pPacket->m_sequenceID.GetValue();
            unsigned char uiSequenceLow = static_cast<unsigned char>(uiSequenceID);
            unsigned char uiSequenceHigh = static_cast<unsigned char>(uiSequenceID >> 8);
            packetKey.GetContentPtr()[0] += uiSequenceLow;
            packetKey.GetContentPtr()[1] += uiSequenceHigh;
            packetKey.GetContentPtr()[packetKey.GetLength() - 1] += pPacket->m_uiSessionID;
            m_pUnreliableEncryptor->SetKey(packetKey);
            m_pUnreliableEncryptor->Encrypt(pPayload);
        }

        if (bSwapPayload)
            pPacket->SetPayload(pPayload);
    }
}

void PacketEncDec::GenerateKey(const Key& rKey, const unsigned char* pSaltA, const unsigned char* pSaltB, Key* pOutput)
{
    MD5 digestA;
    MD5 digestB;
    digestA.update(rKey.GetContentPtr(), rKey.GetLength());
    digestB.update(rKey.GetContentPtr(), rKey.GetLength());
    digestA.update(pSaltA, 8);
    digestB.update(pSaltB, 8);
    digestA.finalize();
    digestB.finalize();
    memcpy(pOutput->GetContentPtr(), digestA.GetDigest(), MD5::DigestSize);
    memcpy(pOutput->GetContentPtr() + MD5::DigestSize, digestB.GetDigest(), MD5::DigestSize);
}

void PacketEncDec::SetUnreliableEncryptionKey(const Key& rKey)
{
    m_encryptionKey = rKey;
    GenerateKey(m_encryptionKey, s_unreliableSaltA, s_unreliableSaltB, &m_unreliableKey);
    GenerateKey(m_encryptionKey, s_alternateSaltA, s_alternateSaltB, &m_unreliableAlternateKey);
}

void PacketEncDec::ReinitUserReliableSubStreams()
{
    if (m_encryptors.size() > 1)
    {
        Key encryptKey(m_encryptors[0]->GetKey());
        Key decryptKey(m_decryptors[0]->GetKey());
        for (unsigned int i = 1; i < m_encryptors.size(); ++i)
        {
            for (unsigned int j = 0; j < encryptKey.GetLength() / 2; ++j)
            {
                encryptKey.GetContentPtr()[j] += encryptKey.GetLength() / 2 + 1 - j;
                decryptKey.GetContentPtr()[j] += encryptKey.GetLength() / 2 + 1 - j;
            }

            m_encryptors[i]->SetKey(encryptKey);
            m_decryptors[i]->SetKey(decryptKey);
        }
    }
}

void PacketEncDec::SetEncryptionKey(const Key& rKey)
{
    m_encryptionKey = rKey;
    m_encryptors[0]->SetKey(rKey);
    m_decryptors[0]->SetKey(rKey);
    ReinitUserReliableSubStreams();
}

void PacketEncDec::DecideMaxUserReliableSubStreams(unsigned short uiMaxSubStreams)
{
    m_uiMaxUserReliableSubStreams = uiMaxSubStreams;
    if (m_bReliableKeySet)
    {
        while (m_encryptors.size() > uiMaxSubStreams + 1U)
        {
            m_encryptors.pop_back();
            m_decryptors.pop_back();
        }

        int iCount = uiMaxSubStreams + 1;
        m_encryptors.reserve(iCount);
        m_decryptors.reserve(iCount);
        for (int i = m_encryptors.size(); i < iCount; ++i)
        {
            RC4Encryption* pEncryptor = new RC4Encryption();
            pEncryptor->SetReinitEverytime(false);
            m_encryptors.push_back(pEncryptor);
            RC4Encryption* pDecryptor = new RC4Encryption();
            pDecryptor->SetReinitEverytime(false);
            m_decryptors.push_back(pDecryptor);
        }

        ReinitUserReliableSubStreams();
    }
}

void PacketEncDec::SetReliableEncryptionKey(const Key& rKey, unsigned char uiEncryptSalt, unsigned char uiDecryptSalt)
{
    m_encryptionKey = rKey;
    Key encryptKey(rKey);
    encryptKey.GetContentPtr()[rKey.GetLength() / 2] += uiEncryptSalt;
    Key decryptKey(rKey);
    decryptKey.GetContentPtr()[rKey.GetLength() / 2] += uiDecryptSalt;
    m_encryptors[0]->SetKey(encryptKey);
    m_decryptors[0]->SetKey(decryptKey);
    m_bReliableKeySet = true;
    if (m_uiMaxUserReliableSubStreams > 1)
        DecideMaxUserReliableSubStreams(m_uiMaxUserReliableSubStreams);
}

Key& PacketEncDec::GetEncryptionKey()
{
    return m_encryptionKey;
}

}
}

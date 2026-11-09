#include "nn/nex/Ticket.hpp"

#include "nn/nex/BitStream.hpp"
#include "nn/nex/KerberosEncryption.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int TICKET_KEY_LENGTH = 0x20;
const unsigned int TICKET_KEY_BITS = 0x100;

}

Ticket::Ticket(const Buffer& rBuffer, unsigned int uiTargetPrincipalID)
    : m_buffer(rBuffer),
      m_uiTargetPrincipalID(uiTargetPrincipalID),
      m_key(),
      m_time()
{
    m_bValid = false;
}

Ticket::Ticket(const Buffer& rBuffer, unsigned int uiSourcePrincipalID, unsigned int uiTargetPrincipalID,
               const Key& rKey, DateTime oTime)
    : m_buffer(rBuffer),
      m_uiSourcePrincipalID(uiSourcePrincipalID),
      m_uiTargetPrincipalID(uiTargetPrincipalID),
      m_key(rKey),
      m_time(oTime)
{
    m_bValid = true;
}

Ticket::~Ticket()
{
}

void Ticket::Encrypt(KerberosEncryption* pKerberosEncryption, const Key& rKey)
{
    BitStream stream;
    const unsigned long long uiTime = m_time;
    stream.Append(reinterpret_cast<const unsigned char*>(&uiTime), sizeof(uiTime), 1);
    stream.Append(reinterpret_cast<const unsigned char*>(&m_uiSourcePrincipalID), sizeof(m_uiSourcePrincipalID), 1);
    m_key.GetLength();
    stream.AppendRaw(m_key.GetContentPtr(), TICKET_KEY_BITS);
    pKerberosEncryption->Encrypt(*stream.m_pBuffer, &m_buffer, rKey);
}

void Ticket::Decrypt(KerberosEncryption* pKerberosEncryption, const Key& rKey)
{
    Key concatenatedKey = pKerberosEncryption->GenerateConcatenateKey(rKey);
    BitStream stream;
    if (pKerberosEncryption->Decrypt(m_buffer, stream.m_pBuffer, concatenatedKey))
    {
        m_bValid = true;
        stream.AdjustLength();
        unsigned long long uiTime;
        stream.Extract(reinterpret_cast<unsigned char*>(&uiTime), sizeof(uiTime), 1);
        m_time = DateTime(uiTime);
        stream.Extract(reinterpret_cast<unsigned char*>(&m_uiSourcePrincipalID), sizeof(m_uiSourcePrincipalID), 1);
        stream.ExtractRaw(m_key.PrepareContentPtr(TICKET_KEY_LENGTH), TICKET_KEY_BITS);
    }
}

bool Ticket::IsValid() const
{
    return m_bValid;
}

void Ticket::Trace(uint64_t uiFilter) const
{
    if (IsValid())
        m_key.Trace(uiFilter);
    if (m_buffer.GetLength() != 0)
        m_buffer.Trace(uiFilter);
}

}
}

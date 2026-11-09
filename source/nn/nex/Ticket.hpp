#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TICKET_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TICKET_HPP

#include <stdint.h>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/DateTime.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/RefCountedObject.hpp"

namespace nn
{
namespace nex
{

class KerberosEncryption;

class Ticket : public RefCountedObject
{
public:
    Ticket(const Buffer& rBuffer, unsigned int uiTargetPrincipalID);
    Ticket(const Buffer& rBuffer, unsigned int uiSourcePrincipalID, unsigned int uiTargetPrincipalID, const Key& rKey,
           DateTime oTime);
    Ticket(unsigned int uiSourcePrincipalID, unsigned int uiTargetPrincipalID, const Key& rKey, DateTime oTime);
    virtual ~Ticket();

    void Encrypt(KerberosEncryption* pKerberosEncryption, const Key& rKey);
    void Decrypt(KerberosEncryption* pKerberosEncryption, const Key& rKey);
    bool IsValid() const;
    void Trace(uint64_t uiFilter) const;

    unsigned int GetPrincipalID() const { return m_uiSourcePrincipalID; }

    const Key& GetKey() const { return m_key; }

private:
    friend class KerberosAuthentication;
    friend class TicketManager;

    bool m_bValid;
    Buffer m_buffer;
    unsigned int m_uiSourcePrincipalID;
    unsigned int m_uiTargetPrincipalID;
    Key m_key;
    DateTime m_time;
};

}
}

#endif

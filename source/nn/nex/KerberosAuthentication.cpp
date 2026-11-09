#include "nn/nex/KerberosAuthentication.hpp"

#include "nn/nex/AuthenticationClient.hpp"
#include "nn/nex/BitStream.hpp"
#include "nn/nex/Buffer.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/DateTime.hpp"
#include "nn/nex/KerberosEncryption.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/Platform.hpp"
#include "nn/nex/Ticket.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int MAX_TICKET_AGE = 120000;
const unsigned int REJECTED_RESPONSE = 0;

}

void KerberosAuthentication::PrepareConnectionRequest(BitStream* pRequest, AuthenticationClient* pAuthenticationClient,
                                                      Ticket* pTicket, unsigned int* pNonce)
{
    Credentials* pCredentials = pAuthenticationClient->m_pCredentials;
    *pRequest << pTicket->m_buffer;
    *pNonce = Platform::GetRandomNumber(0xFFFFFFFF);
    BitStream stream;
    unsigned int uiPrincipalID = pAuthenticationClient->m_uiPrincipalID;
    stream.Append(reinterpret_cast<const unsigned char*>(&uiPrincipalID), sizeof(uiPrincipalID), 1);
    if (pCredentials != 0)
    {
        unsigned int uiConnectionID = pCredentials->m_state;
        stream.Append(reinterpret_cast<const unsigned char*>(&uiConnectionID), sizeof(uiConnectionID), 1);
    }
    else
    {
        unsigned int uiConnectionID = 0;
        stream.Append(reinterpret_cast<const unsigned char*>(&uiConnectionID), sizeof(uiConnectionID), 1);
    }
    stream.Append(reinterpret_cast<const unsigned char*>(pNonce), sizeof(*pNonce), 1);
    Buffer encryptedRequest;
    pAuthenticationClient->m_pKerberosEncryption->Encrypt(*stream.m_pBuffer, &encryptedRequest, pTicket->m_key);
    *pRequest << encryptedRequest;
}

bool KerberosAuthentication::ValidateConnectionRequest(BitStream* pRequest, BitStream* pResponse,
                                                       AuthenticationClient* pAuthenticationClient,
                                                       unsigned int* pConnectionID, Ticket** ppTicket)
{
    Buffer requestData;
    *pRequest >> requestData;
    BitStream requestStream(&requestData);
    Buffer sessionKeyData;
    requestStream >> sessionKeyData;
    Key sessionKey(sessionKeyData.GetData(), sessionKeyData.GetLength());
    Buffer ticketData;
    requestStream >> ticketData;
    if (pRequest->m_bError || requestStream.m_bError)
    {
        BitStream rejection;
        unsigned int uiRejection = REJECTED_RESPONSE;
        rejection.Append(reinterpret_cast<const unsigned char*>(&uiRejection), sizeof(uiRejection), 1);
        *pResponse << *rejection.m_pBuffer;
        return false;
    }
    Ticket* pTicket = new Ticket(ticketData, pAuthenticationClient->m_uiPrincipalID);
    pTicket->Decrypt(pAuthenticationClient->m_pKerberosEncryption, sessionKey);
    DateTime currentTime;
    DateTime::GetSystemTime(currentTime);
    DateTime ticketTime(pTicket->m_time);
    if (currentTime - ticketTime > MAX_TICKET_AGE)
    {
        delete pTicket;
        BitStream rejection;
        unsigned int uiRejection = REJECTED_RESPONSE;
        rejection.Append(reinterpret_cast<const unsigned char*>(&uiRejection), sizeof(uiRejection), 1);
        *pResponse << *rejection.m_pBuffer;
        return false;
    }
    if (!pTicket->IsValid())
    {
        delete pTicket;
        BitStream rejection;
        unsigned int uiRejection = REJECTED_RESPONSE;
        rejection.Append(reinterpret_cast<const unsigned char*>(&uiRejection), sizeof(uiRejection), 1);
        *pResponse << *rejection.m_pBuffer;
        return false;
    }
    Buffer encryptedRequest;
    *pRequest >> encryptedRequest;
    Buffer decryptedRequest;
    pAuthenticationClient->m_pKerberosEncryption->Decrypt(encryptedRequest, &decryptedRequest, pTicket->m_key);
    BitStream decryptedStream(&decryptedRequest);
    unsigned int uiResponse;
    unsigned int uiNonce;
    unsigned int uiPrincipalID;
    unsigned int uiConnectionID;
    decryptedStream.Extract(reinterpret_cast<unsigned char*>(&uiPrincipalID), sizeof(uiPrincipalID), 1);
    decryptedStream.Extract(reinterpret_cast<unsigned char*>(&uiConnectionID), sizeof(uiConnectionID), 1);
    decryptedStream.Extract(reinterpret_cast<unsigned char*>(&uiNonce), sizeof(uiNonce), 1);
    if (decryptedStream.m_bError)
    {
        delete pTicket;
        BitStream rejection;
        unsigned int uiRejection = REJECTED_RESPONSE;
        rejection.Append(reinterpret_cast<const unsigned char*>(&uiRejection), sizeof(uiRejection), 1);
        *pResponse << *rejection.m_pBuffer;
        return false;
    }
    if (uiPrincipalID != pTicket->m_uiSourcePrincipalID)
    {
        delete pTicket;
        BitStream rejection;
        unsigned int uiRejection = REJECTED_RESPONSE;
        rejection.Append(reinterpret_cast<const unsigned char*>(&uiRejection), sizeof(uiRejection), 1);
        *pResponse << *rejection.m_pBuffer;
        return false;
    }
    *pConnectionID = uiConnectionID;
    BitStream acceptance;
    uiResponse = uiNonce + 1;
    acceptance.Append(reinterpret_cast<const unsigned char*>(&uiResponse), sizeof(uiResponse), 1);
    *pResponse << *acceptance.m_pBuffer;
    *ppTicket = pTicket;
    return true;
}

bool KerberosAuthentication::ValidateConnectionResponse(BitStream* pResponse, unsigned int uiNonce)
{
    BitStream stream;
    *pResponse >> stream;
    unsigned int uiValue;
    if (!pResponse->m_bError)
    {
        stream.Extract(reinterpret_cast<unsigned char*>(&uiValue), sizeof(uiValue), 1);
        if (!stream.m_bError && uiValue == uiNonce + 1)
            return true;
    }
    return false;
}

}
}

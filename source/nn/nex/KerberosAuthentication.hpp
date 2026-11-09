#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_KERBEROSAUTHENTICATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_KERBEROSAUTHENTICATION_HPP

namespace nn
{
namespace nex
{

class AuthenticationClient;
class BitStream;
class Ticket;

class KerberosAuthentication
{
public:
    static void PrepareConnectionRequest(BitStream* pRequest, AuthenticationClient* pAuthenticationClient,
                                         Ticket* pTicket, unsigned int* pNonce);
    static bool ValidateConnectionRequest(BitStream* pRequest, BitStream* pResponse,
                                          AuthenticationClient* pAuthenticationClient, unsigned int* pConnectionID,
                                          Ticket** ppTicket);
    static bool ValidateConnectionResponse(BitStream* pResponse, unsigned int uiNonce);
};

}
}

#endif

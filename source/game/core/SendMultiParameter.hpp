#ifndef GAME_CORE_SENDMULTIPARAMETER_HPP
#define GAME_CORE_SENDMULTIPARAMETER_HPP

#include <netinet/in.h>
#include <string.h>

#include <nex.h>

class SendMultiParameter : public nn::nex::RootObject
{
public:
    enum
    {
        PeerAddressMax = 15
    };

    SendMultiParameter() : m_lock(0x20000000), m_uiPeerAddressCount(0), m_uiSendFlags(0)
    {
        memset(m_peerAddresses, 0, sizeof(m_peerAddresses));
    }

    SendMultiParameter& operator=(const SendMultiParameter& other)
    {
        m_uiPeerAddressCount = other.m_uiPeerAddressCount;
        memcpy(m_peerAddresses, other.m_peerAddresses, sizeof(m_peerAddresses));
        m_uiSendFlags = other.m_uiSendFlags;
        return *this;
    }

    nn::nex::ForcedCriticalSection m_lock;
    unsigned int m_uiPeerAddressCount;
    sockaddr_in m_peerAddresses[PeerAddressMax];
    unsigned int m_uiSendFlags;
};

#endif

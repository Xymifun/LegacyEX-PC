#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RELAYSIGNATUREKEY_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RELAYSIGNATUREKEY_HPP

#include "nn/nex/DateTime.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class RelaySignatureKey
{
public:
    RelaySignatureKey()
        : m_currentUTCTime(),
          m_relayAddress(),
          m_relayPort(0),
          m_relayAddressType(0),
          m_relayMode(0),
          m_gameServerID(0xffffffff)
    {
    }

    unsigned int m_unknown0;
    unsigned int m_unknown4;
    DateTime m_currentUTCTime;
    String m_relayAddress;
    unsigned short m_relayPort;
    int m_relayAddressType;
    int m_relayMode;
    unsigned int m_gameServerID;
};

}
}

#endif

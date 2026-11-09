#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RELAYPROTOCOLHEADER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RELAYPROTOCOLHEADER_HPP

namespace nn
{
namespace nex
{

struct RelayProtocolHeader
{
    unsigned short m_unknown0;
    unsigned short m_uiType;
    unsigned int m_uiSourceAddress;
    unsigned int m_uiDestinationAddress;
    unsigned short m_uiSourcePort;
    unsigned short m_uiDestinationPort;
    unsigned int m_uiGameServerID;
    unsigned int m_uiGatheringID;
    int m_uiTimeStamp;
};

}
}

#endif

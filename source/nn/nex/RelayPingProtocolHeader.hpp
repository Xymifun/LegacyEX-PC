#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RELAYPINGPROTOCOLHEADER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RELAYPINGPROTOCOLHEADER_HPP

namespace nn
{
namespace nex
{

struct RelayPingProtocolHeader
{
    unsigned char m_unknown0;
    unsigned char m_unknown1;
    unsigned short m_unknown2;
    unsigned int m_unknown4;
    unsigned int m_unknown8;
};

}
}

#endif

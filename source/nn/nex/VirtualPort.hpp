#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_VIRTUALPORT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_VIRTUALPORT_HPP

#include "nn/nex/Stream.hpp"

namespace nn
{
namespace nex
{

class VirtualPort
{
public:
    VirtualPort() : m_uiValue(0) {}

    VirtualPort(Stream::Type type, unsigned char uiStreamID)
        : m_uiValue(static_cast<unsigned char>((type << 4) | uiStreamID))
    {
    }

    explicit VirtualPort(unsigned char uiValue) : m_uiValue(uiValue) {}

    ~VirtualPort() {}

    Stream::Type GetType() const { return static_cast<Stream::Type>(m_uiValue >> 4); }

    unsigned char GetStreamID() const { return static_cast<unsigned char>(m_uiValue & 0xF); }

    bool operator<(const VirtualPort& rPort) const { return m_uiValue < rPort.m_uiValue; }

    unsigned char m_uiValue;
};

}
}

#endif

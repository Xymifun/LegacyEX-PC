#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_MONITORING_DATA_HEADER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_MONITORING_DATA_HEADER_HPP

#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class MonitoringDataHeader
{
public:
    static unsigned int GetSerializedSize();
    Result Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const;
    Result Deserialize(const unsigned char* buffer, unsigned int size);
    void Trace(unsigned long long value) const;

    enum
    {
        cIdentifierSize = 8
    };

    unsigned char m_headerType;
    unsigned char m_dataType;
    unsigned char m_attribute;
    unsigned char m_reserved1;
    unsigned short m_contentSize;
    unsigned char m_identifier[cIdentifierSize];
    unsigned char m_keyIndex;
    unsigned char m_reserved2;
};

}
}
}

#endif

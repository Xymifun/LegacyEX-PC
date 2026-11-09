#ifndef NN_PIA_NAT_NATPROBEDATA_HPP
#define NN_PIA_NAT_NATPROBEDATA_HPP

#include <stdint.h>

#include "nn/pia/Common.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

class NatProbeData
{
public:
    NatProbeData();
    virtual ~NatProbeData() {}

    Result Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const;
    Result Deserialize(const unsigned char* buffer, unsigned int size);
    void Trace(uint64_t value) const;

    unsigned int m_connectionId;
    unsigned char m_type;
    unsigned char m_extra[3];
    common::Time m_time;
};

}
}
}

#endif

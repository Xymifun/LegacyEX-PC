#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_EMULATIONDEVICE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_EMULATIONDEVICE_HPP

#include <stdint.h>
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class EmulationDevice : public RootObject
{
public:
    EmulationDevice();
    virtual ~EmulationDevice() {}
    virtual void Enable();

    unsigned int GetBandwidth();
    float GetPacketDropProbability();
    unsigned int GetJitter();
    unsigned int GetLatency();
    void SetPacketDropProbability(float probability);

    void SetLatency(unsigned int latency);
    void SetJitter(unsigned int jitter);
    void SetProtocolHeaderSize(unsigned int size);
    void SetBandwidth(unsigned int bandwidth);
    unsigned int GetProtocolHeaderSize();

    bool IsEnabled() const { return m_enabled; }

private:
    bool m_enabled;
    uint32_t m_field4;
    uint32_t m_bandwidth;
    uint32_t m_latency;
    uint32_t m_jitter;
    float m_packetDropProbability;
};

}
}

#endif

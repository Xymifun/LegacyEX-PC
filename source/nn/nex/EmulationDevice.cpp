#include "nn/nex/EmulationDevice.hpp"

#include <climits>

namespace nn
{
namespace nex
{

EmulationDevice::EmulationDevice()
    : m_enabled(false),
      m_field4(0),
      m_bandwidth(UINT_MAX),
      m_latency(0),
      m_jitter(0),
      m_packetDropProbability(0.0f)
{
}

void EmulationDevice::Enable()
{
    m_enabled = true;
}

unsigned int EmulationDevice::GetBandwidth()
{
    return m_bandwidth;
}

float EmulationDevice::GetPacketDropProbability()
{
    return m_packetDropProbability;
}

unsigned int EmulationDevice::GetJitter()
{
    return m_jitter;
}

unsigned int EmulationDevice::GetLatency()
{
    return m_latency;
}

void EmulationDevice::SetPacketDropProbability(float probability)
{
    m_packetDropProbability = probability;
}

}
}

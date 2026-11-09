#include "nn/nex/NetworkEmulator.hpp"

#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

NetworkEmulator::NetworkEmulator(EmulationDevice* pDevice) : m_modem(pDevice), m_internet(pDevice), m_pDevice(pDevice)
{
}

void NetworkEmulator::Queue(Buffer* pBuffer, const InetAddress& rAddress)
{
    NetworkPacket packet(pBuffer, rAddress);
    m_modem.Queue(packet, Time::GetTime());
}

bool NetworkEmulator::GetHead(Buffer** ppBuffer, InetAddress* pAddress)
{
    while (m_modem.Ready(Time::GetTime()))
    {
        NetworkPacket packet = m_modem.GetHead(Time::GetTime());
        m_internet.Queue(packet, Time::GetTime());
        m_modem.Dequeue(m_modem.GetHead(Time::GetTime()));
    }

    if (m_internet.Ready(Time::GetTime()))
    {
        NetworkPacket packet = m_internet.GetHead(Time::GetTime());
        if (ppBuffer != 0)
            *ppBuffer = packet.m_pBuffer;

        if (pAddress != 0)
            *pAddress = packet.m_address;

        return true;
    }

    return false;
}

void NetworkEmulator::Dequeue(Buffer* pBuffer)
{
    NetworkPacket packet = m_internet.GetHead(Time::GetTime());
    m_internet.Dequeue(packet);
}

void NetworkEmulator::Trace(unsigned long long uiFilter)
{
    m_modem.Trace(uiFilter);
    m_internet.Trace(uiFilter);
}

}
}

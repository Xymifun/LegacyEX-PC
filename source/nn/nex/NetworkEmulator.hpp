#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NETWORKEMULATOR_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NETWORKEMULATOR_HPP

#include "nn/nex/Buffer.hpp"
#include "nn/nex/EmulationDevice.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/VirtualInternet.hpp"
#include "nn/nex/VirtualModem.hpp"

namespace nn
{
namespace nex
{

class NetworkEmulator : public RootObject
{
public:
    class NetworkPacket
    {
    public:
        NetworkPacket(Buffer* pBuffer, const InetAddress& rAddress)
        {
            m_pBuffer = pBuffer;
            m_pBuffer->AddRef();
            m_address = rAddress;
        }

        NetworkPacket(const NetworkPacket& rPacket)
        {
            m_pBuffer = rPacket.m_pBuffer;
            m_pBuffer->AddRef();
            m_address = rPacket.m_address;
        }

        ~NetworkPacket() { m_pBuffer->ReleaseRef(); }

        Buffer* m_pBuffer;
        InetAddress m_address;
    };

    explicit NetworkEmulator(EmulationDevice* pDevice);

    void Queue(Buffer* pBuffer, const InetAddress& rAddress);
    bool GetHead(Buffer** ppBuffer, InetAddress* pAddress);
    void Dequeue(Buffer* pBuffer);
    void Trace(unsigned long long uiFilter);

    EmulationDevice* GetDevice() const { return m_pDevice; }

private:
    VirtualModem<NetworkPacket> m_modem;
    VirtualInternet<NetworkPacket> m_internet;
    EmulationDevice* m_pDevice;
};

}
}

#endif

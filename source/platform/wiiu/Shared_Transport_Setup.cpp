#include "platform/wiiu/Shared_Transport_Setup.hpp"
#include "nn/pia/ProtocolManager.hpp"
#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/transport/TransportSupport.hpp"
#include "nn/pia/transport/UnreliableProtocol.hpp"

#include <iostream>

void checkPiaCoreToUse();

nn::pia::Result transport_InitializePiaTransportImpl(
    const nn::pia::transport::Transport::Setting& setting, unsigned short unreliablePort,
    unsigned int* unreliableProtocolHandle, unsigned short reliablePort, unsigned int* reliableProtocolHandle,
    unsigned int reliableSendBufferNum, unsigned int reliableReceiveBufferNum, nn::pia::Result (*setupCallback)(),
    unsigned int unreliableReceiveBufferNum)
{
    checkPiaCoreToUse();
    nn::pia::transport::Initialize();
    nn::pia::transport::BeginSetup();
    nn::pia::transport::Transport::CreateInstance(setting);

    if (unreliableProtocolHandle != NULL)
    {
        *unreliableProtocolHandle = nn::pia::transport::g_piaTransport->m_protocolManager
                                        .CreateProtocol<nn::pia::transport::UnreliableProtocol>(unreliablePort);
        nn::pia::transport::g_piaTransport->m_protocolManager
            .GetProtocol<nn::pia::transport::UnreliableProtocol>(*unreliableProtocolHandle)
            ->Initialize(unreliableReceiveBufferNum);
    }

    if (reliableProtocolHandle != NULL)
    {
        *reliableProtocolHandle =
            nn::pia::transport::g_piaTransport->m_protocolManager.CreateProtocol<nn::pia::transport::ReliableProtocol>(
                reliablePort);
        nn::pia::transport::g_piaTransport->m_protocolManager
            .GetProtocol<nn::pia::transport::ReliableProtocol>(*reliableProtocolHandle)
            ->Initialize(reliableSendBufferNum, reliableReceiveBufferNum);
    }

    if (setupCallback != NULL)
        setupCallback();

    nn::pia::Result result = nn::pia::transport::EndSetup();
    return result;
}

nn::pia::Result Transport_InitializePiaTransport(const nn::pia::transport::Transport::Setting& setting,
                                                 unsigned short unreliablePort, unsigned int* unreliableProtocolHandle,
                                                 unsigned short reliablePort, unsigned int* reliableProtocolHandle,
                                                 unsigned int reliableSendBufferNum,
                                                 unsigned int reliableReceiveBufferNum,
                                                 nn::pia::Result (*setupCallback)(),
                                                 unsigned int unreliableReceiveBufferNum)
{
    checkPiaCoreToUse();
    return transport_InitializePiaTransportImpl(setting, unreliablePort, unreliableProtocolHandle, reliablePort,
                                                reliableProtocolHandle, reliableSendBufferNum, reliableReceiveBufferNum,
                                                setupCallback, unreliableReceiveBufferNum);
}

void Transport_FinalizePiaTransport(unsigned int unreliableProtocolHandle, unsigned int reliableProtocolHandle,
                                    void (*finalizeCallback)())
{
    checkPiaCoreToUse();
    if (nn::pia::transport::g_piaTransport != NULL)
    {
        if (reliableProtocolHandle != 0)
        {
            nn::pia::transport::ReliableProtocol* reliable = static_cast<nn::pia::transport::ReliableProtocol*>(
                nn::pia::transport::g_piaTransport->m_protocolManager.SearchProtocol(
                    reliableProtocolHandle, nn::pia::transport::ReliableProtocol::cProtocolType));
            if (reliable != NULL)
            {
                reliable->Finalize();
                nn::pia::transport::g_piaTransport->m_protocolManager.DestroyProtocol(reliableProtocolHandle);
            }
        }
        if (unreliableProtocolHandle != 0)
        {
            nn::pia::transport::UnreliableProtocol* unreliable = static_cast<nn::pia::transport::UnreliableProtocol*>(
                nn::pia::transport::g_piaTransport->m_protocolManager.SearchProtocol(
                    unreliableProtocolHandle, nn::pia::transport::UnreliableProtocol::cProtocolType));
            if (unreliable != NULL)
            {
                unreliable->Finalize();
                nn::pia::transport::g_piaTransport->m_protocolManager.DestroyProtocol(unreliableProtocolHandle);
            }
        }
    }
    if (finalizeCallback != NULL)
        finalizeCallback();
    if (nn::pia::transport::g_piaTransport != NULL)
        nn::pia::transport::Transport::DestroyInstance();
    nn::pia::transport::Finalize();
}

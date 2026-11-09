#include "vendor/__N_22_chat_VoiceProtocol_cpp_bf22a0fe.hpp"
#include <new>
#include "nn/pia/transport/UnreliableProtocol.hpp"
#include "nn/pia/ProtocolManager.hpp"
#include "nn/pia/Transport.hpp"

template <typename T>
nn::pia::Result __N_22_chat_VoiceProtocol_cpp_bf22a0fe::createProtocol(T** protocol, unsigned short* protocolId,
                                                                       unsigned char port)
{
    if (*protocolId != 0)
        return nn::pia::transport::cResultInvalidState;
    if (*protocol != 0)
        return nn::pia::transport::cResultInvalidState;
    nn::pia::transport::Transport* transport = nn::pia::transport::g_piaTransport;
    if (transport == 0)
        return nn::pia::transport::cResultInvalidState;
    const unsigned int handle = transport->m_protocolManager.CreateProtocol<T>(port);
    *protocolId = handle;
    if (handle == 0)
        return nn::pia::transport::cResultInvalidArgument;
    *protocol = transport->m_protocolManager.GetProtocol<T>(handle);
    return 0;
}

template <typename T>
void __N_22_chat_VoiceProtocol_cpp_bf22a0fe::destroyProtocol(T** protocol, unsigned short* protocolId)
{
    if (*protocol != 0)
        (*protocol)->Finalize();
    if (nn::pia::transport::g_piaTransport != 0 && *protocolId != 0)
    {
        nn::pia::transport::g_piaTransport->m_protocolManager.DestroyProtocol(*protocolId);
        *protocolId = 0;
        *protocol = 0;
    }
}

template nn::pia::Result __N_22_chat_VoiceProtocol_cpp_bf22a0fe::createProtocol<nn::pia::transport::UnreliableProtocol>(
    nn::pia::transport::UnreliableProtocol** protocol, unsigned short* protocolId, unsigned char port);
template void __N_22_chat_VoiceProtocol_cpp_bf22a0fe::destroyProtocol<nn::pia::transport::UnreliableProtocol>(
    nn::pia::transport::UnreliableProtocol** protocol, unsigned short* protocolId);

VCTCodec __N_22_chat_VoiceProtocol_cpp_bf22a0fe::convertCodec(nn::pia::chat::Codec codec)
{
    return (VCTCodec)(unsigned int)codec;
}

nn::pia::chat::Codec __N_22_chat_VoiceProtocol_cpp_bf22a0fe::convertCodec(VCTCodec codec)
{
    return (nn::pia::chat::Codec)(unsigned int)codec;
}

nn::pia::chat::EcMode __N_22_chat_VoiceProtocol_cpp_bf22a0fe::convertEcMode(TMC_HF_EC_MODE mode)
{
    static const unsigned int conversion[3] = {0, 1, 2};
    return (nn::pia::chat::EcMode)conversion[(unsigned int)mode];
}

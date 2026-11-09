#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_INPUT_STREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_INPUT_STREAM_HPP

#include "nn/pia/common/IPacketInput.hpp"
#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/common/Socket.hpp"
#include "nn/pia/common/SocketStreamBase.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanProtocol;

class LanInputStream : public common::SocketStreamBase, public common::IPacketInput
{
public:
    enum
    {
        cIgnoreAddressMax = 4
    };

    LanInputStream();
    virtual ~LanInputStream();
    virtual Result Read(common::Packet* packet);
    virtual Result CalculateCryptoParameter(unsigned char* parameter, unsigned int size, common::Packet* packet);
    virtual Result ImportBroadcastSocket(common::Socket* socket);
    Result readFromSocket(common::Socket* socket, common::Packet* packet);
    void RegisterLanProtocol(LanProtocol* protocol);
    void UnregisterLanProtocol();
    void AddIgnoreAddress(const common::InetAddress& address);
    void ResetIgnoreAddress();
    void RemoveIgnoreAddess(const common::InetAddress& address);

private:
    common::InetAddress m_ignoreAddresses[cIgnoreAddressMax];
    common::Socket* m_broadcastSocket;
    bool m_broadcastReceived;
    LanProtocol* m_protocol;
};

}
}
}

#endif

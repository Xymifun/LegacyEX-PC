#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_UNRELIABLE_PROTOCOL_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_UNRELIABLE_PROTOCOL_HPP

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/ListBase.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class StationAddress;
}
}
}

namespace nn
{
namespace pia
{
namespace transport
{

class ProtocolEvent;
class ReceivedDataContainer;

class UnreliableProtocol : public Protocol
{
public:
    enum
    {
        cProtocolType = 0x68,
        cReceivedDataSizeMax = 0x584,
        cInvalidStationIndex = 0xfd,
        cStationIndexAll = 0xff
    };

    struct Payload
    {
        unsigned char data[cReceivedDataSizeMax];
        unsigned int size;
        unsigned long long stationId;
    };

    struct ReceivedMessage
    {
        common::ListNode node;
        Payload payload;
    };

    UnreliableProtocol();
    virtual ~UnreliableProtocol();
    virtual void Trace(unsigned long long value) const;
    virtual unsigned short GetProtocolType() const;
    virtual Result StartupWithStationAddress(const common::StationAddress& address);
    virtual void CleanupWithStationAddress();
    virtual Result Dispatch();
    virtual Result UpdateProtocolEvent(const ProtocolEvent& event);

    Result Initialize(unsigned int count);
    void Finalize();
    Result Receive(unsigned long long* stationId, void* buffer, unsigned int* size, unsigned int bufferSize);
    bool IsInCommunication() const;
    Result Send(StationIndex stationIndex, const void* data, unsigned int size);
    Result Send(unsigned long long stationId, const void* data, unsigned int size);
    void SendToAll(const void* data, unsigned int size);
    Result SendToMulti(unsigned int stationBitmap, const void* data, unsigned int size);

private:
    Result sendImpl(StationIndex stationIndex, const void* data, unsigned int size);

    void* m_receivedData;
    ReceivedDataContainer* m_receivedDataContainer;
    unsigned int m_stationIndex;
};

}
}
}

#endif

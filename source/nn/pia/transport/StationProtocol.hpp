#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_PROTOCOL_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_PROTOCOL_HPP

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/common/StationAddress.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class ConnectStationJob;

class ReceivedMessageAccessor
{
public:
    const unsigned char* m_message;
    unsigned int m_messageSize;
    unsigned long long m_sourceStationId;
    common::StationAddress m_stationAddress;
    unsigned char m_field1c;
};

class StationProtocol : public Protocol
{
public:
    enum MessageType
    {
        cMessageTypeConnectionRequest = 1,
        cMessageTypeConnectionResponse = 2,
        cMessageTypeDisconnectionRequest = 3,
        cMessageTypeDisconnectionResponse = 4,
        cMessageTypeAck = 5,
        cMessageTypeRelayConnectionRequest = 6,
        cMessageTypeRelayConnectionResponse = 7
    };

    enum
    {
        cProtocolType = 0x14
    };
    enum
    {
        cConnectionRequestHeaderSize = 17
    };
    enum
    {
        cStationIndexInvalid = 0xfd
    };

    StationProtocol();
    virtual ~StationProtocol();
    Result Initialize();
    virtual Result StartupWithStationAddress(const common::StationAddress& address);
    virtual void CleanupWithStationAddress();
    virtual Result Dispatch();
    virtual unsigned int IsProtocolFilteringEnabled() const;
    virtual unsigned short GetProtocolType() const;
    virtual void Trace(unsigned long long value) const;
    void Finalize();
    bool isValidMessage(unsigned char messageType) const;
    void Clear();
    unsigned int GetConnectionRequestDataSize() const;
    void MakeConnectionRequestData(unsigned char* buffer, unsigned char nonce, unsigned long long principalId,
                                   unsigned int infoField, unsigned char field55, bool relay, bool secure) const;
    void MakeConnectionResponseData(unsigned char* buffer, unsigned char section, unsigned long long principalId,
                                    unsigned int infoField, bool relay) const;

    void SendDenyingConnectionResponse(const common::StationAddress& address, unsigned char rejection,
                                       unsigned long long principalId, unsigned int infoField);
    void SendAck(unsigned int value, StationIndex stationIndex);
    void SendAck(unsigned int value, const common::StationAddress& address);
    void ParseConnectionRequestCommon(const ReceivedMessageAccessor& message, bool relay);
    void ParseConnectionRequest(const ReceivedMessageAccessor& message);
    void ParseConnectionResponseCommon(const ReceivedMessageAccessor& message, bool relay);
    void ParseConnectionResponse(const ReceivedMessageAccessor& message);
    unsigned int SendSimpleMessage(unsigned char messageType, const common::StationAddress& address);
    void SendDisconnectionResponse(const common::StationAddress& address);
    void ParseDisconnectionRequest(const ReceivedMessageAccessor& message);
    void ParseDisconnectionResponse(const ReceivedMessageAccessor& message);
    void ParseAck(const ReceivedMessageAccessor& message);
    void ParseRelayConnectionRequest(const ReceivedMessageAccessor& message);
    void ParseRelayConnectionResponse(const ReceivedMessageAccessor& message);
    Result ParseHelper(const ReceivedMessageAccessor& message);
    unsigned int SendSimpleMessage(unsigned char messageType, StationIndex stationIndex, bool relay);
    void SendDisconnectionRequest(StationIndex stationIndex, bool relay);
    void SendDisconnectionRequest(const common::StationAddress& address);

    friend class ConnectStationJob;
    unsigned int m_timeoutMsec;
    unsigned int m_reconnectionCount;
};

}
}
}

#endif

#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RELAY_STATION_PACKET_HANDLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RELAY_STATION_PACKET_HANDLER_HPP

#include "nn/pia.h"
#include "nn/pia/transport/StationPacketHandler.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class RelayRouteManager;

class RelayStationPacketHandler : public StationPacketHandler
{
public:
    static const pead::RuntimeTypeInfo::Interface* getRuntimeTypeInfoStatic()
    {
        static const pead::RuntimeTypeInfo::Derive<StationPacketHandler> typeInfo;
        return &typeInfo;
    }
    static bool checkDerivedRuntimeTypeInfoStatic(const pead::RuntimeTypeInfo::Interface* type)
    {
        if (type == getRuntimeTypeInfoStatic())
            return true;
        return StationPacketHandler::checkDerivedRuntimeTypeInfoStatic(type);
    }

    RelayStationPacketHandler();
    virtual bool checkDerivedRuntimeTypeInfo(const pead::RuntimeTypeInfo::Interface* type) const;
    virtual const pead::RuntimeTypeInfo::Interface* getRuntimeTypeInfo() const;
    virtual ~RelayStationPacketHandler();
    virtual void Finalize();
    virtual void Cleanup();
    virtual void Trace(unsigned long long value) const;
    virtual void RelayMessage(const ProtocolMessageReader& reader);
    virtual unsigned int CheckMessage(const ProtocolMessageReader& reader);
    virtual unsigned int CheckReceive(const ProtocolMessageReader& reader);
    virtual Result Initialize(unsigned int stationNum, bool isStarConnection, unsigned int maxProtocolNum);
    virtual ProtocolMessageWriter* AssignSingle(const ProtocolId& protocolId, ::nn::pia::StationIndex stationIndex,
                                                unsigned int size, bool isPriority, bool field602, bool field604);
    virtual ProtocolMessageWriter* AssignAll(const ProtocolId& protocolId, unsigned int size, bool isPriority,
                                             bool field602, bool field604);
    virtual ProtocolMessageWriter* AssignMulti(const ProtocolId& protocolId, unsigned int stationBitmap,
                                               unsigned int size, bool isPriority, bool field602, bool field604);
    ProtocolMessageWriter* AssignMulti(const ProtocolId& protocolId, unsigned int stationBitmap, unsigned int size,
                                       bool isPriority, bool field602, unsigned long long sourceId, bool isReliable,
                                       bool field604);
    bool AssignRelayStationMessage(::nn::pia::StationIndex relayStation, unsigned int stationBitmap, unsigned int size,
                                   unsigned int sizeLimit, bool isPriority, bool field602, bool field604);
    static Result GetRelayRoute(const RelayRouteManager* manager, ::nn::pia::StationIndex source,
                                ::nn::pia::StationIndex destination, ::nn::pia::StationIndex* relay);
    static Result GetDirectStationList(const RelayRouteManager* manager, ::nn::pia::StationIndex source,
                                       unsigned int* directBitmap);
    static Result GetDestStationList(const RelayRouteManager* manager, ::nn::pia::StationIndex source,
                                     ::nn::pia::StationIndex relay, unsigned int* destinationBitmap);

    Result Startup(PacketStream::Writer* writer, PacketStream::Reader* reader, common::IPacketInput* packetInput,
                   common::IPacketOutput* packetOutput, unsigned long long* stationId,
                   const RelayRouteManager* relayRouteManager, const common::CryptoSetting* cryptoSetting);

    const RelayRouteManager* m_relayRouteManager;
};

}
}
}

#endif

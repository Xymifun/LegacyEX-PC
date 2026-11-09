#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_PACKET_HANDLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_PACKET_HANDLER_HPP

#include "nn/pia/transport/PacketHandler.hpp"
#include "nn/pia/transport/KeepAliveSender.hpp"
#include "nn/pia/transport/KeepAliveReceiver.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class Station;
class StationConnectionInfoTable;
class StationManager;

class StationPacketHandler : public PacketHandler
{
public:
    static const pead::RuntimeTypeInfo::Interface* getRuntimeTypeInfoStatic()
    {
        static const pead::RuntimeTypeInfo::Derive<PacketHandler> typeInfo;
        return &typeInfo;
    }
    static bool checkDerivedRuntimeTypeInfoStatic(const pead::RuntimeTypeInfo::Interface* type)
    {
        if (type == getRuntimeTypeInfoStatic())
            return true;
        return PacketHandler::checkDerivedRuntimeTypeInfoStatic(type);
    }

    StationPacketHandler();
    virtual bool checkDerivedRuntimeTypeInfo(const pead::RuntimeTypeInfo::Interface* type) const;
    virtual const pead::RuntimeTypeInfo::Interface* getRuntimeTypeInfo() const;
    virtual ~StationPacketHandler();
    virtual void Finalize();
    virtual void Cleanup();
    virtual void BeginDispatch(const common::Time& time);
    virtual void EndDispatch(const common::Time& time);
    virtual unsigned int GenerateConnectionId() const;
    virtual void Trace(unsigned long long value) const;
    virtual unsigned int CheckPacket(const common::Packet& packet);
    virtual unsigned int CheckMessage(const ProtocolMessageReader& reader);
    virtual unsigned int CheckReceive(const ProtocolMessageReader& reader);
    virtual unsigned int IsBroadcastSupported() const;
    virtual Result Initialize(unsigned int stationNum, bool isStarConnection, unsigned int maxProtocolNum);
    virtual ProtocolMessageWriter* AssignSingle(const ProtocolId& protocolId, ::nn::pia::StationIndex stationIndex,
                                                unsigned int size, bool isPriority, bool field602, bool field604);
    virtual ProtocolMessageWriter* AssignAll(const ProtocolId& protocolId, unsigned int size, bool isPriority,
                                             bool field602, bool field604);
    virtual ProtocolMessageWriter* AssignMulti(const ProtocolId& protocolId, unsigned int stationBitmap,
                                               unsigned int size, bool isPriority, bool field602, bool field604);
    ProtocolMessageWriter* AssignMulti(const ProtocolId& protocolId, unsigned int stationBitmap, unsigned int size,
                                       bool isPriority, bool field602, unsigned long long sourceId, bool field604);
    common::Packet* AssignPacket(::nn::pia::StationIndex stationIndex, unsigned int stationBitmap,
                                 const common::StationAddress& address, bool isPriority, bool field602, bool field604);
    bool AssignHostMessage(unsigned int size, unsigned int sizeLimit, bool isPriority, bool field602, bool field604);
    bool AssignDirectMessage(unsigned int stationBitmap, unsigned int size, unsigned int sizeLimit, bool isPriority,
                             bool field602, bool field604);
    bool AssignAllMessage(unsigned int stationBitmap, unsigned int size, unsigned int sizeLimit, bool isPriority,
                          bool field602, bool field604);

    Result Startup(PacketStream::Writer* writer, PacketStream::Reader* reader, common::IPacketInput* packetInput,
                   common::IPacketOutput* packetOutput, unsigned long long* stationId,
                   const common::CryptoSetting* cryptoSetting);

    ProtocolMessageWriter* AssignByStationIndex(const ProtocolId& protocolId, ::nn::pia::StationIndex stationIndex,
                                                unsigned int size, bool isReliable, bool isPriority, bool isBroadcast);
    ProtocolMessageWriter* AssignByStationId(const ProtocolId& protocolId, unsigned long long stationId,
                                             unsigned int size, bool isReliable, bool isPriority, bool isBroadcast);
    StationManager* GetStationManager();
    void UpdateLocalStation();
    ProtocolMessageWriter* AssignByStationBitmap(const ProtocolId& protocolId, unsigned int stationBitmap,
                                                 unsigned int size, bool isReliable, bool isPriority, bool isBroadcast);
    static Result GetStationAddress(const StationManager* manager, common::StationAddress* address,
                                    ::nn::pia::StationIndex stationIndex);
    static unsigned int GetParticipatingStationBitmap(const StationManager* manager);
    StationConnectionInfoTable* GetStationConnectionInfoTable();
    static Station* GetStation(StationManager* manager, const common::StationAddress& address);
    static ::nn::pia::StationIndex GetStationIndex(const Station* station);
    static Station* GetLocalStation(StationManager* manager);
    static unsigned long long GetStationId(const Station* station);
    static const common::StationAddress& GetHostStationAddress(const StationConnectionInfoTable* table);
    static Station* GetStation(StationManager* manager, ::nn::pia::StationIndex stationIndex);
    static unsigned char GetSendingPassId(const Station* station);
    static unsigned short GetNextSendSequenceId(Station* station);
    static unsigned char GetReceivingPassId(const Station* station);
    static bool CheckReceivedSequenceId(Station* station, unsigned short sequenceId);

    unsigned int m_localStationIndex;
    unsigned long long m_localStationId;
    unsigned int m_localStationBitmap;
    unsigned int m_keepAliveStationBitmap;
    KeepAliveSender m_keepAliveSender;
    KeepAliveReceiver m_keepAliveReceiver;
    unsigned int m_stationNum;
    bool m_isStarConnection;
};

}
}
}

#endif

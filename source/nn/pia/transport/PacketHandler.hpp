#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PACKET_HANDLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PACKET_HANDLER_HPP

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/common/CryptoSetting.hpp"
#include "nn/pia/transport/PacketStream.hpp"
#include "nn/pia/transport/ProtocolMessageReader.hpp"
#include "nn/pia/transport/ProtocolMessageWriter.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class Packet;
class Time;
class IPacketInput;
class IPacketOutput;
}
}
}

namespace nn
{
namespace pia
{
namespace transport
{

class PacketAnalyzer;
class PacketAnalysisData;

class PacketHandler
{
public:
    enum
    {
        cPacketHeaderSize = 36,
        cPayloadSizeMax = 1436
    };

    class Iterator
    {
    public:
        explicit Iterator(PacketHandler* handler) : m_handler(handler) {}
        ProtocolMessageReader* GetMessageReader() const;
        inline void BeginIteration();
        inline bool IsEndIteration() const;
        inline void NextIteration();

    private:
        PacketHandler* m_handler;
    };

    static const pead::RuntimeTypeInfo::Interface* getRuntimeTypeInfoStatic()
    {
        static const pead::RuntimeTypeInfo::Root typeInfo;
        return &typeInfo;
    }
    static bool checkDerivedRuntimeTypeInfoStatic(const pead::RuntimeTypeInfo::Interface* type)
    {
        return type == getRuntimeTypeInfoStatic();
    }

    PacketHandler();
    virtual bool checkDerivedRuntimeTypeInfo(const pead::RuntimeTypeInfo::Interface* type) const;
    virtual const pead::RuntimeTypeInfo::Interface* getRuntimeTypeInfo() const;
    virtual ~PacketHandler();
    virtual void Finalize();
    virtual void Cleanup();
    virtual void BeginDispatch(const common::Time& time);
    virtual void EndDispatch(const common::Time& time);
    virtual unsigned int GenerateConnectionId() const;
    virtual void Trace(unsigned long long value) const;
    virtual void RelayMessage(const ProtocolMessageReader& reader);
    virtual void BeginIteration();
    virtual bool IsEndIteration() const;
    virtual void NextIteration();
    virtual unsigned int CheckPacket(const common::Packet& packet);
    virtual unsigned int CheckMessage(const ProtocolMessageReader& reader);
    virtual unsigned int CheckReceive(const ProtocolMessageReader& reader);
    virtual bool CheckFiltering(const common::StationAddress& stationAddress) const;
    virtual unsigned int IsBroadcastSupported() const;

    Iterator& GetIterator(const ProtocolId& protocolId);
    Iterator& GetIterator(unsigned char protocolType);
    Result Initialize(unsigned int maxProtocolNum);
    Result Startup(PacketStream::Writer* writer, PacketStream::Reader* reader, common::IPacketInput* packetInput,
                   common::IPacketOutput* packetOutput, unsigned long long* stationId,
                   const common::CryptoSetting* cryptoSetting);
    Result UpdateSetting(const common::CryptoSetting* cryptoSetting);
    int GetPayloadSizeLimit() const;
    Result GetPacketAnalysisData(PacketAnalysisData* send, PacketAnalysisData* unicast, PacketAnalysisData* broadcast,
                                 PacketAnalysisData* receive) const;
    void ClearPacketAnalysisData();
    ProtocolMessageWriter* AssignByStationAddress(const ProtocolId& protocolId, const common::StationAddress& address,
                                                  unsigned int size, bool isPriority, bool field602);
    common::Packet* AssignPacket(const common::StationAddress& address, bool isPriority, bool field602);
    unsigned char* AssignPacketPayload(common::Packet* packet, unsigned int size);
    bool IsBundleCandidate(const common::Packet* packet, bool field602, unsigned int sizeLimit, bool field604);
    Result Commit();
    ProtocolMessageWriter* ReserveMessageWriter();

    PacketStream::Writer* m_writer;
    PacketStream::Reader* m_reader;
    ProtocolMessageWriter m_protocolMessageWriter;
    ProtocolMessageReader m_protocolMessageReader;
    unsigned int m_payloadSize;
    unsigned int m_packetSize;
    ProtocolMessageWriter* m_pendingWriter;
    unsigned short m_field35c;
    unsigned short m_field35e;
    int m_field360;
    unsigned int m_field364;
    Iterator m_iterator;
    PacketAnalyzer* m_sendPacketAnalyzer;
    PacketAnalyzer* m_sendUnicastPacketAnalyzer;
    PacketAnalyzer* m_sendBroadcastPacketAnalyzer;
    PacketAnalyzer* m_receivePacketAnalyzer;
    common::CryptoSetting m_cryptoSetting;
    unsigned long long* m_stationId;
    common::IPacketInput* m_packetInput;
    common::IPacketOutput* m_packetOutput;
};

inline void PacketHandler::Iterator::BeginIteration()
{
    m_handler->BeginIteration();
}

inline bool PacketHandler::Iterator::IsEndIteration() const
{
    return m_handler->IsEndIteration();
}

inline void PacketHandler::Iterator::NextIteration()
{
    m_handler->NextIteration();
}

}
}
}

#endif

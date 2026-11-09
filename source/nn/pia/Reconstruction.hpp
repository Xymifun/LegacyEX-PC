#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_RECONSTRUCTION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_RECONSTRUCTION_HPP

#include <stdint.h>
#include "nn/pia/Common.hpp"
#include "nn/pia/common/WatermarkManager.hpp"
#include "nn/pia/ListBase.hpp"
#include "nn/pia/ProtocolMessageFilteringManager.hpp"
#include "nn/pia/NetworkFactory.hpp"
#include "nn/pia/session/SessionSearchCriteria.hpp"

namespace nn
{
namespace pia
{


namespace common
{

class StationAddress;


}

namespace transport
{

class PacketHandler;
class ProtocolEvent;

class ProtocolId
{
public:
    struct Element
    {
        unsigned char m_protocolType;
        unsigned char m_port;
    };

    ProtocolId();
    ProtocolId(unsigned char protocolType, unsigned char port)
    {
        m_element.m_protocolType = protocolType;
        m_element.m_port = port;
    }
    ProtocolId(unsigned short value) { m_value = value; }
    operator unsigned short() const { return m_value; }
    unsigned short value() const { return m_value; }
    unsigned char GetProtocolType() const { return m_element.m_protocolType; }
    unsigned char GetPort() const { return m_element.m_port; }

    static const ProtocolId INVALID;

    union
    {
        Element m_element;
        unsigned short m_value;
    };
};

inline ProtocolId::ProtocolId()
{
    m_value = INVALID.m_value;
}

class Protocol
{
public:
    Protocol();
    virtual ~Protocol();
    virtual void Trace(unsigned long long value) const;
    virtual unsigned short GetProtocolType() const;
    virtual Result StartupWithStationAddress(const common::StationAddress& address);
    virtual Result Startup();
    virtual void CleanupWithStationAddress();
    virtual void Cleanup();
    virtual Result Dispatch();
    virtual Result UpdateProtocolEvent(const ProtocolEvent& event);
    virtual unsigned int IsProtocolFilteringEnabled() const;
    void SetPort(unsigned char port);
    PacketHandler* GetStationPacketHandler();
    ProtocolId GetProtocolId() const { return mProtocolId; }

protected:
    friend class ProtocolManager;
    common::ListNode m_listNode;
    ProtocolId mProtocolId;
    PacketHandler* mStationPacketHandler;
};

class ReceivedDataContainer;

class UnreliableProtocol;

class ReliableSlidingWindow;
class StationManager;

class ReliableProtocol : public Protocol
{
public:
    enum
    {
        cProtocolType = 0x7c
    };

    ReliableProtocol();
    virtual ~ReliableProtocol();
    virtual void Trace(unsigned long long value) const;
    virtual unsigned short GetProtocolType() const;
    virtual Result StartupWithStationAddress(const common::StationAddress& address);
    virtual void CleanupWithStationAddress();
    virtual Result Dispatch();
    virtual Result UpdateProtocolEvent(const ProtocolEvent& event);

    void Finalize();
    Result Initialize(unsigned int a1, unsigned int a2);
    ReliableSlidingWindow* GetSlidingWindow(StationIndex stationIndex);
    const ReliableSlidingWindow* GetSlidingWindow(StationIndex stationIndex) const;
    Result Send(StationIndex stationIndex, const void* data, unsigned int size);
    Result Send(unsigned long long stationId, const void* data, unsigned int size);
    Result SendToAll(const void* data, unsigned int size);
    Result Receive(unsigned long long* stationId, void* buffer, unsigned int* size, unsigned int bufferSize);
    bool IsInCommunication(StationIndex stationIndex) const;
    bool IsInCommunication(unsigned long long stationId) const;
    void SetThroughputLimit(unsigned int limit);
    static void RegisterResendTimeCallback(int (*callback)(int, int));

private:
    int m_field14;
    unsigned long long m_localStationId;
    unsigned int m_field20;
    unsigned char* m_field24;
    unsigned int m_dispatchIndex;
    unsigned int m_throughputLimit;
    unsigned int m_sendBudget;
};

}

namespace session
{

class SessionStationInfo
{
public:
    enum StationInfoRole
    {
        cStationInfoRole_Invalid = 0
    };

    SessionStationInfo();
    virtual void Clear();
    virtual ~SessionStationInfo();
    StationInfoRole GetRole() const;
    void SetRole(StationInfoRole role);
    static unsigned int GetSerializedSize();

    StationInfoRole m_role;
};

}

}
}

#endif

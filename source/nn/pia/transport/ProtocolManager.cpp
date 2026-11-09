#include "nn/pia/ProtocolManager.hpp"
#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/Memory.hpp"
#include "platform/wiiu/pead.hpp"
#include "nn/pia/Transport.hpp"

#include <stddef.h>

namespace nn
{
namespace pia
{
namespace transport
{

ProtocolManager::ProtocolManager() : m_networkFactory(0), m_protocols()
{
    m_protocols.SetOffset(offsetof(Protocol, m_listNode));
    m_started = 0;
}

ProtocolManager::~ProtocolManager()
{
    Finalize();
}

Result ProtocolManager::Initialize(NetworkFactory* networkFactory)
{
    m_networkFactory = networkFactory;
    m_started = 0;
    return 0;
}

void ProtocolManager::Trace(unsigned long long) const
{
}

Result ProtocolManager::Startup(PacketHandler* packetHandler)
{
    if (packetHandler == 0)
        return static_cast<Result>(transport::cResultInvalidArgument);
    if (m_started != 0)
        return static_cast<Result>(transport::cResultInvalidState);

    const common::OffsetList::iterator end = m_protocols.end();
    for (common::OffsetList::iterator it = m_protocols.begin(); it != end; ++it)
    {
        static_cast<Protocol*>(*it)->mStationPacketHandler = packetHandler;
    }
    m_started = 1;
    return 0;
}

Result ProtocolManager::StartupProtocols()
{
    if (m_started == 0)
        return static_cast<Result>(transport::cResultInvalidState);

    const common::OffsetList::iterator end = m_protocols.end();
    for (common::OffsetList::iterator it = m_protocols.begin(); it != end; ++it)
    {
        const Result result = static_cast<Protocol*>(*it)->Startup();
        if (result != 0)
        {
            CleanupProtocols();
            return result;
        }
    }
    return 0;
}

Result ProtocolManager::Dispatch()
{
    if (m_started == 0)
        return static_cast<Result>(transport::cResultInvalidState);

    const common::OffsetList::iterator end = m_protocols.end();
    for (common::OffsetList::iterator it = m_protocols.begin(); it != end; ++it)
    {
        const Result result = static_cast<Protocol*>(*it)->Dispatch();
        if (result != 0)
            return result;
    }
    return 0;
}

Result ProtocolManager::UpdateProtocolEvent(const ProtocolEvent& event)
{
    if (m_started == 0)
        return static_cast<Result>(transport::cResultInvalidState);

    Result firstFailure = 0;
    const common::OffsetList::iterator end = m_protocols.end();
    for (common::OffsetList::iterator it = m_protocols.begin(); it != end; ++it)
    {
        const Result result = static_cast<Protocol*>(*it)->UpdateProtocolEvent(event);
        if (result != 0 && firstFailure == 0)
            firstFailure = result;
    }
    return firstFailure;
}

void ProtocolManager::Cleanup()
{
    const common::OffsetList::iterator end = m_protocols.end();
    for (common::OffsetList::iterator it = m_protocols.begin(); it != end; ++it)
    {
        static_cast<Protocol*>(*it)->mStationPacketHandler = 0;
    }
    m_started = 0;
}

void ProtocolManager::CleanupProtocolsWithStationAddress()
{
    const common::OffsetList::iterator end = m_protocols.end();
    for (common::OffsetList::iterator it = m_protocols.begin(); it != end; ++it)
    {
        static_cast<Protocol*>(*it)->CleanupWithStationAddress();
    }
}

Result ProtocolManager::StartupProtocolsWithStationAddress(const common::StationAddress& address)
{
    if (!m_started)
        return static_cast<Result>(transport::cResultInvalidState);

    const common::OffsetList::iterator end = m_protocols.end();
    for (common::OffsetList::iterator it = m_protocols.begin(); it != end; ++it)
    {
        const Result result = static_cast<Protocol*>(*it)->StartupWithStationAddress(address);
        if (result != 0)
        {
            CleanupProtocolsWithStationAddress();
            return result;
        }
    }
    return 0;
}

void ProtocolManager::CleanupProtocols()
{
    const common::OffsetList::iterator end = m_protocols.end();
    for (common::OffsetList::iterator it = m_protocols.begin(); it != end; ++it)
    {
        static_cast<Protocol*>(*it)->Cleanup();
    }
}

}
}
}

namespace
{
unsigned int g_piaProtocolTypeMask = 0;

unsigned int getProtocolTypeBit(unsigned short protocolType)
{
    switch (protocolType)
    {
    case 0x14:
        return 2;
    case 0x18:
        return 4;
    case 0x1c:
        return 8;
    case 0x34:
        return 0x10;
    case 0x58:
        return 0x20;
    case 0x64:
        return 0x40;
    case 0x68:
        return 0x80;
    case 0x7c:
        return 0x100;
    case 0x84:
        return 0x200;
    case 0xa4:
        return 0x400;
    case 0x6c:
        return 0x1000;
    case 0x74:
        return 0x4000;
    case 0x78:
        return 0x8000;
    case 0x94:
        return 0x10000;
    case 0x44:
        return 0x20000;
    case 0x54:
        return 0x40000;
    case 0x24:
        return 0x80000;
    case 0x28:
        return 0x100000;
    case 0x2c:
        return 0x200000;
    default:
        return 0;
    }
}
}

namespace nn
{
namespace pia
{
namespace transport
{

unsigned short ProtocolManager::CreateProtocolImpl(Protocol* protocol, ProtocolId id)
{
    const unsigned short value = id;
    const unsigned char type = static_cast<unsigned char>(value >> 8);
    if (m_networkFactory->IsSupportedProtocol(type) == 0)
        return ProtocolId::INVALID;
    protocol->SetPort(static_cast<unsigned char>(value));
    Protocol* end = static_cast<Protocol*>(m_protocols.End());
    Protocol* current = static_cast<Protocol*>(m_protocols.Begin());
    for (; current != end; current = static_cast<Protocol*>(m_protocols.Next(current)))
    {
        const unsigned short currentId = static_cast<unsigned short>(current->mProtocolId);
        if (value <= currentId)
        {
            if (currentId <= value)
                return ProtocolId::INVALID;
            break;
        }
    }
    m_protocols.InsertBeforeNode(m_protocols.ToListNode(current), m_protocols.ToListNode(protocol));
    if (protocol->IsProtocolFilteringEnabled() == 0)
        g_piaProtocolMessageFilteringManager->AddNoFilteringProtocolType(type);
    g_piaProtocolTypeMask &= ~getProtocolTypeBit(type);
    return value;
}

void ProtocolManager::DestroyProtocol(unsigned short id)
{
    const common::OffsetList::iterator end = m_protocols.end();
    for (common::OffsetList::iterator it = m_protocols.begin(); it != end; ++it)
    {
        Protocol* protocol = static_cast<Protocol*>(*it);
        if (protocol->GetProtocolId() == id)
        {
            if (protocol->IsProtocolFilteringEnabled() == 0)
                g_piaProtocolMessageFilteringManager->RemoveNoFilteringProtocolType(
                    static_cast<unsigned char>(protocol->GetProtocolId() >> 8));
            m_protocols.EraseNode(m_protocols.ToListNode(protocol));
            protocol->~Protocol();
            common::DeleteArray(reinterpret_cast<unsigned char*>(protocol));
            return;
        }
    }
}

void* ProtocolManager::AllocProtocol(unsigned int size)
{
    return common::NewArray<unsigned char>(size);
}

Protocol* ProtocolManager::SearchProtocol(ProtocolId id, unsigned char type)
{
    if (id.GetProtocolType() != type)
        return 0;
    for (common::OffsetList::iterator it = m_protocols.begin(); it != m_protocols.end(); ++it)
    {
        Protocol* protocol = static_cast<Protocol*>(*it);
        if (protocol->GetProtocolId() == id)
            return protocol;
    }
    return 0;
}

void ProtocolManager::Finalize()
{
    m_networkFactory = 0;
    m_started = 0;
    while (m_protocols.GetNodeNum() != 0)
    {
        Protocol* protocol = static_cast<Protocol*>(m_protocols.FromListNode(m_protocols.PopBackNode()));
        protocol->~Protocol();
        common::DeleteArray(reinterpret_cast<unsigned char*>(protocol));
    }
}

}
}
}

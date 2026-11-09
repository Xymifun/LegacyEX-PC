#include "nn/pia/inet/NexNatRelay.hpp"

#include <coreinit/memory.h>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/RelaySignatureKey.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/qList.hpp"
#include "nn/pia/String.hpp"
#include "nn/pia/common/Md5Context.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/Socket.hpp"
#include "nn/pia/common/SocketAddress.hpp"
#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/transport/StationLocation.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{

enum
{
    cResultSuccess = 0,
    cResultNotSupported = 0xc0b,
    cResultInvalidRelayPacket = 0x2c09,
    cResultRequestFailed = 0x2c0a,
    cResultCancelled = 0x6c05,
    cResultInvalidState = 0x10c08
};

enum
{
    cTraceNatRelay = 0x800,
    cTraceStationLocation = 0x1000
};

enum
{
    cUndefinedStreamType = 0xff
};

enum
{
    cRelayAddressTypeHostName = 1,
    cDefaultRelayServerPort = 12525,
    cRelayHostNameLengthMax = 255
};

enum
{
    cRelayServerSettingTimeoutMsec = 10000,
    cRelayEntryLifetimeMsec = 15000,
    cHashedHeaderSize = 0x1c,
    cHashSize = 16,
    cHashedPayloadSizeMax = 0x32,
    cRelayHeaderSize = cHashedHeaderSize + cHashSize,
    cRelayTimeToleranceSec = 150
};

const unsigned int cRelayPacketMagic = 0xf5d00000;
const unsigned int cGameServerIdMask = 0xdb319579;
const unsigned int cGatheringIdMask = 0xbee17d2f;

common::InetAddress s_invalidAddress;

inline unsigned int SwapU32(unsigned int value)
{
    unsigned int swapped = ((value & 0xff00ff00) >> 8) | ((value & 0x00ff00ff) << 8);
    return (swapped << 16) | (swapped >> 16);
}

inline unsigned short SwapU16(unsigned short value)
{
    return static_cast<unsigned short>((value << 8) | (value >> 8));
}

}

void GetRelaySignatureKeyCompletionCallback(nex::CallContext*, const nex::UserContext*)
{
}

NexNatRelay::NexNatRelay()
{
    m_targetUrlList = common::NewObj<nex::qList<nex::StationURL> >();
    m_targetUrlList->push_back(nex::StationURL());
    m_probeTargetUrl = common::NewObj<nex::StationURL>();
    m_nexInetAddress = common::NewObj<nex::InetAddress>();
    m_relayCallContext = common::NewObj<nex::ProtocolCallContext>();
    m_signatureKeyCallContext = common::NewObj<nex::ProtocolCallContext>();
    m_relaySignatureKey = common::NewObj<nex::RelaySignatureKey>();
    m_relayInterface.AssociateRelay(this, m_relayCallContext);
    m_relayServerAddress.m_address = 0;
    m_relayServerAddress.m_port = 0;
    m_relayTable.Clear();
}

NexNatRelay::~NexNatRelay()
{
    m_relayInterface.AssociateRelay(0, 0);
    if (m_targetUrlList != 0)
    {
        m_targetUrlList->clear();
        common::DeleteObj(m_targetUrlList);
        m_targetUrlList = 0;
    }
    if (m_probeTargetUrl != 0)
    {
        common::DeleteObj(m_probeTargetUrl);
        m_probeTargetUrl = 0;
    }
    if (m_nexInetAddress != 0)
    {
        common::DeleteObj(m_nexInetAddress);
        m_nexInetAddress = 0;
    }
    if (m_relayCallContext != 0)
    {
        common::DeleteObj(m_relayCallContext);
        m_relayCallContext = 0;
    }
    if (m_relaySignatureKey != 0)
    {
        common::DeleteObj(m_relaySignatureKey);
        m_relaySignatureKey = 0;
    }
    if (m_signatureKeyCallContext != 0)
    {
        common::DeleteObj(m_signatureKeyCallContext);
        m_signatureKeyCallContext = 0;
    }
    m_relayTable.Clear();
}

void NexNatRelay::RequestProbe(const transport::StationLocation& location)
{
    NexFacade::GetInstance()->NotifyProbeRequest(location);
}

void NexNatRelay::RelayProbeRequest(const transport::StationLocation& source,
                                    const transport::StationLocation& destination)
{
    if (!NexFacade::GetInstance()->m_started)
        return;
    if (m_nexInetAddress == 0)
        return;
    if (m_targetUrlList == 0)
        return;

    source.Trace(cTraceNatRelay);
    destination.Trace(cTraceNatRelay);

    nex::StationURL& sourceUrl = m_targetUrlList->front();
    NexFacade::ConvertInetAddressToNexInetAddress(source.m_stationAddress.m_inetAddress, m_nexInetAddress);
    sourceUrl.SetInetAddress(m_nexInetAddress);
    sourceUrl.SetPrincipalID(static_cast<unsigned int>(source.m_principalId));
    sourceUrl.SetConnectionID(source.m_field18);
    sourceUrl.SetRVConnectionID(source.m_field1c);
    sourceUrl.SetURLType(static_cast<nex::StationURL::URLType>(source.m_field20));
    sourceUrl.SetStreamID(source.m_field21);
    sourceUrl.SetStreamType(static_cast<nex::Stream::Type>(source.m_field22));
    sourceUrl.SetNATMapping(static_cast<nex::NATProperties::MappingProperties>(source.m_field23));
    sourceUrl.SetNATFiltering(static_cast<nex::NATProperties::FilteringProperties>(source.m_field24));
    sourceUrl.SetType(source.m_field25);
    sourceUrl.SetProbeRequestInitiation(source.m_field26 != 0);
    if (source.m_inetAddress.IsValid())
    {
        sourceUrl.SetUseRelayServer(true);
        NexFacade::ConvertInetAddressToNexInetAddress(source.m_inetAddress, m_nexInetAddress);
        sourceUrl.SetRelayServerAddress(m_nexInetAddress->GetAddressStr());
        sourceUrl.SetRelayServerPort(source.m_inetAddress.m_port);
        NexFacade::ConvertInetAddressToNexInetAddress(source.m_stationAddress.m_inetAddress, m_nexInetAddress);
        sourceUrl.SetRelayAddress(m_nexInetAddress->GetAddressStr());
        sourceUrl.SetRelayPort(source.m_stationAddress.m_inetAddress.m_port);
    }
    else
    {
        sourceUrl.SetUseRelayServer(false);
        NexFacade::ConvertInetAddressToNexInetAddress(s_invalidAddress, m_nexInetAddress);
        sourceUrl.SetRelayServerAddress(m_nexInetAddress->GetAddressStr());
        sourceUrl.SetRelayServerPort(0);
        sourceUrl.SetRelayAddress(m_nexInetAddress->GetAddressStr());
        sourceUrl.SetRelayPort(0);
    }

    nex::StationURL& destinationUrl = *m_probeTargetUrl;
    NexFacade::ConvertInetAddressToNexInetAddress(destination.m_stationAddress.m_inetAddress, m_nexInetAddress);
    destinationUrl.SetInetAddress(m_nexInetAddress);
    destinationUrl.SetPrincipalID(static_cast<unsigned int>(destination.m_principalId));
    destinationUrl.SetConnectionID(destination.m_field18);
    destinationUrl.SetRVConnectionID(destination.m_field1c);
    destinationUrl.SetURLType(static_cast<nex::StationURL::URLType>(destination.m_field20));
    destinationUrl.SetStreamID(destination.m_field21);
    destinationUrl.SetStreamType(static_cast<nex::Stream::Type>(
        source.m_field22 == cUndefinedStreamType ? cUndefinedStreamType : destination.m_field22));
    destinationUrl.SetNATMapping(static_cast<nex::NATProperties::MappingProperties>(destination.m_field23));
    destinationUrl.SetNATFiltering(static_cast<nex::NATProperties::FilteringProperties>(destination.m_field24));
    destinationUrl.SetType(destination.m_field25);
    destinationUrl.SetProbeRequestInitiation(destination.m_field26 != 0);
    if (destination.m_inetAddress.IsValid())
    {
        destinationUrl.SetUseRelayServer(true);
        NexFacade::ConvertInetAddressToNexInetAddress(destination.m_inetAddress, m_nexInetAddress);
        destinationUrl.SetRelayServerAddress(m_nexInetAddress->GetAddressStr());
        destinationUrl.SetRelayServerPort(destination.m_inetAddress.m_port);
        NexFacade::ConvertInetAddressToNexInetAddress(destination.m_stationAddress.m_inetAddress, m_nexInetAddress);
        destinationUrl.SetRelayAddress(m_nexInetAddress->GetAddressStr());
        destinationUrl.SetRelayPort(destination.m_stationAddress.m_inetAddress.m_port);

        bool found = false;
        for (common::ListNode* node = m_relayTable.Sentinel()->next; node != m_relayTable.Sentinel(); node = node->next)
        {
            RelayEntry* entry = RelayTable::ValueOf(node);
            const common::InetAddress& entryAddress = entry->m_stationAddress;
            if (entryAddress == source.m_stationAddress.m_inetAddress
                || entry->m_connectionId == static_cast<unsigned int>(source.m_field1c))
            {
                found = true;
                if (!entry->m_relayAddress.IsValid())
                {
                    if (source.m_inetAddress.IsValid())
                        entry->m_relayAddress = source.m_inetAddress;
                    else
                        entry->m_relayAddress = destination.m_inetAddress;
                }
                entry->m_stationAddress = source.m_stationAddress.m_inetAddress;
                entry->m_connectionId = source.m_field1c;
                entry->m_expireTime.m_tick = common::TimeSpan::GetTicksPerMSec().m_tick * cRelayEntryLifetimeMsec
                                             + common::g_piaJobScheduler->m_dispatchTime.m_tick;
            }
        }
        if (!found)
        {
            RelayEntry* entry = m_relayTable.Alloc();
            if (entry != 0)
            {
                entry->m_stationAddress = source.m_stationAddress.m_inetAddress;
                entry->m_relayAddress = destination.m_inetAddress;
                entry->m_connectionId = source.m_field1c;
                entry->m_expireTime.m_tick = common::TimeSpan::GetTicksPerMSec().m_tick * cRelayEntryLifetimeMsec
                                             + common::g_piaJobScheduler->m_dispatchTime.m_tick;
            }
        }
    }
    else
    {
        destinationUrl.SetUseRelayServer(false);
        NexFacade::ConvertInetAddressToNexInetAddress(s_invalidAddress, m_nexInetAddress);
        destinationUrl.SetRelayServerAddress(m_nexInetAddress->GetAddressStr());
        destinationUrl.SetRelayServerPort(0);
        destinationUrl.SetRelayAddress(m_nexInetAddress->GetAddressStr());
        destinationUrl.SetRelayPort(0);
    }

    m_relayInterface.RelayProbeRequest(*m_targetUrlList, *m_probeTargetUrl);
}

void NexNatRelay::ReportNatTraversalResult(unsigned int connectionId, bool success, unsigned int rtt)
{
    if (NexFacade::GetInstance()->m_started)
        m_relayInterface.ReportNATTraversalResult(connectionId, success, rtt);
}

void NexNatRelay::ReportNatProperties(unsigned int mapping, unsigned int filtering, unsigned int portIncrement)
{
    if (NexFacade::GetInstance()->m_started)
        m_relayInterface.ReportNATProperties(mapping, filtering, portIncrement);
}

void NexNatRelay::SetLocalCID(unsigned int connectionId)
{
    NexFacade::GetInstance()->NotifyLocalServiceConnectionId(connectionId);
}

void NexNatRelay::RequestRelayServerSettingAsync()
{
    if (NexFacade::GetInstance()->m_started && NexFacade::GetInstance()->IsRelayServiceEnabled()
        && m_signatureKeyCallContext->GetState() != nex::CallContext::CallInProgress)
    {
        m_relayServerAddress.m_address = 0;
        m_relayServerAddress.m_port = 0;
        m_signatureKeyCallContext->Reset();
        m_signatureKeyCallContext->SetTimeout(cRelayServerSettingTimeoutMsec);
        m_signatureKeyCallContext->RegisterCompletionCallback(GetRelaySignatureKeyCompletionCallback,
                                                              nex::UserContext(), true);
        m_relayInterface.GetRelaySignatureKey(m_signatureKeyCallContext, m_relaySignatureKey);
    }
}

bool NexNatRelay::IsRequestRelayServerSettingCompleted() const
{
    if (NexFacade::GetInstance()->IsRelayServiceEnabled())
    {
        nex::CallContext::State state = m_signatureKeyCallContext->GetState();
        return state == nex::CallContext::CallSucceeded || state == nex::CallContext::CallFailed
               || state == nex::CallContext::CallCanceled;
    }
    return true;
}

Result NexNatRelay::GetRequestRelayServerSettingResult() const
{
    if (NexFacade::GetInstance()->IsRelayServiceEnabled())
    {
        if (m_signatureKeyCallContext->GetState() == nex::CallContext::CallSucceeded)
            return Result(cResultSuccess);
        else if (m_signatureKeyCallContext->GetState() == nex::CallContext::CallCanceled)
            return Result(cResultCancelled);
        else if (m_signatureKeyCallContext->GetState() == nex::CallContext::CallFailed)
            return Result(cResultRequestFailed);
        return Result(cResultInvalidState);
    }
    return Result(cResultNotSupported);
}

void NexNatRelay::ResolveRelayServerSetting()
{
    if (!NexFacade::GetInstance()->m_started)
        return;
    if (!NexFacade::GetInstance()->IsRelayServiceEnabled())
        return;

    common::InetAddress address;
    const nex::RelaySignatureKey* key = m_relaySignatureKey;
    char hostName[cRelayHostNameLengthMax + 1];
    unsigned int length = 0;
    const wchar_t* source = key->m_relayAddress;
    char* destination = hostName;
    while (length < key->m_relayAddress.GetLength() && length < cRelayHostNameLengthMax)
    {
        unsigned short character = *source;
        if (character >= L' ' && character <= L'~')
        {
            *destination = static_cast<char>(character);
            ++destination;
            ++length;
        }
        ++source;
    }
    hostName[length] = '\0';

    if (key->m_relayMode > 0)
    {
        if (key->m_relayAddressType == cRelayAddressTypeHostName)
        {
            common::SocketAddress socketAddress;
            if (socketAddress.GetAddressInfo(hostName, 0, 0))
                socketAddress.GetInetAddress(&address);
        }
        else
        {
            Result result = common::Socket::InetAtoH(hostName, &address);
            if (result != cResultSuccess)
            {
                address.m_address = 0;
                address.m_port = 0;
            }
        }
        address.m_port = key->m_relayPort != 0 ? key->m_relayPort : cDefaultRelayServerPort;
    }
    SetRelayServerAddress(address);
}

const nex::RelaySignatureKey* NexNatRelay::GetRelayServerSetting() const
{
    return NexFacade::GetInstance()->IsRelayServiceEnabled() ? m_relaySignatureKey : 0;
}

unsigned int NexNatRelay::IsRelayServerAvailable() const
{
    return m_relayServerAddress.IsValid();
}

void NexNatRelay::SetRelayServerAddress(const common::InetAddress& address)
{
    m_relayServerAddress = address;
    m_relayServerAddressTime = common::g_piaJobScheduler->m_dispatchTime;
    common::g_sessionBeginMonitoringContent.m_metric156 = address.m_address;
    common::g_sessionBeginMonitoringContent.m_metric157 = address.m_port;
}

const common::InetAddress& NexNatRelay::GetRelayServerAddress() const
{
    return m_relayServerAddress;
}

unsigned int NexNatRelay::GetCurrentUTCEpochTime() const
{
    if (NexFacade::GetInstance()->IsRelayServiceEnabled() && m_relayServerAddress.IsValid())
    {
        long long elapsed = (common::g_piaJobScheduler->m_dispatchTime.m_tick - m_relayServerAddressTime.m_tick)
                            / common::TimeSpan::GetTicksPerSec().m_tick;
        return static_cast<unsigned int>(m_relaySignatureKey->m_currentUTCTime.ToEpochTime() + elapsed);
    }
    return 0;
}

unsigned int NexNatRelay::GetGameServerId() const
{
    if (NexFacade::GetInstance()->IsRelayServiceEnabled() && m_relayServerAddress.IsValid())
        return m_relaySignatureKey->m_gameServerID;
    return 0;
}

void NexNatRelay::SetRelayGatheringId(unsigned int gatheringId)
{
    m_relayGatheringId = gatheringId;
}

unsigned int NexNatRelay::GetRelayGatheringId() const
{
    return m_relayGatheringId;
}

void NexNatRelay::SetRelayServerSignatureKey(unsigned char* key)
{
    OSBlockMove(m_relayServerSignatureKey, key, cSignatureKeySize, FALSE);
}

const unsigned char* NexNatRelay::GetRelayServerSignatureKey() const
{
    return m_relayServerSignatureKey;
}

void NexNatRelay::UpdateRelayTable()
{
    common::ListNode* node = m_relayTable.Sentinel()->next;
    common::ListNode* next = node->next;
    while (node != m_relayTable.Sentinel())
    {
        RelayEntry* entry = RelayTable::ValueOf(node);
        if (entry->m_expireTime < common::g_piaJobScheduler->m_dispatchTime)
        {
            transport::StationLocation location;
            common::StationAddress stationAddress;
            location.m_field1c = entry->m_connectionId;
            stationAddress.SetInetAddress(entry->m_stationAddress);
            location.SetStationAddress(stationAddress);
            if (!NexFacade::GetInstance()->IsTraversed(location))
            {
                m_relayTable.Destroy(entry);
            }
            else
            {
                entry->m_expireTime.m_tick = common::TimeSpan::GetTicksPerMSec().m_tick * cRelayEntryLifetimeMsec
                                             + common::g_piaJobScheduler->m_dispatchTime.m_tick;
            }
        }
        node = next;
        next = node->next;
    }
}

bool NexNatRelay::RemoveRelayEntry(const transport::StationLocation& location)
{
    bool removed = false;
    NexFacade::GetInstance()->m_natRelay->Trace(cTraceStationLocation);
    location.Trace(cTraceStationLocation);
    common::ListNode* node = m_relayTable.Sentinel()->next;
    common::ListNode* next = node->next;
    while (node != m_relayTable.Sentinel())
    {
        RelayEntry* entry = RelayTable::ValueOf(node);
        const common::InetAddress& stationAddress = entry->m_stationAddress;
        if (stationAddress == location.m_stationAddress.m_inetAddress
            || entry->m_connectionId == static_cast<unsigned int>(location.m_field1c))
        {
            m_relayTable.Destroy(entry);
            removed = true;
        }
        node = next;
        next = node->next;
    }
    NexFacade::GetInstance()->m_natRelay->Trace(cTraceStationLocation);
    return removed;
}

void NexNatRelay::Cleanup()
{
    m_relayServerAddress.m_address = 0;
    m_relayServerAddress.m_port = 0;
    common::Time now;
    now.SetNow();
    m_relayServerAddressTime = now;
    m_relayGatheringId = 0;
    OSBlockSet(m_relayServerSignatureKey, 0, cSignatureKeySize);
    if (NexFacade::GetInstance()->IsRelayServiceEnabled() && m_relaySignatureKey != 0)
    {
        m_relaySignatureKey->m_gameServerID = 0xffffffff;
        m_relaySignatureKey->m_relayPort = 0;
        m_relaySignatureKey->m_relayAddressType = 0;
        m_relaySignatureKey->m_relayMode = 0;
        m_relaySignatureKey->m_currentUTCTime = nex::DateTime::s_never;
        m_relaySignatureKey->m_relayAddress = nex::String();
    }
    m_relayTable.Clear();
    common::g_sessionBeginMonitoringContent.m_metric156 = 0xffffffff;
    common::g_sessionBeginMonitoringContent.m_metric157 = 0xffff;
}

void NexNatRelay::Trace(unsigned long long)
{
    common::FixedString<32> relayAddress;
    common::FixedString<32> stationAddress;
    m_relayServerAddress.GetAddressString(&relayAddress);
    for (common::ListNode* node = m_relayTable.Sentinel()->next; node != m_relayTable.Sentinel(); node = node->next)
    {
        relayAddress.Clear();
        stationAddress.Clear();
        RelayTable::ValueOf(node)->m_relayAddress.GetAddressString(&relayAddress);
        RelayTable::ValueOf(node)->m_stationAddress.GetAddressString(&stationAddress);
    }
}

Result NexNatRelay::Pack(void* buffer, const void* payload, unsigned int payloadSize, const common::InetAddress& source,
                         const common::InetAddress& destination)
{
    unsigned char* header = static_cast<unsigned char*>(buffer);
    unsigned char* cursor = header;
    common::serializeU32(cursor, cRelayPacketMagic);
    cursor += sizeof(unsigned int);
    common::serializeU32(cursor, SwapU32(source.m_address));
    cursor += sizeof(unsigned int);
    common::serializeU32(cursor, SwapU32(destination.m_address));
    cursor += sizeof(unsigned int);
    common::serializeU16(cursor, SwapU16(source.m_port));
    cursor += sizeof(unsigned short);
    common::serializeU16(cursor, SwapU16(destination.m_port));
    cursor += sizeof(unsigned short);
    common::serializeU32(cursor, SwapU32(GetGameServerId() ^ cGameServerIdMask));
    cursor += sizeof(unsigned int);
    common::serializeU32(cursor, SwapU32(GetRelayGatheringId() ^ cGatheringIdMask));
    cursor += sizeof(unsigned int);
    common::serializeU32(cursor, SwapU32(GetCurrentUTCEpochTime()));
    cursor += sizeof(unsigned int);

    common::Md5Context md5;
    md5.Initialize();
    md5.Update(GetRelayServerSignatureKey(), cSignatureKeySize);
    md5.Update(header, cHashedHeaderSize);
    md5.Update(payload, payloadSize > cHashedPayloadSizeMax ? cHashedPayloadSizeMax : payloadSize);
    md5.GetHash(cursor);
    cursor += cHashSize;

    OSBlockMove(cursor, payload, payloadSize, FALSE);
    return Result(cResultSuccess);
}

Result NexNatRelay::Unpack(void* buffer, int* size, common::InetAddress* source)
{
    unsigned char* header = static_cast<unsigned char*>(buffer);
    if (*static_cast<char*>(buffer) != static_cast<char>(cRelayPacketMagic >> 24))
        return Result(cResultSuccess);
    if (common::deserializeU32(header) != cRelayPacketMagic)
        return Result(cResultInvalidRelayPacket);
    if (static_cast<unsigned int>(*size) <= cRelayHeaderSize)
        return Result(cResultInvalidRelayPacket);

    unsigned int payloadSize = *size - cRelayHeaderSize;
    common::Md5Context md5;
    md5.Initialize();
    md5.Update(NexFacade::GetInstance()->m_natRelay->GetRelayServerSignatureKey(), cSignatureKeySize);
    md5.Update(header, cHashedHeaderSize);
    md5.Update(header + cRelayHeaderSize, payloadSize > cHashedPayloadSizeMax ? cHashedPayloadSizeMax : payloadSize);
    char hash[cHashSize];
    md5.GetHash(hash);
    const char* signature = reinterpret_cast<const char*>(header + cHashedHeaderSize);
    for (int i = 0; i < cHashSize; ++i)
    {
        if (hash[i] != signature[i])
            return Result(cResultInvalidRelayPacket);
    }

    unsigned int timestamp = SwapU32(common::deserializeU32(header + cHashedHeaderSize - sizeof(unsigned int)));
    unsigned int lower = NexFacade::GetInstance()->m_natRelay->GetCurrentUTCEpochTime() - cRelayTimeToleranceSec;
    unsigned int upper = NexFacade::GetInstance()->m_natRelay->GetCurrentUTCEpochTime() + cRelayTimeToleranceSec;
    if (lower < upper)
    {
        if (upper < timestamp || timestamp < lower)
            return Result(cResultInvalidRelayPacket);
    }
    else if (upper < timestamp && timestamp < lower)
    {
        return Result(cResultInvalidRelayPacket);
    }

    source->m_address = SwapU32(common::deserializeU32(header + sizeof(unsigned int)));
    source->m_port = SwapU16(common::deserializeU16(header + sizeof(unsigned int) * 3));
    OSBlockMove(header, header + cRelayHeaderSize, payloadSize, FALSE);
    *size = payloadSize;
    return Result(cResultSuccess);
}

const common::InetAddress& NexNatRelay::GetRelayServerAddress(const common::InetAddress& stationAddress) const
{
    for (const common::ListNode* node = m_relayTable.Sentinel()->next; node != m_relayTable.Sentinel();
         node = node->next)
    {
        const long long key = RelayTable::ValueOf(node)->m_stationAddress.GetKey();
        if (key == stationAddress.GetKey())
            return RelayTable::ValueOf(node)->m_relayAddress;
    }
    return s_invalidAddress;
}

}
}
}

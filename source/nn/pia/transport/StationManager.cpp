#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/Common.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/transport/TransportSupport.hpp"
#include "platform/wiiu/pead.hpp"

#include <new>
#include <cstddef>

namespace nn
{
namespace pia
{
namespace transport
{
StationManager* StationManager::s_pInstance = 0;
namespace
{
void* g_stationManagerStorage = 0;
}

StationManager::StationManager()
    : m_stationList(),
      m_stationStorage(0),
      m_stations(),
      m_freeStations(),
      m_localStation(0),
      m_invalidStationIndex(static_cast<StationIndex>(0xfd))
{
}
StationManager::~StationManager()
{
    if (m_stationStorage != 0)
        Finalize();
}

Result StationManager::CreateInstance()
{
    if (IsInitialized() == 0)
        return 0x10c0c;
    if (IsInSetupMode() == 0)
        return 0x10c08;
    if (s_pInstance != 0)
        return 0x10c0f;
    g_stationManagerStorage = pead::allocateFromSeadHeap(sizeof(StationManager), common::HeapManager::GetHeap(), 8);
    if (g_stationManagerStorage == 0)
        return 0x10c01;
    for (unsigned int i = 0; i < 0x140; ++i)
        static_cast<unsigned char*>(g_stationManagerStorage)[i] = 0;
    s_pInstance = new (g_stationManagerStorage) StationManager();
    return 0;
}
void StationManager::DestroyInstance()
{
    if (s_pInstance != 0)
    {
        s_pInstance->~StationManager();
        s_pInstance = 0;
    }
    if (g_stationManagerStorage != 0)
    {
        pead::DeleteArray(reinterpret_cast<unsigned char*>(g_stationManagerStorage));
        g_stationManagerStorage = 0;
    }
}

Result StationManager::Initialize(NetworkFactory* factory, unsigned int count, StationProtocol* protocol,
                                  RttProtocol* rtt)
{
    if (factory == 0 || count == 0 || count > cStationNumMax)
        return 0x10c07;
    const unsigned int bytes = count * sizeof(StationList::Node);
    unsigned char* memory = static_cast<unsigned char*>(
        pead::allocateFromSeadHeap(bytes, common::HeapManager::GetHeap()));
    if (memory == 0)
    {
        m_stationStorage = 0;
        return 0x10c01;
    }
    for (unsigned int i = 0; i < bytes; ++i)
        memory[i] = 0;
    m_stationStorage = memory;
    m_stationList.Initialize(memory, count);
    for (unsigned int remaining = count; remaining != 0; --remaining)
    {
        Station* station = m_stationList.Alloc();
        station->Initialize(factory, protocol, rtt);
    }
    for (common::ListNode* node = m_stationList.Sentinel()->next; node != m_stationList.Sentinel(); node = node->next)
        if (m_freeStations.size < cStationNumMax)
            m_freeStations.data[m_freeStations.size++] = StationList::ValueOf(node);
    return 0;
}

void StationManager::Finalize()
{
    if (m_stationList.IsInitialized())
    {
        for (common::ListNode* node = m_stationList.Sentinel()->next; node != m_stationList.Sentinel();
             node = node->next)
            StationList::ValueOf(node)->Finalize();
        Station* station;
        while ((station = m_stationList.Back()) != 0)
            m_stationList.Destroy(station);
    }
    if (m_stationStorage != 0)
    {
        common::DeleteArray(m_stationStorage);
        m_stationStorage = 0;
    }
}
Station* StationManager::CreateStation()
{
    if (!m_stationList.IsInitialized() || m_freeStations.size == 0 || m_stations.size == cStationNumMax)
        return 0;
    Station* station = m_freeStations.data[m_freeStations.size - 1];
    --m_freeStations.size;
    m_stations.data[m_stations.size++] = station;
    return station;
}
Station* StationManager::CreateLocalStation()
{
    if (m_localStation != 0)
        return 0;
    m_localStation = CreateStation();
    return m_localStation;
}
bool StationManager::isInStationList(const Station* station) const
{
    for (const common::ListNode* n = m_stationList.Sentinel()->next; n != m_stationList.Sentinel(); n = n->next)
        if (StationList::ValueOf(n) == station)
            return true;
    return false;
}
bool StationManager::isInPtrArray(const common::SimpleContainer<Station*, cStationNumMax>& array,
                                  const Station* station) const
{
    for (unsigned int i = 0; i < array.size; ++i)
        if (array.data[i] == station)
            return true;
    return false;
}
void StationManager::DestroyStation(Station* station)
{
    if (station == 0 || !isInStationList(station))
        return;
    if (!isInPtrArray(m_stations, station))
        return;
    for (unsigned int i = 0; i < m_stations.size; ++i)
    {
        if (m_stations.data[i] != station)
            continue;
        for (unsigned int j = i + 1; j < m_stations.size; ++j)
            m_stations.data[j - 1] = m_stations.data[j];
        --m_stations.size;
        if (m_freeStations.size < cStationNumMax)
            m_freeStations.data[m_freeStations.size++] = station;
        if (m_localStation == station)
            m_localStation = 0;
        return;
    }
}
}
}
}

namespace
{
unsigned int isValidStationIndex(nn::pia::StationIndex stationIndex)
{
    if (stationIndex <= nn::pia::transport::StationManager::cStationNumMax - 1
        || stationIndex == nn::pia::cStationIndexSentinelFE)
        return 1;
    return 0;
}
}

namespace nn
{
namespace pia
{
namespace transport
{
Station* StationManager::GetStation(StationIndex index)
{
    if (!isValidStationIndex(index))
        return 0;
    for (unsigned int i = 0; i < m_stations.size; ++i)
        if (m_stations.data[i]->m_field10 == static_cast<unsigned int>(index))
            return m_stations.data[i];
    return 0;
}
const Station* StationManager::GetStation(StationIndex index) const
{
    return const_cast<StationManager*>(this)->GetStation(index);
}
Station* StationManager::GetStation(unsigned long long id)
{
    return GetStation(Conv2StationIndex(id));
}
Station* StationManager::GetStation(const common::StationAddress& address)
{
    for (unsigned int i = 0; i < m_stations.size; ++i)
        if (m_stations.data[i]->m_stationAddress == address)
            return m_stations.data[i];
    return 0;
}
Station* StationManager::GetStationByStationIdWithoutConvert(unsigned long long id)
{
    for (unsigned int i = 0; i < m_stations.size; ++i)
        if (m_stations.data[i]->m_stationId == id)
            return m_stations.data[i];
    return 0;
}
Result StationManager::GetStationAddress(common::StationAddress* address, StationIndex index) const
{
    if (address == 0 || !isValidStationIndex(index))
        return 0x10c07;
    const Station* station = GetStation(index);
    if (station == 0)
        return 0x2c0a;
    *address = station->m_stationAddress;
    return 0;
}
unsigned short StationManager::GetValidStationNum() const
{
    return static_cast<unsigned short>(m_stations.size);
}
unsigned int StationManager::GetParticipatingStationBitmap(bool includeLocal) const
{
    unsigned int result = 0;
    for (unsigned int i = 0; i < m_stations.size; ++i)
    {
        const Station* s = m_stations.data[i];
        if (s->m_field10 < 32 && s->m_field20 == 5)
            result |= 1u << s->m_field10;
    }
    if (!includeLocal && m_localStation != 0 && m_localStation->m_field10 < 32)
        result &= ~(1u << m_localStation->m_field10);
    return result;
}
void StationManager::Trace(unsigned long long) const
{
}
}
}
}

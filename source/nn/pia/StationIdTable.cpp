#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Memory.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

StationIdTable::StationIdTable() : m_storage(0)
{
}

void StationIdTable::Clear()
{
    if (m_entries.IsInitialized())
        m_entries.Clear();
    m_availableEntryNum = m_entries.GetCapacity();
}

void StationIdTable::Finalize()
{
    if (m_storage != 0)
    {
        Clear();
        common::DeleteArray(m_storage);
        m_storage = 0;
    }
}

StationIdTable::~StationIdTable()
{
    Finalize();
}

Result StationIdTable::Initialize(unsigned int count)
{
    if (count == 0)
        return static_cast<Result>(0x10c07);
    if (m_storage != 0)
        return static_cast<Result>(0x10c02);

    m_storage = common::NewArray<unsigned long long>((sizeof(EntryList::Node) * count + sizeof(unsigned long long) - 1)
                                                     / sizeof(unsigned long long));
    if (m_storage == 0)
        return static_cast<Result>(0x10c01);
    m_entries.Initialize(m_storage, count);
    m_availableEntryNum = count;
    return 0;
}

Result StationIdTable::Add(uint64_t stationId, StationIndex index, uint64_t uniqueId)
{
    if (m_entries.GetFreeNum() == 0)
        return static_cast<Result>(0xc10);
    Entry* entry = m_entries.Alloc();
    entry->stationId = stationId;
    entry->stationIndex = static_cast<uint32_t>(index);
    entry->uniqueId = uniqueId;
    entry->joinEventNotified = false;
    return 0;
}

StationIdTable::Entry* StationIdTable::FindByStationIdCore(uint64_t stationId) const
{
    for (common::ListNode* node = m_entries.Sentinel()->next; node != m_entries.Sentinel(); node = node->next)
    {
        Entry* entry = EntryList::ValueOf(node);
        if (entry->stationId == stationId)
            return entry;
    }
    return 0;
}

void StationIdTable::SetJoinEventNotifiedByStationId(uint64_t stationId, bool notified)
{
    Entry* entry = FindByStationIdCore(stationId);
    if (entry != 0)
        entry->joinEventNotified = notified;
}

StationIdTable::Entry* StationIdTable::FindByStationIndexCore(StationIndex index) const
{
    for (common::ListNode* node = m_entries.Sentinel()->next; node != m_entries.Sentinel(); node = node->next)
    {
        Entry* entry = EntryList::ValueOf(node);
        if (entry->stationIndex == static_cast<uint32_t>(index))
            return entry;
    }
    return 0;
}

void StationIdTable::SetJoinEventNotifiedByStationIndex(StationIndex index, bool notified)
{
    Entry* entry = FindByStationIndexCore(index);
    if (entry != 0)
        entry->joinEventNotified = notified;
}

uint8_t StationIdTable::IsJoinEventNotifiedByStationIndex(StationIndex index) const
{
    Entry* entry = FindByStationIndexCore(index);
    return entry != 0 ? entry->joinEventNotified : 0;
}

uint8_t StationIdTable::IsJoinEventNotifiedByStationId(uint64_t stationId) const
{
    Entry* entry = FindByStationIdCore(stationId);
    if (entry == 0)
        return 0;
    return entry->joinEventNotified;
}

Result StationIdTable::RemoveCore(Entry* entry)
{
    if (entry == 0)
        return static_cast<Result>(0x2c0a);
    entry->stationId = 0;
    entry->stationIndex = cStationIndexInvalid;
    entry->uniqueId = 0;
    entry->joinEventNotified = false;
    m_entries.Free(EntryList::NodeOf(entry));
    return 0;
}

Result StationIdTable::RemoveByStationId(uint64_t stationId)
{
    return RemoveCore(FindByStationIdCore(stationId));
}

StationIdTable::Entry* StationIdTable::FindByUniqueIdCore(uint64_t uniqueId) const
{
    for (common::ListNode* node = m_entries.Sentinel()->next; node != m_entries.Sentinel(); node = node->next)
    {
        Entry* entry = EntryList::ValueOf(node);
        if (entry->uniqueId == uniqueId)
            return entry;
    }
    return 0;
}

Result StationIdTable::Copy(Entry* destination, const Entry* source) const
{
    if (source == 0)
        return static_cast<Result>(0x2c0a);
    *destination = *source;
    return 0;
}

Result StationIdTable::FindByStationIndex(Entry* entry, StationIndex index) const
{
    return entry == 0 ? Result(cResultInvalidArgument) : Copy(entry, FindByStationIndexCore(index));
}

Result StationIdTable::FindByStationId(Entry* entry, uint64_t stationId) const
{
    return entry == 0 ? Result(cResultInvalidArgument) : Copy(entry, FindByStationIdCore(stationId));
}

Result StationIdTable::FindByUniqueId(Entry* entry, uint64_t uniqueId) const
{
    return entry == 0 ? Result(cResultInvalidArgument) : Copy(entry, FindByUniqueIdCore(uniqueId));
}

StationIndex StationIdTable::FindUnsafe(uint64_t stationId) const
{
    Entry* entry = FindByStationIdCore(stationId);
    return entry != 0 ? static_cast<StationIndex>(entry->stationIndex) : pia::cStationIndexInvalid;
}

unsigned int StationIdTable::GetEntryNum() const
{
    unsigned int entryNum = 0;
    const common::ListNode* sentinel = m_entries.Sentinel();
    for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
        ++entryNum;
    return entryNum;
}

void StationIdTable::SetAvailableEntryNum(unsigned int count)
{
    if (m_entries.GetCapacity() < count)
        return;
    m_availableEntryNum = count;
}

void StationIdTable::Trace(uint64_t) const
{
}

}
}
}

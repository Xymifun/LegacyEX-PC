#include "nn/pia/transport/IdentificationInfoTable.hpp"

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/transport/TransportSupport.hpp"
#include "platform/wiiu/pead.hpp"

#include <coreinit/memory.h>

#include <new>

namespace nn
{
namespace pia
{
namespace transport
{

namespace
{
IdentificationInfoTable* g_identificationInfoTable = 0;
unsigned char* g_identificationInfoTableStorage = 0;

inline void CopyToken(IdentificationInfo* destination, const IdentificationInfo* source)
{
    OSBlockMove(destination->m_field00.m_token, source->m_field00.m_token, sizeof(destination->m_field00.m_token),
                FALSE);
}

inline void CopyCounts(IdentificationInfo* destination, const IdentificationInfo* source)
{
    destination->m_field109 = source->m_field109;
    destination->m_playerCount = source->m_playerCount;
    destination->m_participantCount = source->m_participantCount;
    destination->m_field10c = source->m_field10c;
}

inline bool SameCountsAndToken(const IdentificationInfo* stored, const IdentificationInfo* info)
{
    if (stored->m_field109 != info->m_field109 || stored->m_playerCount != info->m_playerCount
        || stored->m_participantCount != info->m_participantCount || stored->m_field10c != info->m_field10c)
        return false;
    for (unsigned int i = 0; i < sizeof(stored->m_field00.m_token); ++i)
        if (stored->m_field00.m_token[i] != info->m_field00.m_token[i])
            return false;
    return true;
}
}

IdentificationInfoTable::IdentificationInfoTable() : m_count(0), m_criticalSection(-1)
{
    ClearIdentificationInfo(this);
}

IdentificationInfoTable::~IdentificationInfoTable()
{
    Finalize();
}

Result IdentificationInfoTable::CreateInstance()
{
    if (IsInitialized() == 0)
        return 0x10c0c;
    if (IsInSetupMode() == 0)
        return 0x10c08;
    if (g_identificationInfoTable != 0)
        return 0x10c0f;

    g_identificationInfoTableStorage = common::NewArray<unsigned char>(sizeof(IdentificationInfoTable), 8);
    if (g_identificationInfoTableStorage == 0)
        return 0x10c01;
    g_identificationInfoTable = new (g_identificationInfoTableStorage) IdentificationInfoTable();
    return 0;
}

void IdentificationInfoTable::DestroyInstance()
{
    if (g_identificationInfoTable != 0)
    {
        g_identificationInfoTable->~IdentificationInfoTable();
        g_identificationInfoTable = 0;
    }
    if (g_identificationInfoTableStorage != 0)
    {
        common::DeleteArray(g_identificationInfoTableStorage);
        g_identificationInfoTableStorage = 0;
    }
}

IdentificationInfoTable* IdentificationInfoTable::GetInstance()
{
    return g_identificationInfoTable;
}

void IdentificationInfoTable::Finalize()
{
    Record* record;
    while ((record = m_pool.Back()) != 0)
        m_pool.Destroy(record);
}

Result IdentificationInfoTable::Initialize(unsigned short count)
{
    if (count > 32)
        return 0x10c07;
    m_count = count;
    for (unsigned int i = 0; i < m_count; ++i)
    {
        Record* record = m_pool.Alloc();
        if (record == 0)
            return 0x10c08;
        record->m_station = 0;
        record->m_state = 0;
        ClearIdentificationInfo(&record->m_info);
    }
    return 0;
}

void IdentificationInfoTable::ClearIdentificationInfo(IdentificationInfo* info)
{
    OSBlockSet(info->m_field00.m_token, 0, sizeof(info->m_field00.m_token));
    info->m_fieldC8[0].Clear();
    info->m_field60[0].Clear();
    info->m_field108 = Station::PlayerInfo::cLanguageUnset;
    OSBlockSet(info->m_field20.m_bytes, 0, sizeof(info->m_field20.m_bytes));
    info->m_playerCount = 0;
    info->m_field109 = 0;
    info->m_principalId[0] = 0;
    info->m_field10c = 0;
    info->m_participantCount = 0;
}

void IdentificationInfoTable::SetLocalIdentificationInfo(const IdentificationInfo* info)
{
    if (info != 0)
    {
        OSBlockMove(m_field00.m_token, info->m_field00.m_token, sizeof(m_field00.m_token), FALSE);
        m_fieldC8[0].Copy(info->m_fieldC8[0]);
        m_field60[0].Copy(info->m_field60[0]);
        m_field108 = info->m_field108;
        OSBlockMove(m_field20.m_bytes, info->m_field20.m_bytes, sizeof(m_field20.m_bytes), FALSE);
        m_principalId[0] = info->m_principalId[0];
        m_fieldC8[0].Trace(0x4000000);
        m_field60[0].Trace(0x4000000);
        CopyCounts(this, info);
    }
}

Result IdentificationInfoTable::GetLocalIdentificationInfo(IdentificationInfo* info)
{
    if (info == 0)
        return 0x10c07;
    OSBlockMove(info->m_field00.m_token, m_field00.m_token, sizeof(info->m_field00.m_token), FALSE);
    info->m_fieldC8[0].Copy(m_fieldC8[0]);
    info->m_field60[0].Copy(m_field60[0]);
    info->m_field108 = m_field108;
    OSBlockMove(info->m_field20.m_bytes, m_field20.m_bytes, sizeof(info->m_field20.m_bytes), FALSE);
    info->m_principalId[0] = m_principalId[0];
    CopyCounts(info, this);
    return 0;
}

void IdentificationInfoTable::ClearLocalIdentificationInfo()
{
    ClearIdentificationInfo(this);
}

Result IdentificationInfoTable::AddToTable(const Station* station, const IdentificationInfo* info, unsigned char kind)
{
    if (station == 0 || info == 0 || kind > 2)
        return 0x10c07;
    station->Trace(0x8000);
    m_criticalSection.Lock();
    common::ListNode* first = m_pool.Sentinel()->next;
    for (common::ListNode* node = first; node != m_pool.Sentinel(); node = node->next)
    {
        Record& record = *RecordPool::ValueOf(node);
        if (record.m_station != station)
            continue;
        if (kind == 0)
        {
            record.m_state = 3;
            OSBlockMove(record.m_info.m_field00.m_token, info->m_field00.m_token,
                        sizeof(record.m_info.m_field00.m_token), FALSE);
            record.m_info.m_fieldC8[0].Copy(info->m_fieldC8[0]);
            record.m_info.m_field60[0].Copy(info->m_field60[0]);
            record.m_info.m_field108 = info->m_field108;
            OSBlockMove(record.m_info.m_field20.m_bytes, info->m_field20.m_bytes,
                        sizeof(record.m_info.m_field20.m_bytes), FALSE);
            record.m_info.m_principalId[0] = info->m_principalId[0];
            CopyCounts(&record.m_info, info);
        }
        else if (kind == 1)
        {
            if ((record.m_state & 2) != 0 && !SameCountsAndToken(&record.m_info, info))
            {
                m_criticalSection.Unlock();
                return 0x10c08;
            }
            record.m_state |= 1;
            OSBlockMove(record.m_info.m_field00.m_token, info->m_field00.m_token,
                        sizeof(record.m_info.m_field00.m_token), FALSE);
            record.m_info.m_fieldC8[0].Copy(info->m_fieldC8[0]);
            record.m_info.m_field60[0].Copy(info->m_field60[0]);
            record.m_info.m_field108 = info->m_field108;
            OSBlockMove(record.m_info.m_field20.m_bytes, info->m_field20.m_bytes,
                        sizeof(record.m_info.m_field20.m_bytes), FALSE);
            record.m_info.m_principalId[0] = info->m_principalId[0];
            CopyCounts(&record.m_info, info);
        }
        else
        {
            if ((record.m_state & 1) != 0 && !SameCountsAndToken(&record.m_info, info))
            {
                m_criticalSection.Unlock();
                return 0x10c08;
            }
            record.m_state |= 2;
            CopyToken(&record.m_info, info);
            CopyCounts(&record.m_info, info);
        }
        m_criticalSection.Unlock();
        return 0;
    }
    for (common::ListNode* node = first; node != m_pool.Sentinel(); node = node->next)
    {
        Record& record = *RecordPool::ValueOf(node);
        if (record.m_station != 0)
            continue;
        record.m_station = station;
        CopyToken(&record.m_info, info);
        if (kind == 0)
        {
            record.m_state = 3;
            record.m_info.m_fieldC8[0].Copy(info->m_fieldC8[0]);
            record.m_info.m_field60[0].Copy(info->m_field60[0]);
            record.m_info.m_field108 = info->m_field108;
            OSBlockMove(record.m_info.m_field20.m_bytes, info->m_field20.m_bytes,
                        sizeof(record.m_info.m_field20.m_bytes), FALSE);
            record.m_info.m_principalId[0] = info->m_principalId[0];
        }
        else if (kind == 1)
        {
            record.m_state |= 1;
            record.m_info.m_fieldC8[0].Copy(info->m_fieldC8[0]);
            record.m_info.m_field60[0].Copy(info->m_field60[0]);
            record.m_info.m_field108 = info->m_field108;
            OSBlockMove(record.m_info.m_field20.m_bytes, info->m_field20.m_bytes,
                        sizeof(record.m_info.m_field20.m_bytes), FALSE);
            record.m_info.m_principalId[0] = info->m_principalId[0];
        }
        else
        {
            record.m_state |= 2;
        }
        CopyCounts(&record.m_info, info);
        m_criticalSection.Unlock();
        return 0;
    }
    m_criticalSection.Unlock();
    return 0x10c08;
}

IdentificationInfo* IdentificationInfoTable::GetIdentificationInfoPtr(const Station* station)
{
    common::ScopedLock lock(m_criticalSection);
    for (common::ListNode* node = m_pool.Sentinel()->next; node != m_pool.Sentinel(); node = node->next)
    {
        Record* record = RecordPool::ValueOf(node);
        if (record->m_station == station && record->m_state == 3)
            return &record->m_info;
    }
    return 0;
}

Result IdentificationInfoTable::GetIdentificationInfo(const Station* station, IdentificationInfo* info)
{
    if (station == 0 || info == 0)
        return 0x10c07;
    IdentificationInfo* found = GetIdentificationInfoPtr(station);
    if (found == 0)
        return 0x2c09;
    OSBlockMove(info->m_field00.m_token, found->m_field00.m_token, sizeof(info->m_field00.m_token), FALSE);
    info->m_fieldC8[0].Copy(found->m_fieldC8[0]);
    info->m_field60[0].Copy(found->m_field60[0]);
    info->m_field108 = found->m_field108;
    OSBlockMove(info->m_field20.m_bytes, found->m_field20.m_bytes, sizeof(info->m_field20.m_bytes), FALSE);
    info->m_principalId[0] = found->m_principalId[0];
    CopyCounts(info, found);
    return 0;
}

IdentificationInfo* IdentificationInfoTable::GetIdentificationInfoPtr(unsigned long long principalId)
{
    common::ScopedLock lock(m_criticalSection);
    for (common::ListNode* node = m_pool.Sentinel()->next; node != m_pool.Sentinel(); node = node->next)
    {
        Record* record = RecordPool::ValueOf(node);
        if (record->m_state == 3 && record->m_info.m_principalId[0] == principalId)
            return &record->m_info;
    }
    return 0;
}

Result IdentificationInfoTable::GetIdentificationInfo(unsigned long long principalId, IdentificationInfo* info)
{
    if (info == 0)
        return 0x10c07;
    IdentificationInfo* found = GetIdentificationInfoPtr(principalId);
    if (found == 0)
        return 0x2c09;
    OSBlockMove(info->m_field00.m_token, found->m_field00.m_token, sizeof(info->m_field00.m_token), FALSE);
    info->m_fieldC8[0].Copy(found->m_fieldC8[0]);
    info->m_field60[0].Copy(found->m_field60[0]);
    info->m_field108 = found->m_field108;
    OSBlockMove(info->m_field20.m_bytes, found->m_field20.m_bytes, sizeof(info->m_field20.m_bytes), FALSE);
    info->m_principalId[0] = found->m_principalId[0];
    CopyCounts(info, found);
    return 0;
}

Result IdentificationInfoTable::GetPlayerCount(const Station* station, unsigned char* count)
{
    if (station == 0)
        return 0x10c07;
    if (count == 0)
        return 0x10c07;
    IdentificationInfo* found = GetIdentificationInfoPtr(station);
    if (found == 0)
        return 0x2c09;
    *count = found->m_playerCount;
    return 0;
}

Result IdentificationInfoTable::GetParticipantCount(const Station* station, unsigned char* count)
{
    if (station == 0)
        return 0x10c07;
    if (count == 0)
        return 0x10c07;
    IdentificationInfo* found = GetIdentificationInfoPtr(station);
    if (found == 0)
        return 0x2c09;
    *count = found->m_participantCount;
    return 0;
}

void IdentificationInfoTable::EraseFromTable(const Station* station)
{
    if (station == 0)
        return;
    common::ScopedLock lock(m_criticalSection);
    for (common::ListNode* node = m_pool.Sentinel()->next; node != m_pool.Sentinel(); node = node->next)
    {
        Record* record = RecordPool::ValueOf(node);
        if (record->m_station == station)
        {
            record->m_station = 0;
            record->m_state = 0;
            ClearIdentificationInfo(&record->m_info);
            return;
        }
    }
}

void IdentificationInfoTable::ClearTable()
{
    common::ScopedLock lock(m_criticalSection);
    for (common::ListNode* node = m_pool.Sentinel()->next; node != m_pool.Sentinel(); node = node->next)
    {
        Record* record = RecordPool::ValueOf(node);
        record->m_station = 0;
        record->m_state = 0;
        ClearIdentificationInfo(&record->m_info);
    }
}

void IdentificationInfoTable::Trace(unsigned long long) const
{
}

}
}
}
